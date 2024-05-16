#include <iostream>
#include <string>

#include "mdworker.hpp"

//------1-------2-------3-------4-------5-------6-------7-------8-------9-------10------11------12------13------14------15------

int main(int argc, char* argv[])
{
    bool verbose = false ;
    if (argc > 1)
    {
        if (std::string(argv[1]) == "-v")
        {
            verbose = true ;
        }   
        else
        {
            std::cerr << "Usage: mdclient (-v for verbose)" << std::endl ;
            return 1 ;
        }
    }

    md1::majordomo::Worker worker("tcp://localhost:5555", "echo", verbose) ;

    std::optional<zmq::multipart_t> reply = std::nullopt ;
    while (true)
    {
        auto request = worker.recv(reply) ;
        if (request.size() == 0)
        {
            break ;
        }
        reply = std::move(request) ;
    }
}

//------1-------2-------3-------4-------5-------6-------7-------8-------9-------10------11------12------13------14------15------
