#ifndef SERVICE_TESTING_H
#define SERVICE_TESTING_H

#include "avtas/lmcp/Object.h"

#include <initializer_list>
#include <memory>
#include <string>
#include <vector>

namespace uxas
{

namespace communications
{
class LmcpObjectMessageProcessor;
} // namespace communications

namespace service
{
class ServiceBase;
} // namespace service

} // namespace uxas

namespace test
{

namespace utility
{

std::shared_ptr<avtas::lmcp::Object> CreateMessage(const std::string& xml);

template<typename T>
std::shared_ptr<T> CreateMessage(const std::string& xml)
{
    auto msg = CreateMessage(xml);

    // work-around lacking self-id function in interface
    if ((msg != nullptr) &&
        (msg->getSeriesNameAsLong() == T::SeriesId) &&
        (msg->getSeriesVersion() == T::SeriesVersion) &&
        (msg->getLmcpType() == T::TypeId))
    {
        return std::static_pointer_cast<T>(msg);
    }

    return nullptr;
}

} // namespace utility

namespace service
{

bool Configure(uxas::service::ServiceBase& service, const std::string& xml);
bool Configure(uxas::service::ServiceBase& service, const std::string& xml, const std::string& workingDirectory);

// TODO: filter by subscribed when adding (requires client tracking subscriptions)
// processes a message as 'sent' from the specified source entity/service
bool Process(uxas::communications::LmcpObjectMessageProcessor& msgProcessor, const avtas::lmcp::Object& message, uint64_t entityId, uint64_t serviceId, const std::string& messageGroup = "");
bool Process(uxas::communications::LmcpObjectMessageProcessor& msgProcessor, const std::string& xml, uint64_t entityId, uint64_t serviceId, const std::string& messageGroup = "");
bool Process(uxas::communications::LmcpObjectMessageProcessor& msgProcessor, const std::shared_ptr<avtas::lmcp::Object>& message, uint64_t entityId, uint64_t serviceId, const std::string& messageGroup = "");

// given a UxAS database, process the messages from the requested rows in the specified order
// returns true when all specified messages were processed, false on partial or complete failure
bool Process(uxas::communications::LmcpObjectMessageProcessor& msgProcessor, const std::string& database, const std::vector<int>& lines);
bool Process(uxas::communications::LmcpObjectMessageProcessor& msgProcessor, const std::string& database, std::initializer_list<int> lines);

} // namespace service

} // namespace test

#endif // SERVICE_TESTING_H