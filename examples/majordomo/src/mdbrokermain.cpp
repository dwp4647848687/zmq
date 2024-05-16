#include <iostream>
#include <string>

#include "mdbroker.hpp"
#include "zhelpers.hpp"

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
            std::cerr << "Usage: mdbroker (-v for verbose)" << std::endl ;
            return 1 ;
        }
    }

    s_version_assert(4, 0) ;
    s_catch_signals() ;
    md1::majordomo::Broker brk(verbose) ;
    brk.bind("tcp://*:5555") ;
    brk.startBrokering() ;

    if (s_interrupted)
    {
        std::cout << "W: interrupt received, shutting down..." << std::endl ;
    }
}

//------1-------2-------3-------4-------5-------6-------7-------8-------9-------10------11------12------13------14------15------