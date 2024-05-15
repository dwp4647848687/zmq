#ifndef MD1_ZMQMANAGER_EXAMPLES_MAJORDOMO_CLIENT_HPP
#define MD1_ZMQMANAGER_EXAMPLES_MAJORDOMO_CLIENT_HPP

#include <string>
#include <memory>
#include <cstdint>
#include <chrono>

#include "zmq.hpp"
#include "zmq_addon.hpp"

namespace md1::majordomo {
//------1-------2-------3-------4-------5-------6-------7-------8-------9-------10------11------12------13------14------15------

class Client
{
public:
    Client(const std::string& brokerAddr, bool verbose, std::shared_ptr<zmq::context_t> ctx = nullptr) ;
    virtual ~Client() = default ;

    /**
     * @brief Set how long the client should wait for a reply before retrying.
    */
    void setTimeOut(Duration timeout) ;

    /**
     * @brief Set how many times the client should retry sending a request before giving up.
    */
    void setRetries(size_t retries) ;

    /**
     * @brief Send a request to the broker.
     * 
     * @param service The service that the broker should route the request to.
     * @param request The request to send to the broker.
     * 
     * @return zmq::multipart_t The reply from the broker.
    */
    zmq::multipart_t send(const std::string& service, zmq::multipart_t&& request) ;

private:
    void connectToBroker() ;

    const std::string m_brokerAddr ;
    const bool m_verbose ;
    std::shared_ptr<zmq::context_t> m_ctx ;
    std::unique_ptr<zmq::socket_t> m_client = nullptr ;
    Duration m_timeout = std::chrono::milliseconds(2500) ;
    size_t m_retries = 3 ;
} ;

//------1-------2-------3-------4-------5-------6-------7-------8-------9-------10------11------12------13------14------15------
}       // namespace md1::majordomo
#endif  // MD1_ZMQMANAGER_EXAMPLES_MAJORDOMO_CLIENT_HPP