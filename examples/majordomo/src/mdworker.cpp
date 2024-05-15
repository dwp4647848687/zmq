#include "mdworker.hpp"

#include <chrono>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <thread>

#include "zmq.hpp"
#include "zmq_addon.hpp"

#include "mdp.hpp"
#include "zhelpers.hpp"

namespace md1::majordomo {
//------1-------2-------3-------4-------5-------6-------7-------8-------9-------10------11------12------13------14------15------

Worker::Worker(const std::string& brokerAddr, const std::string& service, bool verbose, std::shared_ptr<zmq::context_t> ctx) :
        m_brokerAddr(brokerAddr),
        m_service(service),
        m_verbose(verbose),
        m_ctx(ctx)
{
    assert(brokerAddr.length() > 0) ;
    s_version_assert(4, 0) ;
    if (m_ctx == nullptr)
    {
        m_ctx = std::make_shared<zmq::context_t>(1) ;
    }
    s_catch_signals() ;
    connectToBroker() ;
}

void Worker::connectToBroker()
{
    m_worker.reset() ;
    m_worker = std::make_unique<zmq::socket_t>(*m_ctx, ZMQ_REQ) ;
    s_set_id(*m_worker) ;
    m_worker->set(zmq::sockopt::linger, 0) ;
    m_worker->connect(m_brokerAddr) ;
    if (m_verbose == true)
    {
        s_console("I: connecting to broker at %s...", m_brokerAddr.c_str()) ;
    }
    sendToBroker(WMessageType::READY, std::nullopt) ;
    m_liveness = HEARTBEAT_LIVENESS ;
    m_nextHeartbeat = std::chrono::system_clock::now() + m_heartbeat ;
}

void Worker::setHeartbeat(Duration heartbeat)
{
    m_heartbeat = heartbeat ;
}

void Worker::setReconnect(Duration reconnect)
{
    m_reconnect = reconnect ;
}

//------1-------2-------3-------4-------5-------6-------7-------8-------9-------10------11------12------13------14------15------

zmq::multipart_t Worker::recv(std::optional<zmq::multipart_t>&& reply)
{
    assert (m_expectingReply == false || reply.has_value()) ;
    if (reply.has_value())
    {
        assert (m_replyAddr.length() > 0) ;
        reply->push(zmq::message_t()) ;
        reply->pushstr(m_replyAddr) ;
        sendToBroker(WMessageType::REPLY, std::move(reply)) ;
        m_replyAddr.clear() ;
    }
    m_expectingReply = true ;
    while (s_interrupted == false)
    {
        zmq::pollitem_t items[] = { { *m_worker, 0, ZMQ_POLLIN, 0 } } ;
        zmq::poll(items, 1, m_heartbeat) ;

        if (items[0].revents & ZMQ_POLLIN)
        {
            zmq::multipart_t msg ;
            msg.recv(*m_worker) ;
            if (m_verbose == true)
            {
                s_console("I: received message from broker:") ;
                dump(msg) ;
            }
            m_liveness = HEARTBEAT_LIVENESS ;
            // Don't try to handle errors, just assert noisily
            assert (msg.size() >= 3) ;
            assert (msg.pop() == zmq::message_t()) ;  // Empty delimiter
            assert (msg.popstr() == std::string(MDP_WORKER)) ;  // MDP/Worker header
            WMessageType type = static_cast<WMessageType>(*msg.pop().data<uint8_t>()) ;
            switch (type)
            {
                case WMessageType::REQUEST:
                {
                    m_replyAddr = msg.popstr() ;
                    msg.pop() ;  // Empty delimiter
                    return msg ;
                }
                case WMessageType::HEARTBEAT:
                {
                    // Do nothing
                    break ;
                }
                case WMessageType::DISCONNECT:
                {
                    connectToBroker() ;
                    break ;
                }
                default:
                {
                    s_console("E: invalid message type %d", static_cast<uint8_t>(type)) ;
                    dump(msg) ;
                    break ;
                }
            }
        }
        else if (--m_liveness == 0)
        {
            if (m_verbose == true)
            {
                s_console("W: disconnected from broker - retrying...") ;
            }
            std::this_thread::sleep_for(m_reconnect) ;
            connectToBroker() ;
        }
        if (std::chrono::system_clock::now() > m_nextHeartbeat)
        {
            sendToBroker(WMessageType::HEARTBEAT, std::nullopt) ;
            m_nextHeartbeat = std::chrono::system_clock::now() + m_heartbeat ;
        }
    }
    if (s_interrupted == true)
    {
        s_console("W: interrupt received, shutting down...") ;
    }
    return zmq::multipart_t() ;
}

void Worker::sendToBroker(WMessageType type, std::optional<zmq::multipart_t>&& msg)
{
    if (!msg.has_value())
    {
        msg = zmq::multipart_t() ;
    }
    if (type == WMessageType::READY)
    {
        msg->pushstr(m_service) ;               // Add the service name
    }
    msg->push(zmq::message_t(&type, 1)) ;       // Add the message type
    msg->pushstr(std::string(MDP_WORKER)) ;     // Add the MDP/Worker header
    msg->push(zmq::message_t()) ;               // Add an empty delimiter
    if (m_verbose == true)
    {
        s_console("I: send %s to broker:", mdps_commands[static_cast<uint8_t>(type)].data()) ;
        dump(*msg) ;
    }
    msg->send(*m_worker) ;
}

//------1-------2-------3-------4-------5-------6-------7-------8-------9-------10------11------12------13------14------15------
}  // namespace md1::majordomo