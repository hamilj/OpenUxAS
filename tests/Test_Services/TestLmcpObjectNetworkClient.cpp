#include "TestLmcpObjectNetworkClient.h"

#include "LmcpObjectMessageProcessor.h"

#include "pugixml.hpp"

namespace test
{

TestLmcpObjectNetworkClient::TestLmcpObjectNetworkClient(uint32_t entityId, const std::string& entityType, int64_t networkId)
{
    m_entityId = entityId;
    m_entityIdString = std::to_string(m_entityId);
    m_entityType = entityType;

    m_networkId = networkId;
}

bool TestLmcpObjectNetworkClient::configureNetworkClient(const std::string& subclassTypeName, const pugi::xml_node& networkClientXmlNode,
    uxas::communications::LmcpObjectMessageProcessor& msgProcessor)
{
    m_networkClientTypeName = subclassTypeName;
    return msgProcessor.configure(networkClientXmlNode);
}

bool TestLmcpObjectNetworkClient::initializeAndStart(uxas::communications::LmcpObjectMessageProcessor& msgProcessor)
{
    if (msgProcessor.initialize())
    {
        return msgProcessor.start();
    }

    return false;
}

bool TestLmcpObjectNetworkClient::addSubscriptionAddress(const std::string& address)
{
    // TODO
    return true;
}

bool TestLmcpObjectNetworkClient::removeSubscriptionAddress(const std::string& address)
{
    // TODO
    return true;
}

bool TestLmcpObjectNetworkClient::removeAllSubscriptionAddresses()
{
    // TODO
    return true;
}

void TestLmcpObjectNetworkClient::sendLmcpObjectLimitedCastMessage(const std::string& castAddress, std::unique_ptr<avtas::lmcp::Object> lmcpObject)
{
    m_msgQueue.emplace(castAddress, std::move(lmcpObject));
}

void TestLmcpObjectNetworkClient::sendLmcpObjectBroadcastMessage(std::unique_ptr<avtas::lmcp::Object> lmcpObject)
{
    m_msgQueue.emplace(std::string(), std::move(lmcpObject));
}

void TestLmcpObjectNetworkClient::sendSerializedLmcpObjectMessage(std::unique_ptr<uxas::communications::data::AddressedAttributedMessage> serializedLmcpObject)
{
    // TODO
}

void TestLmcpObjectNetworkClient::sendSharedLmcpObjectBroadcastMessage(const std::shared_ptr<avtas::lmcp::Object>& lmcpObject)
{
    m_msgQueue.emplace(std::string(), lmcpObject);
}

void TestLmcpObjectNetworkClient::sendSharedLmcpObjectLimitedCastMessage(const std::string& castAddress, const std::shared_ptr<avtas::lmcp::Object>& lmcpObject)
{
    m_msgQueue.emplace(castAddress, lmcpObject);
}

bool TestLmcpObjectNetworkClient::empty()
{
    return m_msgQueue.empty();
}

} // namespace test