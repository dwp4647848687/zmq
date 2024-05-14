#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "cluster.hpp"

int main(int argc, char* argv[])
{
    // Check valid arguments then parse them
    if (argc < 2)
    {
        std::cerr << "Usage: peering3 <num_clients> <numworkers> <clustername> <peername1> ...\n" ;
        return 1 ;
    }
    size_t numClients = std::stoul(argv[1]) ;
    size_t numWorkers = std::stoul(argv[2]) ;
    std::string clusterName = argv[3] ;
    std::vector<std::string> peerNames ;
    for (int i = 4; i < argc; ++i)
    {
        peerNames.push_back(argv[i]) ;
    }
    md1::zmq_clusters::Cluster cluster(clusterName, peerNames, numClients, numWorkers) ;
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1)) ;
    }
}