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

#include "avtas/lmcp/Object.h"

#include <atomic>
#include <memory>
#include <string>

namespace pugi
{
class xml_node;
} // namespace pugi

namespace uxas
{
namespace communications
{

int64_t getUniqueId();

int64_t getUniqueEntitySendMessageId();

/** \brief Multi-cast entity-based subscription address string 
 * 
 * @param entityId UxAS entity ID.
 * @return address string to used to send a message to all services hosted by 
 * a particular UxAS entity.
 */
std::string getEntityCastAddress(const uint32_t entityId);

/** \brief Multi-cast entity-based subscription address string 
 * 
 * @param entityId UxAS entity ID.
 * @return address string to used to send a message to all services hosted by 
 * a particular UxAS entity.
 */
std::string getEntityCastAddress(const std::string entityId);

/** \brief Multi-cast subscription address string that addresses a message 
 * to all services of a specific entity.
 * 
 * @param entityId UxAS entity ID.
 * @return address string to used to send a message to all services hosted by 
 * a particular UxAS entity.
 */
std::string getEntityServicesCastAllAddress(const uint32_t entityId);

/** \brief Uni-cast service-based subscription address string 
 * 
 * @param entityId UxAS entity ID.
 * @param networkClientId UxAS bridge or service ID.
 * @return address string to used to send a message to a specific service 
 * hosted by a particular UxAS entity.
 */
std::string getNetworkClientUnicastAddress(const uint32_t entityId, const int64_t networkClientId);

/** \brief Uni-cast service-based subscription address string 
 * 
 * @param entityId UxAS entity ID.
 * @param networkClientId UxAS bridge or service ID.
 * @return address string to used to send a message to a specific service 
 * hosted by a particular UxAS entity.
 */
std::string getNetworkClientUnicastAddress(const uint32_t entityId, const std::string networkClientId);

/** \brief Uni-cast service-based subscription address string 
 * 
 * @param entityId UxAS entity ID.
 * @param networkClientId UxAS bridge or service ID.
 * @return address string to used to send a message to a specific service 
 * hosted by a particular UxAS entity.
 */
std::string getNetworkClientUnicastAddress(const std::string& entityId, const std::string& networkClientId);

std::string getNetworkClientUnicastAddress(const std::string& entityId, const int64_t& networkClientId);

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

    LmcpObjectNetworkClient();

    virtual ~LmcpObjectNetworkClient() { }

    virtual bool configureNetworkClient(const std::string& subclassTypeName, ReceiveProcessingType receiveProcessingType,
        const pugi::xml_node& networkClientXmlNode, LmcpObjectMessageProcessor& msgProcessor) = 0;

    virtual bool initializeAndStart(LmcpObjectMessageProcessor& msgProcessor) = 0;

    virtual bool addSubscriptionAddress(const std::string& address) = 0;

    virtual bool removeSubscriptionAddress(const std::string& address) = 0;

    virtual bool removeAllSubscriptionAddresses() = 0;

    virtual void sendLmcpObjectLimitedCastMessage(const std::string& castAddress, std::unique_ptr<avtas::lmcp::Object> lmcpObject) = 0;

    virtual void sendLmcpObjectBroadcastMessage(std::unique_ptr<avtas::lmcp::Object> lmcpObject) = 0;

    virtual void sendSerializedLmcpObjectMessage(std::unique_ptr<uxas::communications::data::AddressedAttributedMessage> serializedLmcpObject) = 0;

    virtual void sendSharedLmcpObjectBroadcastMessage(const std::shared_ptr<avtas::lmcp::Object>& lmcpObject) = 0;

    virtual void sendSharedLmcpObjectLimitedCastMessage(const std::string& castAddress, const std::shared_ptr<avtas::lmcp::Object>& lmcpObject) = 0;

    inline bool getIsTerminationFinished()
    {
        return(m_isBaseClassTerminationFinished && m_isSubclassTerminationFinished);
    }

    /** \brief Unique ID for UxAS entity instance; value read from configuration XML */
    uint32_t m_entityId;

    /** \brief String representation of the unique ID for UxAS entity instance; value read from configuration XML */
    std::string m_entityIdString;

    /** \brief Type of UxAS entity instance; value read from configuration XML */
    std::string m_entityType;

    /** \brief Unique ID of the <b>LMCP</b> object communication network actor (e.g., bridge or service). */
    int64_t m_networkId;

    /** \brief String representation of the unique ID of the <b>LMCP</b> object communication network actor (e.g., bridge or service). */
    std::string m_networkIdString;

    /** \brief Name of subclass used for logging/messaging. */
    std::string m_networkClientTypeName;

    /** \brief Unicast message address for messaging case of sending message to only this network client instance */
    std::string m_entityIdNetworkIdUnicastString;

    /** \brief Multi-cast group address that is subscribed to and included in sent messages  */
    std::string m_messageSourceGroup;

    std::atomic<bool> m_isBaseClassKillServiceProcessingPermitted;
    std::atomic<bool> m_isTerminateNetworkClient;
    std::atomic<bool> m_isBaseClassTerminationFinished;
    std::atomic<bool> m_isSubclassTerminationFinished;

    uint32_t m_subclassTerminationAbortDuration_ms;
    uint32_t m_subclassTerminationWarnDuration_ms;
    uint32_t m_subclassTerminationAttemptPeriod_ms;
};

}; // namespace communications
}; // namespace uxas

#endif /* UXAS_MESSAGE_LMCP_OBJECT_NETWORK_CLIENT_H */