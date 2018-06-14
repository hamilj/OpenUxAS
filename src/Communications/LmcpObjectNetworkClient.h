// ===============================================================================
// Authors: AFRL/RQQA
// Organization: Air Force Research Laboratory, Aerospace Systems Directorate, Power and Control Division
// 
// Copyright (c) 2018 Government of the United State of America, as represented by
// the Secretary of the Air Force.  No copyright is claimed in the United States under
// Title 17, U.S. Code.  All Other Rights Reserved.
// ===============================================================================

#ifndef UXAS_MESSAGE_LMCP_OBJECT_NETWORK_CLIENT_H
#define UXAS_MESSAGE_LMCP_OBJECT_NETWORK_CLIENT_H

#include <memory>
#include <string>

namespace pugi
{
class xml_node;
} // namespace pugi

namespace avtas
{
namespace lmcp
{
class Object;
} // namespace lmcp
} // namespace avtas

namespace uxas
{
namespace communications
{

namespace data
{
class AddressedAttributedMessage;
} // namespace data

class LmcpObjectMessageProcessor;

class LmcpObjectNetworkClient
{
public:
    /** \class ReceiveProcessingType
     * 
     * \par Enumeration specifying whether or not to de-serialize a received message.
     * 
     * \n
     */
    enum class ReceiveProcessingType
    {
        /** \brief Received <b>LMCP</b> objects are de-serialized */
        LMCP,
        /** \brief Received <b>LMCP</b> objects are not de-serialized */
        SERIALIZED_LMCP
    };

    virtual ~LmcpObjectNetworkClient() { }

    virtual bool configureNetworkClient(const std::string& subclassTypeName, ReceiveProcessingType receiveProcessingType, const pugi::xml_node& networkClientXmlNode) = 0;

    virtual bool initializeAndStart(LmcpObjectMessageProcessor& msgProcessor) = 0;

    virtual bool addSubscriptionAddress(const std::string& address) = 0;

    virtual bool removeSubscriptionAddress(const std::string& address) = 0;

    virtual bool removeAllSubscriptionAddresses() = 0;

    virtual void sendLmcpObjectLimitedCastMessage(const std::string& castAddress, std::unique_ptr<avtas::lmcp::Object> lmcpObject) = 0;

    virtual void sendLmcpObjectBroadcastMessage(std::unique_ptr<avtas::lmcp::Object> lmcpObject) = 0;

    virtual void sendSerializedLmcpObjectMessage(std::unique_ptr<uxas::communications::data::AddressedAttributedMessage> serializedLmcpObject) = 0;

    virtual void sendSharedLmcpObjectBroadcastMessage(const std::shared_ptr<avtas::lmcp::Object>& lmcpObject) = 0;

    virtual void sendSharedLmcpObjectLimitedCastMessage(const std::string& castAddress, const std::shared_ptr<avtas::lmcp::Object>& lmcpObject) = 0;
};

}; // namespace communications
}; // namespace uxas

#endif /* UXAS_MESSAGE_LMCP_OBJECT_NETWORK_CLIENT_H */