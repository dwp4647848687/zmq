#include "broker.hpp"

#include <string>
#include <vector>
#include <iostream>

#include "zmq.hpp"
#include "zmq_addon.hpp"

#include "common.hpp"
#include "zhelpers.hpp"

namespace md1::zmq_clusters {
//------1-------2-------3-------4-------5-------6-------7-------8-------9-------10------11------12------13------14------15------

Broker::Broker(const std::string& clusterName, const std::vector<std::string>& otherClusters, zmq::context_t& ctx) :
        m_localfe(ctx, ZMQ_ROUTER),
        m_localbe(ctx, ZMQ_ROUTER),
        m_cloudfe(ctx, ZMQ_ROUTER),
        m_cloudbe(ctx, ZMQ_ROUTER),
        m_statefe(ctx, ZMQ_SUB),
        m_statebe(ctx, ZMQ_PUB),
        m_monitor(ctx, ZMQ_PULL),
        m_clusterName(clusterName),
        m_otherClusters(otherClusters)
{
    // Local routing
    m_localfe.bind("ipc://" + m_clusterName + "-localfe.ipc") ;
    m_localbe.bind("ipc://" + m_clusterName + "-localbe.ipc") ;

    // Cloud routing
    m_cloudfe.bind("ipc://" + m_clusterName + "-cloud.ipc") ;
    m_cloudbe.setsockopt(ZMQ_IDENTITY, m_clusterName.c_str(), m_clusterName.size()) ;
    for (const std::string& cluster : m_otherClusters)
    {
        std::cout << "Info: connecting to cloud frontend at '" << cluster << "'\n" ;
        m_cloudbe.connect("ipc://" + cluster + "-cloud.ipc") ;
    }
    // State
    m_statefe.setsockopt(ZMQ_SUBSCRIBE, "", 0) ;
    for (const std::string& cluster : m_otherClusters)
    {
        std::cout << "Info: connecting to state backend at '" << cluster << "'\n" ;
        m_statefe.connect("ipc://" + cluster + "-state.ipc") ;
    }
    m_statebe.bind("ipc://" + m_clusterName + "-state.ipc") ;

    // Monitor
    m_monitor.bind("ipc://" + m_clusterName + "-monitor.ipc") ;
}
    
void Broker::run()
{
    zmq::pollitem_t primaryItems[] = { { m_localbe, 0, ZMQ_POLLIN, 0 },
                                           { m_cloudbe, 0, ZMQ_POLLIN, 0 },
                                           { m_statefe, 0, ZMQ_POLLIN, 0 },
                                           { m_monitor, 0, ZMQ_POLLIN, 0 } } ;

    zmq::pollitem_t clientItems[] = { { m_localfe, 0, ZMQ_POLLIN, 0 },
                                        { m_cloudfe, 0, ZMQ_POLLIN, 0 } } ;

    while (true)
    {
        std::chrono::duration waitTime = std::chrono::milliseconds(m_localCapacity > 0 ? 1000 : -1) ;
        int rc = zmq::poll(primaryItems, 4, waitTime) ;
        if (rc == -1)
        {
            break ;  // Interrupted
        }
        int previousLocalCapacity = m_localCapacity ;
        pollWorkers(primaryItems) ;
        pollState(primaryItems) ;
        pollLog(primaryItems) ;

        pollClients(clientItems) ;

        if (previousLocalCapacity != m_localCapacity)
        {
            zmq::multipart_t message ;
            message.addstr(m_clusterName) ;
            message.addstr(std::to_string(m_localCapacity)) ;
            message.send(m_statebe) ;
        }
    }
}

void Broker::pollWorkers(zmq::pollitem_t* items)
    {
    zmq::multipart_t message ;
    if (items[0].revents & ZMQ_POLLIN)
    {
        message.recv(m_localbe) ;
        if (message.empty())
        {
            return ;  // Interrupted
        }
        zmq::message_t workerID = message.pop() ;
        m_availableWorkers.push(std::move(workerID)) ;
        ++m_localCapacity ;
        message.pop() ;  // Pop the delimiter
        // Check if this is a worker READY message
        if (message.peekstr(0) == std::string(WORKER_READY))
        {
            message.pop() ;
            std::cout << "Worker is ready" << std::endl ;
        }
    }
    else if (items[1].revents & ZMQ_POLLIN)
    {
        message.recv(m_cloudbe) ;
        if (message.empty())
        {
            return ;  // Interrupted
        }
        message.pop() ;  // Pop the remote cluster ID
        message.pop() ;  // Pop the delimiter

    }
    if (message.empty() == false)
    {
        const std::string& firstFrame = message.peekstr(0) ;
        if (std::find (m_otherClusters.begin(), m_otherClusters.end(), firstFrame) != m_otherClusters.end())
        {
            message.send(m_cloudfe) ;
        }
        else
        {
            message.send(m_localfe) ;
        }
    }
}

void Broker::pollState(zmq::pollitem_t* items)
{
    if (items[2].revents & ZMQ_POLLIN)
    {
        zmq::multipart_t message ;
        message.recv(m_statefe) ;
        if (message.empty())
        {
            return ;  // Interrupted
        }
        std::string clusterID = message.popstr() ;
        size_t capacity = std::stoul(message.popstr()) ;
        m_cloudCapacities[clusterID] = capacity ;
    }
}

void Broker::pollLog(zmq::pollitem_t* items)
{
    if (items[3].revents & ZMQ_POLLIN)
    {
        std::cout << s_recv(m_monitor) << std::endl ;
    }
}

void Broker::pollClients(zmq::pollitem_t* items)
{
    size_t totalCapacity = getTotalCapacity() ;
    while (totalCapacity > 0)
    {
        int rc = m_localCapacity > 0 ? zmq::poll(items, 2, std::chrono::milliseconds(0))
                                        : zmq::poll(items, 1, std::chrono::milliseconds(0)) ;
        if (rc == -1)
        {
            return ;  // Interrupted
        }
        zmq::multipart_t message ;
        if (items[0].revents & ZMQ_POLLIN)
        {
            message.recv(m_localfe) ;
        }
        else if (items[1].revents & ZMQ_POLLIN)
        {
            message.recv(m_cloudfe) ;
            std::cout << "Received task " << message.peekstr(2)
                        << " from cloud cluster " << message.peekstr(0) << std::endl ;
        }
        else
        {
            return ;
        }

        if (message.empty())
        {
            return ;  // Interrupted
        }
        message.push(zmq::message_t()) ;  // Empty delimiter
        if (m_localCapacity > 0)
        {
            message.push(std::move(m_availableWorkers.front())) ;
            m_availableWorkers.pop() ;
            --m_localCapacity ;
            message.send(m_localbe) ;
        }
        else
        {
            auto max_it = std::max_element(m_cloudCapacities.begin(), m_cloudCapacities.end(),
                [](const auto& a, const auto& b) {
                    return a.second < b.second;
                }
            ) ;
            std::string bestCluster = max_it->first;
            std::cout << "No local workers, forwarding task " << message.peekstr(3)
                        << " to cloud cluster " << bestCluster << std::endl ;
            message.pushstr(bestCluster) ;
        }
        totalCapacity = getTotalCapacity() ;
    }
}

size_t Broker::getTotalCapacity()
{
    size_t totalCapacity = m_localCapacity ;
    for (const auto& [_, capacity] : m_cloudCapacities)
    {
        totalCapacity += capacity ;
    }
    return totalCapacity ;
}

//------1-------2-------3-------4-------5-------6-------7-------8-------9-------10------11------12------13------14------15------
}  // namespace md1::zmq_clusters