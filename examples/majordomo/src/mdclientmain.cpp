#include <iostream>
#include <string>

#include "zmq.hpp"
#include "zmq_addon.hpp"

#include "mdclient.hpp"

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
    md1::majordomo::Client client("tcp://localhost:5555", verbose) ;

    size_t count = 0 ;
    for ( ; count < 100000 ; ++count)
    {
        auto reply = client.send("echo", zmq::multipart_t("Hello world")) ;
        if (reply.size() == 0)
        {
            break ;
        }
    }
    std::cout << count << " requests/replies processed" << std::endl ;
}

//------1-------2-------3-------4-------5-------6-------7-------8-------9-------10------11------12------13------14------15------
