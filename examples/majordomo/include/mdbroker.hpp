#ifndef MD1_ZMQMANAGER_EXAMPLES_MAJORDOMO_BROKER_HPP
#define MD1_ZMQMANAGER_EXAMPLES_MAJORDOMO_BROKER_HPP

#include <chrono>
#include <deque>
#include <list>
#include <memory>
#include <map>
#include <optional>
#include <set>
#include <string>

#include "zmq.hpp"
#include "zmq_addon.hpp"

#include "mdp.hpp"

namespace md1::majordomo {
//------1-------2-------3-------4-------5-------6-------7-------8-------9-------10------11------12------13------14------15------

struct Service ;

struct Worker
{
    Worker(const std::string& identity, Service* service = nullptr, TimePoint expiry = TimePoint()) :
            m_identity(identity),
            m_service(service),
            m_expiry(expiry)
    {}

    std::string m_identity ;
    Service* m_service ;
    TimePoint m_expiry ;
} ;

struct Service
{
    Service(const std::string& name) :
            m_name(name)
    {}

    std::string m_name ;
    std::deque<zmq::multipart_t> m_requests ;
    std::list<Worker*> m_waitingWorkers ;
    size_t m_numWorkers
} ;

//------1-------2-------3-------4-------5-------6-------7-------8-------9-------10------11------12------13------14------15------

class Broker
{
public:
    Broker(bool verbose, std::shared_ptr<zmq::context_t> ctx = nullptr) ;
    virtual ~Broker() = default ;

    void bind(const std::string& endpoint) ;
    void startBrokering() ;

private:
    Service& getService(const std::string& name) ;

    Worker& getWorker(const std::string& identity) ;
    void removeWorker(const std::string& name, bool disconnect) ;
    void purgeWorkers() ;

    void dispatch(Service& service, std::optional<zmq::multipart_t>&& msg) ;
    void handleInternalService(const std::string& service, zmq::multipart_t&& msg) ;

    void processWorkerMessage(Worker& worker, zmq::multipart_t&& msg) ;
    void sendWorkerMessage(Worker& worker, WMessageType type, std::optional<zmq::multipart_t>&& msg) ;
    void markWorkerAsWaiting(Worker& worker) ;

    void processClientMessage(const std::string& clientName, zmq::multipart_t&& msg) ;

    std::shared_ptr<zmq::context_t> m_ctx ;
    std::unique_ptr<zmq::socket_t> m_socket = nullptr ;
    const bool m_verbose ;
    std::string m_endpoint ;
    std::map<std::string, Service> m_services ;
    std::map<std::string, Worker> m_workers ;
    std::set<Worker*> m_waitingWorkers ;
} ;

//------1-------2-------3-------4-------5-------6-------7-------8-------9-------10------11------12------13------14------15------
}       // namespace md1::majordomo
#endif  // MD1_ZMQMANAGER_EXAMPLES_MAJORDOMO_BROKER_HPP