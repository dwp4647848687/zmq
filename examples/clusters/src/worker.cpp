#include "worker.hpp"

#include <chrono>
#include <string>
#include <thread>

#include "zmq.hpp"
#include "zmq_addon.hpp"

#include "common.hpp"
#include "zhelpers.hpp"

namespace md1::zmq_clusters {
//------1-------2-------3-------4-------5-------6-------7-------8-------9-------10------11------12------13------14------15------

Worker::Worker(const std::string& clusterName, zmq::context_t& ctx) :
    m_worker(ctx, ZMQ_REQ),
    m_clusterName(clusterName)
{
    m_worker.connect("ipc://" + m_clusterName + "-localbe.ipc") ;
}

void Worker::run()
{
    s_send(m_worker, std::string(WORKER_READY)) ;

    while (true)
    {
        zmq::multipart_t request ;
        request.recv(m_worker) ;
        if (request.empty())
        {
            break ;  // Interrupted
        }

        // Do some 'work'
        std::this_thread::sleep_for(std::chrono::milliseconds(genRandom(2000))) ;
        request.send(m_worker) ;
    }
}

//------1-------2-------3-------4-------5-------6-------7-------8-------9-------10------11------12------13------14------15------
}  // namespace md1::zmq_clusters