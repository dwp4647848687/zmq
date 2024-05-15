#include "mdclient.hpp"

#include <string>
#include <memory>
#include <cstdint>

#include "zmq.hpp"
#include "zmq_addon.hpp"

#include "mdp.hpp"
#include "zhelpers.hpp"

namespace md1::majordomo {
//------1-------2-------3-------4-------5-------6-------7-------8-------9-------10------11------12------13------14------15------

Client::Client(const std::string& brokerAddr, bool verbose, std::shared_ptr<zmq::context_t> ctx) :
        m_brokerAddr(brokerAddr),
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

void Client::connectToBroker()
{
    m_client.reset() ;
    m_client = std::make_unique<zmq::socket_t>(*m_ctx, ZMQ_REQ) ;
    s_set_id(*m_client) ;
    m_client->set(zmq::sockopt::linger, 0) ;
    m_client->connect(m_brokerAddr) ;
    if (m_verbose == true)
    {
        s_console("I: connecting to broker at %s...", m_brokerAddr.c_str()) ;
    }
}

void Client::setTimeOut(Duration timeout)
{
    m_timeout = timeout ;
}

void Client::setRetries(size_t retries)
{
    m_retries = retries ;
}

//------1-------2-------3-------4-------5-------6-------7-------8-------9-------10------11------12------13------14------15------

zmq::multipart_t Client::send(const std::string& service, zmq::multipart_t&& request)
{
    request.pushstr(service) ;
    request.pushstr(std::string(MDP_CLIENT)) ;
    if (m_verbose == true)
    {
        s_console ("I: send request to '%s' service:", service.c_str());
        dump(request);
    }

    size_t retriesLeft = m_retries ;
    while (retriesLeft > 0 && s_interrupted == false)
    {
        // Send a copy of the request, holding the original in case we have to retry
        request.clone().send(*m_client) ;
        while (s_interrupted == false)
        {
            zmq::pollitem_t items[] = { { *m_client, 0, ZMQ_POLLIN, 0 } } ;
            zmq::poll(&items[0], 1, m_timeout) ;
            if (items[0].revents & ZMQ_POLLIN)
            {
                zmq::multipart_t msg ;
                msg.recv(*m_client) ;
                if (m_verbose == true)
                {
                    s_console("I: received reply") ;
                    dump(msg) ;
                }
                // Don't try to handle errors, just assert noisily
                assert(msg.size() >= 3) ;
                assert(msg.popstr() == std::string(MDP_CLIENT)) ;
                assert(msg.popstr() == std::string(mdps_commands[static_cast<size_t>(WMessageType::REPLY)])) ;
                return msg ;
            }
            if (--retriesLeft == 0)
            {
                if (m_verbose == true)
                {
                    s_console("W: permanent error, abandoning request") ;
                }
                break ;
            }
            if (m_verbose == true)
            {
                s_console("W: no reply, reconnecting") ;
            }
            connectToBroker() ;
            request.clone().send(*m_client) ;
        }
    }
    if (s_interrupted == true)
    {
        s_console("W: interrupt received, killing client...") ;
    }
    return {} ;
}

//------1-------2-------3-------4-------5-------6-------7-------8-------9-------10------11------12------13------14------15------
}  // namespace md1::majordomo