#ifndef MD1_ZMQMANAGER_EXAMPLES_CLUSTERS_CLUSTER_HPP
#define MD1_ZMQMANAGER_EXAMPLES_CLUSTERS_CLUSTER_HPP

#include <string>
#include <vector>

#include "zmq.hpp"

#include "broker.hpp"
#include "client.hpp"
#include "worker.hpp"

namespace md1::zmq_clusters {
//------1-------2-------3-------4-------5-------6-------7-------8-------9-------10------11------12------13------14------15------

class Cluster
{
public:
    Cluster(const std::string clusterName, const std::vector<std::string>& peerNames, size_t numClients, size_t numWorkers) ;
    ~Cluster() ;

private:
    zmq::context_t m_ctx ;
    Broker m_broker ;
    std::vector<Client*> m_clients ;
    std::vector<Worker*> m_workers ;
} ;

//------1-------2-------3-------4-------5-------6-------7-------8-------9-------10------11------12------13------14------15------
}      // namespace md1::zmq_clusters
#endif // MD1_ZMQMANAGER_EXAMPLES_CLUSTERS_CLUSTER_HPP