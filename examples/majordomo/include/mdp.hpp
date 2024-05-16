#ifndef MD1_ZMQMANAGER_EXAMPLES_MAJORDOMO_MDP_HPP
#define MD1_ZMQMANAGER_EXAMPLES_MAJORDOMO_MDP_HPP

#include <chrono>
#include <iostream>
#include <string_view>

#include "zmq.hpp"
#include "zmq_addon.hpp"

namespace md1::majordomo {
//------1-------2-------3-------4-------5-------6-------7-------8-------9-------10------11------12------13------14------15------

static constexpr std::string_view MDP_CLIENT = "MDPC01" ;
static constexpr std::string_view MDP_WORKER = "MDPW01" ;

static constexpr uint32_t HEARTBEAT_LIVENESS = 3 ;
static constexpr uint32_t HEARTBEAT_INTERVAL =  2500 ;    //  msecs
static constexpr uint32_t HEARTBEAT_EXPIRY =  HEARTBEAT_INTERVAL * HEARTBEAT_LIVENESS ;    //  msecs

using Duration = std::chrono::duration<int64_t, std::milli> ;
using TimePoint = std::chrono::time_point<std::chrono::system_clock> ;

enum class WMessageType : uint8_t
{
    READY = 1,
    REQUEST = 2,
    REPLY = 3,
    HEARTBEAT = 4,
    DISCONNECT = 5
} ;

static constexpr std::string_view mdps_commands[] = {
    "",
    "READY",
    "REQUEST",
    "REPLY",
    "HEARTBEAT",
    "DISCONNECT"
} ;

inline void dump(const zmq::multipart_t& msg)
{
    std::cout << "---------------------------------------------" << "\n" ;
    for (auto itr = msg.cbegin(); itr != msg.cend(); ++itr)
    {
        std::cout << "[" << itr->size() << "] " << itr->str() << "\n" ;
    }
    std::cout << std::endl ;
}

//------1-------2-------3-------4-------5-------6-------7-------8-------9-------10------11------12------13------14------15------
}       // namespace md1::majordomo
#endif  // MD1_ZMQMANAGER_EXAMPLES_MAJORDOMO_MDP_HPP