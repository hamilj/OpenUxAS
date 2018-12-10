#ifndef TEST_LMCP_OBJECT_NETWORK_CLIENT_H
#define TEST_LMCP_OBJECT_NETWORK_CLIENT_H

#include "LmcpObjectNetworkClient.h"
#include "stdUniquePtr.h"

#include <memory>
#include <queue>
#include <string>
#include <utility>

namespace uxas
{
namespace communications
{
namespace data
{
class LmcpObjectMessageProcessor;
} // namespace data
} // namespace communications
} // namespace uxas

namespace test
{

template<typename T>
struct AddressedMessage
{
    AddressedMessage(std::string address, const std::shared_ptr<T> msg)
        : address(address), msg(msg) { }

    std::string address;
    const std::shared_ptr<T> msg;
};

class TestLmcpObjectNetworkClient : public uxas::communications::LmcpObjectNetworkClient
{
public:
    TestLmcpObjectNetworkClient(uint32_t entityId, const std::string& entityType, int64_t networkId);

    bool configureNetworkClient(const std::string& subclassTypeName, const pugi::xml_node& networkClientXmlNode,
        uxas::communications::LmcpObjectMessageProcessor& msgProcessor) override;

    bool initializeAndStart(uxas::communications::LmcpObjectMessageProcessor& msgProcessor) override;

    bool addSubscriptionAddress(const std::string& address) override;

    bool removeSubscriptionAddress(const std::string& address) override;

    bool removeAllSubscriptionAddresses() override;

    void sendLmcpObjectLimitedCastMessage(const std::string& castAddress, std::unique_ptr<avtas::lmcp::Object> lmcpObject) override;

    void sendLmcpObjectBroadcastMessage(std::unique_ptr<avtas::lmcp::Object> lmcpObject) override;

    void sendSerializedLmcpObjectMessage(std::unique_ptr<uxas::communications::data::AddressedAttributedMessage> serializedLmcpObject) override;

    void sendSharedLmcpObjectBroadcastMessage(const std::shared_ptr<avtas::lmcp::Object>& lmcpObject) override;

    void sendSharedLmcpObjectLimitedCastMessage(const std::string& castAddress, const std::shared_ptr<avtas::lmcp::Object>& lmcpObject) override;

    bool empty();

    // removes and returns the oldest address/message, if present, downcasting to the requested type
    // or nullptr on failure with no removal
    template<typename T>
    std::unique_ptr<AddressedMessage<T>> get()
    {
        if (!m_msgQueue.empty())
        {
            auto head = m_msgQueue.front();

            // work-around lacking self-id function in interface
            if ((head.second->getSeriesNameAsLong() == T::SeriesId) &&
                (head.second->getSeriesVersion() == T::SeriesVersion) &&
                (head.second->getLmcpType() == T::TypeId))
            {
                auto msg = uxas::stduxas::make_unique<AddressedMessage<T>>(head.first, std::static_pointer_cast<T>(head.second));
                m_msgQueue.pop();
                return msg;
            }
        }

        return nullptr;
    }

private:
    std::queue<std::pair<std::string, const std::shared_ptr<avtas::lmcp::Object>>> m_msgQueue;
};

} // namespace test

#endif // TEST_LMCP_OBJECT_NETWORK_CLIENT_H