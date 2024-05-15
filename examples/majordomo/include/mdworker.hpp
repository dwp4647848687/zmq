#ifndef MD1_ZMQMANAGER_EXAMPLES_MAJORDOMO_WORKER_HPP
#define MD1_ZMQMANAGER_EXAMPLES_MAJORDOMO_WORKER_HPP

#include <chrono>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>

#include "zmq.hpp"
#include "zmq_addon.hpp"

#include "mdp.hpp"

namespace md1::majordomo {

class Worker
{
public:
    Worker(const std::string& brokerAddr, const std::string& service, bool verbose, std::shared_ptr<zmq::context_t> ctx = nullptr) ;
    virtual ~Worker() = default ;
    
    void setHeartbeat(Duration heartbeat) ;
    void setReconnect(Duration reconnect) ;
    zmq::multipart_t recv(std::optional<zmq::multipart_t>&& reply) ;

private:
    void connectToBroker() ;
    void sendToBroker(WMessageType type, std::optional<zmq::multipart_t>&& msg) ;
    
    static constexpr uint32_t HEARTBEAT_LIVENESS = 3 ;

    const std::string m_brokerAddr ;
    const std::string m_service ;
    const bool m_verbose ;

    std::shared_ptr<zmq::context_t> m_ctx ;
    std::unique_ptr<zmq::socket_t> m_worker = nullptr ;
    size_t m_liveness = HEARTBEAT_LIVENESS ;
    TimePoint m_nextHeartbeat ;
    Duration m_heartbeat = std::chrono::milliseconds(2500) ;
    Duration m_reconnect = std::chrono::milliseconds(2500) ;

    bool m_expectingReply = false ;
    std::string m_replyAddr ;
} ;

}       // namespace md1::majordomo
#endif  // MD1_ZMQMANAGER_EXAMPLES_MAJORDOMO_WORKER_HPP