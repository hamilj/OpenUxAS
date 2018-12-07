#include "ServiceTesting.h"

#include "LmcpMessage.h"
#include "LmcpObjectMessageProcessor.h"
#include "MessageAttributes.h"
#include "ServiceBase.h"
#include "stdUniquePtr.h"

#include "Constants/UxAS_String.h"

#include "avtas/lmcp/LmcpXMLReader.h"

#include <SQLiteCpp/Database.h>
#include <SQLiteCpp/SQLiteCpp.h>

namespace test
{

namespace utility
{

std::shared_ptr<avtas::lmcp::Object> CreateMessage(const std::string& xml)
{
    return std::shared_ptr<avtas::lmcp::Object>(avtas::lmcp::xml::readXML(xml));
}

} // namespace utility

namespace service
{

bool Configure(uxas::service::ServiceBase& service, const std::string& xml)
{
    return Configure(service, xml, "");
}

bool Configure(uxas::service::ServiceBase& service, const std::string& xml, const std::string& workingDirectory)
{
    if (service.configureService(workingDirectory, xml))
    {
        return service.initializeAndStartService();
    }

    return false;
}

bool Process(uxas::communications::LmcpObjectMessageProcessor& msgProcessor, const avtas::lmcp::Object& message, uint64_t entityId, uint64_t serviceId, const std::string& messageGroup)
{
    auto attributes = uxas::stduxas::make_unique<uxas::communications::data::MessageAttributes>();
    attributes->setAttributes(uxas::common::ContentType::lmcp(), message.getFullLmcpTypeName(), messageGroup, std::to_string(entityId), std::to_string(serviceId));

    auto lmcpObject = std::unique_ptr<avtas::lmcp::Object>(message.clone());

    return !msgProcessor.processReceivedLmcpMessage(uxas::stduxas::make_unique<uxas::communications::data::LmcpMessage>(std::move(attributes), std::move(lmcpObject)));
}

bool Process(uxas::communications::LmcpObjectMessageProcessor& msgProcessor, const std::string& xml, uint64_t entityId, uint64_t serviceId, const std::string& messageGroup)
{
    auto message = test::utility::CreateMessage(xml);

    if (message != nullptr)
    {
        return Process(msgProcessor, *message, entityId, serviceId, messageGroup);
    }

    return false;
}

bool Process(uxas::communications::LmcpObjectMessageProcessor& msgProcessor, const std::shared_ptr<avtas::lmcp::Object>& message, uint64_t entityId, uint64_t serviceId, const std::string& messageGroup)
{
    if (message != nullptr)
    {
        return Process(msgProcessor, *message, entityId, serviceId, messageGroup);
    }

    return false;
}

bool Process(uxas::communications::LmcpObjectMessageProcessor& msgProcessor, const std::string& database, const std::vector<int>& lines)
{
    bool bResult = true;

    try
    {
        SQLite::Database db(database);

        // using single queries rather than 'IN' for matching 'id' (batched single query) to directly fulfill user-specified message order
        SQLite::Statement query(db, "SELECT groupID, entityID, serviceID, xml FROM msg WHERE id = ?");

        for (auto line : lines)
        {
            query.reset();
            query.bind(1, line);

            if (query.executeStep())
            {
                const std::string groupId = query.getColumn(0);
                const int64_t entityId    = query.getColumn(1);
                const int64_t serviceId   = query.getColumn(2);
                const std::string xml     = query.getColumn(3);

                bResult = Process(msgProcessor, xml, static_cast<uint64_t>(entityId), static_cast<uint64_t>(serviceId), groupId);
            }
            else
            {
                bResult = false;
            }

            if (!bResult)
            {
                break;
            }
        }
    }
    catch (std::exception& e)
    {
        bResult = false;
    }

    return bResult;
}

bool Process(uxas::communications::LmcpObjectMessageProcessor& msgProcessor, const std::string& database, std::initializer_list<int> lines)
{
    return Process(msgProcessor, database, std::vector<int>(lines));
}

} // namespace service
} // namespace test