#include "client.hpp"

#include <string>
#include <thread>

#include "zmq.hpp"
#include "zmq_addon.hpp"

#include "common.hpp"
#include "zhelpers.hpp"

namespace md1::zmq_clusters {
//------1-------2-------3-------4-------5-------6-------7-------8-------9-------10------11------12------13------14------15------

Client::Client(const std::string& clusterName, zmq::context_t& ctx) :
    m_client(ctx, ZMQ_REQ),
    m_monitor(ctx, ZMQ_PUSH),
    m_clusterName(clusterName)
{
    m_client.connect("ipc://" + m_clusterName + "-localfe.ipc") ;
    m_monitor.connect("ipc://" + m_clusterName + "-monitor.ipc") ;
}

void Client::run()
{
    zmq::pollitem_t items[] = { { m_client, 0, ZMQ_POLLIN, 0 } };

    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(genRandom(5000))) ;
        int burst = genRandom(15) ;
        for (int i = 0; i < burst; ++i)
        {
            // Send request with random hex ID
            std::ostringstream ss ;
            ss << std::setfill('0') << std::setw(4) << std::hex ;
            ss << genRandom(0x10000) ;
            std::string taskID = ss.str() ;
            s_send(m_monitor, "Client sending task " + taskID) ;
            s_send(m_client, taskID) ;

            // Wait for reply
            int rc = zmq::poll(items, 1, std::chrono::seconds(10)) ;
            if (rc == -1)
            {
                break ;  // Interrupted
            }
            if (items[0].revents & ZMQ_POLLIN)
            {
                std::string reply = s_recv(m_client) ;
                if (reply.empty())
                {
                    break ;  // Interrupted
                }
                assert(reply == taskID) ;
                s_send(m_monitor, "Task complete " + reply) ;
            }
            else
            {
                s_send(m_monitor, "ERROR: Client exit - lost task " + taskID) ;
                return ;
            }
        }
    }
}

//------1-------2-------3-------4-------5-------6-------7-------8-------9-------10------11------12------13------14------15------
}  // namespace md1::zmq_clusters