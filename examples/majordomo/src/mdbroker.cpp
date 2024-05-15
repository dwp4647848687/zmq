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
        s_console("I: MBP broker is active at %s", endpoint) ;
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
            s_console ("I: Registering new service: %s", name);
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
            s_console ("I: Registering new worker: %s", identity);
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
    if (disconnect == true)
    {
        sendWorkerMessage(identity, WMessageType::DISCONNECT, std::nullopt) ;
    }
    Worker& worker = it->second ;
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

//------1-------2-------3-------4-------5-------6-------7-------8-------9-------10------11------12------13------14------15------
}  // namespace md1::majordomo