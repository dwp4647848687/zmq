#include "mdbroker.hpp"

#include <memory>

#include "zmq.hpp"
#include "zmq_addon.hpp"

#include "mdp.hpp"
#include "zhelpers.hpp"

namespace md1::majordomo {
//------1-------2-------3-------4-------5-------6-------7-------8-------9-------10------11------12------13------14------15------

Broker::Broker(bool verbose, std::shared_ptr<zmq::context_t> ctx) :
        m_ctx(ctx),
        m_verbose(verbose)
{
    if (m_ctx == nullptr)
    {
        m_ctx = std::make_shared<zmq::context_t>(1) ;
    }
    m_socket = std::make_unique<zmq::socket_t>(*m_ctx, ZMQ_ROUTER) ;
}

void Broker::bind(const std::string& endpoint)
{
    m_endpoint = endpoint ;
    m_socket->bind(endpoint) ;
    if (m_verbose == true)
    {
        s_console("I: MBP broker is active at %s", endpoint.c_str()) ;
    }
}

void Broker::startBrokering()
{
    TimePoint now = std::chrono::system_clock::now() ;
    TimePoint heartbeatAt = now + std::chrono::milliseconds(HEARTBEAT_INTERVAL) ;
    while (s_interrupted == false)
    {
        Duration timeout = std::chrono::duration_cast<Duration>(heartbeatAt - now) ;
        if (timeout.count() < 0)
        {
            timeout = std::chrono::milliseconds(0) ;
        }
        zmq::pollitem_t items[] = { { *m_socket, 0, ZMQ_POLLIN, 0 } } ;
        zmq::poll(items, 1, timeout) ;

        if (items[0].revents & ZMQ_POLLIN)
        {
            zmq::multipart_t msg(*m_socket) ;
            if (m_verbose == true)
            {
                s_console("I: received message:") ;
                dump(msg) ;
            }
            std::string senderIdentity = msg.popstr() ;
            msg.pop() ;
            std::string header = msg.popstr() ;
            if (header == MDP_CLIENT)
            {
                processClientMessage(senderIdentity, std::move(msg)) ;
            }
            else if (header == MDP_WORKER)
            {
                processWorkerMessage(senderIdentity, std::move(msg)) ;
            }
            else
            {
                s_console("E: invalid message:") ;
                dump(msg) ;
            }
        }
        if (std::chrono::system_clock::now() > heartbeatAt)
        {
            purgeWorkers() ;
            for (auto* worker : m_waitingWorkers)
            {
                sendWorkerMessage(*worker, WMessageType::HEARTBEAT, std::nullopt) ;
            }
            heartbeatAt += std::chrono::milliseconds(HEARTBEAT_INTERVAL) ;
            now = std::chrono::system_clock::now() ;
        }
    }
}

//------1-------2-------3-------4-------5-------6-------7-------8-------9-------10------11------12------13------14------15------

Service& Broker::getService(const std::string& name)
{
    auto it = m_services.find(name) ;
    if (it == m_services.end())
    {
        m_services.emplace(name, Service(name)) ;
        it = m_services.find(name) ;
        if (m_verbose == true)
        {
            s_console ("I: Registering new service: %s", name.c_str()) ;
        }
    }
    return it->second ;
}

Worker& Broker::getWorker(const std::string& identity)
{
    auto it = m_workers.find(identity) ;
    if (it == m_workers.end())
    {
        m_workers.emplace(identity, Worker(identity)) ;
        it = m_workers.find(identity) ;
        if (m_verbose == true)
        {
            s_console ("I: Registering new worker: %s", identity.c_str()) ;
        }
    }
    return it->second ;
}

void Broker::removeWorker(const std::string& identity, bool disconnect)
{
    auto it = m_workers.find(identity) ;
    if (it == m_workers.end())
    {
        return ;
    }
    Worker& worker = it->second ;
    if (disconnect == true)
    {
        sendWorkerMessage(worker, WMessageType::DISCONNECT, std::nullopt) ;
    }
    if (worker.m_service != nullptr)
    {
        worker.m_service->m_waitingWorkers.remove(&worker) ;
        worker.m_service->m_numWorkers-- ;
    }
    m_waitingWorkers.erase(&worker) ;
    m_workers.erase(it) ;
}

void Broker::purgeWorkers()
{
    std::deque<Worker*> toRemove ;
    TimePoint now = std::chrono::system_clock::now() ;
    for (auto worker : m_waitingWorkers)
    {
        if (worker->m_expiry < now)
        {
            toRemove.push_back(worker) ;
        }
    }
    for (auto worker : toRemove)
    {
        if (m_verbose == true)
        {
            s_console("I: deleting expired worker: %s", worker->m_identity.c_str()) ;
        }
        removeWorker(worker->m_identity, false) ;
    }
}

//------1-------2-------3-------4-------5-------6-------7-------8-------9-------10------11------12------13------14------15------

void Broker::dispatch(Service& service, std::optional<zmq::multipart_t>&& msg)
{
    if (msg.has_value() == true)
    {
        service.m_requests.emplace_back(std::move(msg.value())) ;
    }
    purgeWorkers() ;
    while (service.m_waitingWorkers.empty() == false && service.m_requests.empty() == false)
    {
        auto nextWorkerIt = std::max_element(service.m_waitingWorkers.begin(), service.m_waitingWorkers.end(),
            [](const Worker* a, const Worker* b)
            {
                return a->m_expiry < b->m_expiry;
            }
        ) ;
        // This should never happen since we checked that the worker list is not empty
        if (nextWorkerIt == service.m_waitingWorkers.end())
        {
            break ;
        }
        zmq::multipart_t& request = service.m_requests.front() ;
        sendWorkerMessage(**nextWorkerIt, WMessageType::REQUEST, std::move(request)) ;
        service.m_requests.pop_front() ;
        service.m_waitingWorkers.erase(nextWorkerIt) ;
        m_waitingWorkers.erase(*nextWorkerIt) ;
    }
}

void Broker::handleInternalService(const std::string& service, zmq::multipart_t&& msg)
{
    if (service != "mmi.service")
    {
        messageSetBody(msg, zmq::message_t(std::string("501"))) ;
    }
    std::string msgBody = msg.remove().str() ;
    auto it = m_services.find(msgBody) ;
    if (it != m_services.end() && it->second.m_numWorkers > 0)
    {
        messageSetBody(msg, zmq::message_t(std::string("200"))) ;
    }
    else
    {
        messageSetBody(msg, zmq::message_t(std::string("404"))) ;
    }

    std::string client = msg.popstr() ;
}

void Broker::messageSetBody(zmq::multipart_t& msg, zmq::message_t&& body)
{
    if (msg.empty() == false)
    {
        msg.remove() ;
    }
    msg.add(std::move(body)) ;
}

//------1-------2-------3-------4-------5-------6-------7-------8-------9-------10------11------12------13------14------15------

void Broker::processWorkerMessage(const std::string& workerIdentity, zmq::multipart_t&& message)
{
    assert(message.size() >= 1) ;
    WMessageType type = *static_cast<WMessageType*>(message.pop().data()) ;
    bool workerReady = m_workers.find(workerIdentity) != m_workers.end() ;
    Worker& worker = getWorker(workerIdentity) ;

    switch (type)
    {
        case WMessageType::READY:
        {
            if (workerReady == true || (workerIdentity.length() >= 4 && workerIdentity.substr(0, 4) == "mmi."))
            {
                removeWorker(workerIdentity, true) ;
                break ;
            }
            std::string service = message.popstr() ;
            worker.m_service = &getService(service) ;
            worker.m_service->m_numWorkers++ ;
            markWorkerAsWaiting(worker) ;
            break ;
        }
        case WMessageType::REPLY:
        {
            if (workerReady == false)
            {
                removeWorker(workerIdentity, true) ;
                break ;
            }
            std::string client = message.popstr() ;
            message.pop() ;
            message.pushstr(worker.m_service->m_name) ;
            message.pushstr(std::string(MDP_CLIENT)) ;
            message.push(zmq::message_t()) ;
            message.pushstr(client) ;
            message.send(*m_socket) ;
            markWorkerAsWaiting(worker) ;
            break ;
        }
        case WMessageType::HEARTBEAT:
        {
            if (workerReady == true)
            {
                worker.m_expiry = std::chrono::system_clock::now() + std::chrono::milliseconds(HEARTBEAT_EXPIRY) ;
            }
            else
            {
                removeWorker(workerIdentity, true) ;
            }
            break ;
        }
        case WMessageType::DISCONNECT:
        {
            removeWorker(workerIdentity, false) ;
            break ;
        }
        default:
        {
            s_console("E: invalid input message (%d)", static_cast<uint8_t>(type)) ;
            dump(message) ;
            break ;
        }
    }
}

void Broker::sendWorkerMessage(Worker& worker, WMessageType type, std::optional<zmq::multipart_t>&& msg)
{
    if (msg.has_value() == false)
    {
        msg = zmq::multipart_t() ;
    }
    msg->push(zmq::message_t(&type, 1)) ;
    msg->pushstr(std::string(MDP_WORKER)) ;
    msg->push(zmq::message_t()) ;
    msg->pushstr(worker.m_identity) ;

    if (m_verbose == true)
    {
        s_console("I: sending %s to worker %s", mdps_commands[static_cast<uint8_t>(type)].data(), worker.m_identity.c_str()) ;
        dump(*msg) ;
    }
    msg->send(*m_socket) ;
}

void Broker::markWorkerAsWaiting(Worker& worker)
{
    m_waitingWorkers.insert(&worker) ;
    worker.m_service->m_waitingWorkers.push_back(&worker) ;
    worker.m_expiry = std::chrono::system_clock::now() + std::chrono::milliseconds(HEARTBEAT_EXPIRY) ;
    dispatch(*worker.m_service, std::nullopt) ;
}

void Broker::processClientMessage(const std::string& clientName, zmq::multipart_t&& message)
{
    assert (message.size() >= 2) ;
    std::string serviceName = message.popstr() ;
    Service& service = getService(serviceName) ;

    message.push(zmq::message_t()) ;
    message.pushstr(clientName) ;
    if (serviceName.length() >= 4 && serviceName.substr(0, 4) == "mmi.")
    {
        handleInternalService(serviceName, std::move(message)) ;
    }
    else
    {
        dispatch(service, std::move(message)) ;
    }
}

//------1-------2-------3-------4-------5-------6-------7-------8-------9-------10------11------12------13------14------15------
}  // namespace md1::majordomo