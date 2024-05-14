#include "cluster.hpp"

#include <thread>
#include <vector>
#include <string>

#include "broker.hpp"
#include "client.hpp"
#include "worker.hpp"

namespace md1::zmq_clusters {
//------1-------2-------3-------4-------5-------6-------7-------8-------9-------10------11------12------13------14------15------

Cluster::Cluster(const std::string clusterName, const std::vector<std::string>& peerNames, size_t numClients, size_t numWorkers) :
        m_ctx(1),
        m_broker(clusterName, peerNames, m_ctx),
        m_clients(numClients),
        m_workers(numWorkers)
{
    std::thread brokerThread(std::bind(&Broker::run, &m_broker)) ;
    brokerThread.detach() ;

    for (size_t i = 0; i < numClients; ++i)
    {
        m_clients[i] = new Client(clusterName, m_ctx) ;
    }
    for (size_t i = 0; i < numWorkers; ++i)
    {
        m_workers[i] = new Worker(clusterName, m_ctx) ;
    }

    for (auto& client : m_clients)
    {
        std::thread clientThread(std::bind(&Client::run, client)) ;
        clientThread.detach() ;
    }
    for (auto& worker : m_workers)
    {
        std::thread workerThread(std::bind(&Worker::run, worker)) ;
        workerThread.detach() ;
    }
}

Cluster::~Cluster()
{
    for (auto& client : m_clients)
    {
        delete client ;
    }
    for (auto& worker : m_workers)
    {
        delete worker ;
    }
}

//------1-------2-------3-------4-------5-------6-------7-------8-------9-------10------11------12------13------14------15------
}  // namespace md1::zmq_clusters