#ifndef MD1_ZMQMANAGER_EXAMPLES_CLUSTERS_WORKER_HPP
#define MD1_ZMQMANAGER_EXAMPLES_CLUSTERS_WORKER_HPP

#include <string>

#include "zmq.hpp"

namespace md1::zmq_clusters {
//------1-------2-------3-------4-------5-------6-------7-------8-------9-------10------11------12------13------14------15------

class Worker
{
public:
    Worker(const std::string& clusterName, zmq::context_t& ctx) ;
    void run() ;

private:
    zmq::socket_t m_worker ;
    const std::string m_clusterName ;
} ;

//------1-------2-------3-------4-------5-------6-------7-------8-------9-------10------11------12------13------14------15------
}      // namespace md1::zmq_clusters
#endif // MD1_ZMQMANAGER_EXAMPLES_CLUSTERS_WORKER_HPP