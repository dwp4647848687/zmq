#ifndef MD1_ZMQMANAGER_EXAMPLES_CLUSTERS_BROKER_HPP
#define MD1_ZMQMANAGER_EXAMPLES_CLUSTERS_BROKER_HPP

#include <string>
#include <vector>
#include <queue>
#include <map>

#include "zmq.hpp"

namespace md1::zmq_clusters {
//------1-------2-------3-------4-------5-------6-------7-------8-------9-------10------11------12------13------14------15------

class Broker
{
public:
    Broker(const std::string& clusterName, const std::vector<std::string>& otherClusters, zmq::context_t& ctx) ;
    void run() ;

private:
    /**
     * @brief Polls the local and cloud workers for completed work.
    */
    void pollWorkers(zmq::pollitem_t* items) ;

    /**
     * @brief Polls the states of other clusters to determine if they have available workers.
    */
    void pollState(zmq::pollitem_t* items) ;

    /**
     * @brief Polls the monitor socket for messages to log.
    */
    void pollLog(zmq::pollitem_t* items) ;

    /**
     * @brief Polls the local and cloud clients for work to be done.
    */
    void pollClients(zmq::pollitem_t* items) ;

    /**
     * @brief Calculates the total capacity of both local and cloud workers.
    */
    size_t getTotalCapacity() ;

    zmq::socket_t m_localfe ;
    zmq::socket_t m_localbe ;
    zmq::socket_t m_cloudfe ;
    zmq::socket_t m_cloudbe ;
    zmq::socket_t m_statefe ;
    zmq::socket_t m_statebe ;
    zmq::socket_t m_monitor ;
    const std::string m_clusterName ;
    const std::vector<std::string> m_otherClusters ;

    size_t m_localCapacity = 0 ;
    std::map<std::string, size_t> m_cloudCapacities ;
    std::queue<zmq::message_t> m_availableWorkers ;
} ;

//------1-------2-------3-------4-------5-------6-------7-------8-------9-------10------11------12------13------14------15------
}      // namespace md1::zmq_clusters
#endif // MD1_ZMQMANAGER_EXAMPLES_CLUSTERS_BROKER_HPP