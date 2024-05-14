#ifndef MD1_ZMQMANAGER_EXAMPLES_CLUSTERS_COMMON_HPP
#define MD1_ZMQMANAGER_EXAMPLES_CLUSTERS_COMMON_HPP

#include <string_view>

namespace md1::zmq_clusters {
//------1-------2-------3-------4-------5-------6-------7-------8-------9-------10------11------12------13------14------15------

/**
 * @brief Generate a random number between 0 and max
*/
int genRandom(int max) ;


static constexpr std::string_view WORKER_READY = "\001" ;

//------1-------2-------3-------4-------5-------6-------7-------8-------9-------10------11------12------13------14------15------
}      // namespace md1::zmq_clusters
#endif // MD1_ZMQMANAGER_EXAMPLES_CLUSTERS_COMMON_HPP