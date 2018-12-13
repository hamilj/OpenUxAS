// ===============================================================================
// Authors: AFRL/RQQA
// Organization: Air Force Research Laboratory, Aerospace Systems Directorate, Power and Control Division
// 
// Copyright (c) 2017 Government of the United State of America, as represented by
// the Secretary of the Air Force.  No copyright is claimed in the United States under
// Title 17, U.S. Code.  All Other Rights Reserved.
// ===============================================================================

#include "LmcpObjectNetworkClientBase.h"

#include "LmcpObjectMessageProcessor.h"

#include "avtas/lmcp/ByteBuffer.h"
#include "avtas/lmcp/Factory.h"
#include "uxas/messages/uxnative/KillService.h"

#include "UxAS_ConfigurationManager.h"
#include "UxAS_Log.h"
#include "Constants/UxAS_String.h"

#include "stdUniquePtr.h"

#include "pugixml.hpp"

#include <functional>

namespace
{

/** \brief The <B><i>deserializeMessage</i></B> method deserializes an LMCP
 * string into an LMCP object.
 *
 * @return unique pointer to LMCP object if succeeds; unique pointer with
 * unassigned native pointer.
 */
std::shared_ptr<avtas::lmcp::Object> deserializeMessage(const std::string& payload)
{
    std::shared_ptr<avtas::lmcp::Object> lmcpObject;

    // allocate memory
    avtas::lmcp::ByteBuffer lmcpByteBuffer;
    lmcpByteBuffer.allocate(payload.size());
    lmcpByteBuffer.rewind();

    for (size_t charIndex = 0; charIndex < payload.size(); charIndex++)
    {
        lmcpByteBuffer.putByte(payload[charIndex]); // TODO REVIEW
    }
    lmcpByteBuffer.rewind();

    lmcpObject.reset(avtas::lmcp::Factory::getObject(lmcpByteBuffer));
    if (!lmcpObject)
    {
        UXAS_LOG_ERROR("LmcpObjectMessageReceiverPipe::deserializeMessage failed to convert message payload string into an LMCP object");
    }

    return (lmcpObject);
}

}

namespace uxas
{
namespace communications
{

std::string LmcpObjectNetworkClientBase::s_entityServicesCastAllAddress 
        = getEntityServicesCastAllAddress(uxas::common::ConfigurationManager::getInstance().getEntityId());

LmcpObjectNetworkClientBase::LmcpObjectNetworkClientBase()
    : LmcpObjectNetworkClient()
{
    m_entityId = uxas::common::ConfigurationManager::getInstance().getEntityId();
    m_entityIdString = std::to_string(m_entityId);
    m_entityType = uxas::common::ConfigurationManager::getInstance().getEntityType();

    UXAS_LOG_INFORM("LmcpObjectNetworkClientBase initializing LMCP network ID ", m_networkId);
};

LmcpObjectNetworkClientBase::~LmcpObjectNetworkClientBase()
{
    if (m_networkClientThread && m_networkClientThread->joinable())
    {
        m_networkClientThread->detach();
    }
};

bool
LmcpObjectNetworkClientBase::configureNetworkClient(const std::string& subclassTypeName, const pugi::xml_node& networkClientXmlNode,
    LmcpObjectMessageProcessor& msgProcessor)
{
    UXAS_LOG_DEBUGGING("LmcpObjectNetworkClientBase::configureNetworkClient method START");
    m_clientName = subclassTypeName;

    //
    // DESIGN 20150911 RJT message addressing - entity ID + service ID (uni-cast)
    // - sent messages always include entity ID and service ID
    // - the network client uni-cast address is derived from entity ID and service ID (see getNetworkClientUnicastAddress function)
    // - network clients always subscribe to the entity ID + service ID uni-cast address on the internal network
    // - bridges never subscribe to the entity ID + service ID uni-cast address on an external network (since already subscribing to the entity cast address)
    //
    // subscribe to messages addressed to network client (bridge, service, etc.)
    auto address = getNetworkClientUnicastAddress(m_entityId, m_networkId);
    UXAS_LOG_INFORM(m_clientName, "::configureNetworkClient subscribing to service uni-cast address [", address, "]");
    addSubscriptionAddress(address);
    s_entityServicesCastAllAddress = getEntityServicesCastAllAddress(m_entityId);
    UXAS_LOG_INFORM(m_clientName, "::configureNetworkClient subscribing to entity cast-to-all services address [", s_entityServicesCastAllAddress, "]");
    addSubscriptionAddress(s_entityServicesCastAllAddress);
    
    // network client can be terminated via received KillService message
    addSubscriptionAddress(uxas::messages::uxnative::KillService::Subscription);

#ifdef DEBUG_VERBOSE_LOGGING_ENABLED_MESSAGING
    std::stringstream xmlNd{""};
    networkClientXmlNode.print(xmlNd);
    UXAS_LOG_DEBUG_VERBOSE_MESSAGING(m_clientName, "::configureNetworkClient calling configure - passing XML ", xmlNd.str());
#endif
    m_isConfigured = msgProcessor.configure(networkClientXmlNode);

    if (m_isConfigured)
    {
        UXAS_LOG_INFORM(m_clientName, "::configureNetworkClient configure call succeeded");
    }
    else
    {
        UXAS_LOG_ERROR(m_clientName, "::configureNetworkClient configure call failed");
    }

    UXAS_LOG_DEBUGGING(m_clientName, "::configureNetworkClient method END");
    return (m_isConfigured);
};

bool
LmcpObjectNetworkClientBase::initializeAndStart(LmcpObjectMessageProcessor& msgProcessor)
{
    UXAS_LOG_DEBUGGING(m_clientName, "::initializeAndStart method START");

    if (m_isConfigured)
    {
        UXAS_LOG_INFORM(m_clientName, "::initializeAndStart started since configureNetworkClient has been called");
    }
    else
    {
        UXAS_LOG_ERROR(m_clientName, "::initializeAndStart failed - must invoke configureNetworkClient method BEFORE calling initializeAndStart");
        return (false);
    }

    if (initializeNetworkClient())
    {
        UXAS_LOG_INFORM(m_clientName, "::initializeAndStart initializeNetworkClient call succeeded");
    }
    else
    {
        UXAS_LOG_ERROR(m_clientName, "::initializeAndStart initializeNetworkClient call failed");
        return (false);
    }

    if (msgProcessor.initialize())
    {
        UXAS_LOG_INFORM(m_clientName, "::initializeAndStart initialize call succeeded");
    }
    else
    {
        UXAS_LOG_ERROR(m_clientName, "::initializeAndStart initialize call failed");
        return (false);
    }

    if (msgProcessor.start())
    {
        UXAS_LOG_INFORM(m_clientName, "::initializeAndStart start call succeeded");
    }
    else
    {
        UXAS_LOG_ERROR(m_clientName, "::initializeAndStart start call failed");
        return (false);
    }

    UXAS_LOG_INFORM(m_clientName, "::initializeAndStart processing thread starting ...");
    switch (m_receiveProcessingType)
    {
        case ReceiveProcessingType::LMCP:
            m_networkClientThread = uxas::stduxas::make_unique<std::thread>(&LmcpObjectNetworkClientBase::executeNetworkClient, this, std::ref(msgProcessor));
            UXAS_LOG_INFORM(m_clientName, "::initializeAndStart started LMCP network client processing thread [", m_networkClientThread->get_id(), "]");
            break;
        case ReceiveProcessingType::SERIALIZED_LMCP:
            m_networkClientThread = uxas::stduxas::make_unique<std::thread>(&LmcpObjectNetworkClientBase::executeSerializedNetworkClient, this, std::ref(msgProcessor));
            UXAS_LOG_INFORM(m_clientName, "::initializeAndStart started LMCP network client serialized processing thread [", m_networkClientThread->get_id(), "]");
            break;
        default:
            UXAS_LOG_ERROR(m_clientName, "::initializeAndStart failed to initialize LMCP network client processing thread; un-handled ReceiveProcessingType case");
    }

    UXAS_LOG_INFORM(m_clientName, "::initializeAndStart processing thread started");

    UXAS_LOG_DEBUGGING(m_clientName, "::initializeAndStart method END");
    return (true);
};

bool
LmcpObjectNetworkClientBase::addSubscriptionAddress(const std::string& address)
{
    UXAS_LOG_DEBUGGING(m_clientName, "::addSubscriptionAddress method START");
    bool isAdded{false};
    if (m_isThreadStarted)
    {
        if (m_lmcpObjectMessageReceiverPipe.addLmcpObjectSubscriptionAddress(address))
        {
            UXAS_LOG_INFORM(m_clientName, "::addSubscriptionAddress subscribed to message address [", address, "]");
        }
        else
        {
            UXAS_LOG_INFORM(m_clientName, "::addSubscriptionAddress attempted to subscribe to message address [", address, "] "
                       " subscription not added since already exists");
        }
    }
    else
    {
        if (m_preStartLmcpSubscriptionAddresses.emplace(address).second)
        {
            UXAS_LOG_INFORM(m_clientName, "::addSubscriptionAddress staged subscribe address [", address, "]");
        }
        else
        {
            UXAS_LOG_INFORM(m_clientName, "::addSubscriptionAddress attempted to stage subscribe address  [", address, "] "
                       " subscription not added since already staged");
        }
    }

    UXAS_LOG_DEBUGGING(m_clientName, "::addSubscriptionAddress method END");
    return (true);
};

bool
LmcpObjectNetworkClientBase::removeSubscriptionAddress(const std::string& address)
{
    bool isRemoved{false};
    if (m_isThreadStarted)
    {
        isRemoved = m_lmcpObjectMessageReceiverPipe.removeLmcpObjectSubscriptionAddress(address);
    }
    else
    {
        isRemoved = ((m_preStartLmcpSubscriptionAddresses.erase(address) > 0) ? true : false);
    }

    if (isRemoved)
    {
        UXAS_LOG_INFORM(m_clientName, "::removeSubscriptionAddress unsubscribed to LMCP message address [", address, "]");
    }
    else
    {
        UXAS_LOG_INFORM(m_clientName, "::removeSubscriptionAddress attempted to unsubscribe to LMCP message address [", address, "] "
                   " subscription not removed since did not exist");
    }

    return false;
};

bool
LmcpObjectNetworkClientBase::removeAllSubscriptionAddresses()
{
    bool isRemoved{false};
    if (m_isThreadStarted)
    {
        isRemoved = m_lmcpObjectMessageReceiverPipe.removeAllLmcpObjectSubscriptionAddresses();
    }
    else
    {
        m_preStartLmcpSubscriptionAddresses.clear();
        isRemoved = true;
    }

    if (isRemoved)
    {
        UXAS_LOG_INFORM(m_clientName, "::removeAllSubscriptionAddresses unsubscribed to all LMCP message addresses");
    }
    else
    {
        UXAS_LOG_INFORM(m_clientName, "::removeAllSubscriptionAddresses cleared all pre-start LMCP message addresses");
    }

    return false;
};

bool
LmcpObjectNetworkClientBase::initializeNetworkClient()
{
    UXAS_LOG_DEBUGGING(m_clientName, "::initializeNetworkClient method START");

    m_lmcpObjectMessageReceiverPipe.initializeSubscription(m_entityId, m_networkId);

    for (const auto& address : m_preStartLmcpSubscriptionAddresses)
    {
        if (m_lmcpObjectMessageReceiverPipe.addLmcpObjectSubscriptionAddress(address))
        {
            UXAS_LOG_INFORM(m_clientName, "::addSubscriptionAddress subscribed to staged message address [", address, "]");
        }
        else
        {
            UXAS_LOG_INFORM(m_clientName, "::addSubscriptionAddress attempted to subscribe to staged message address [", address, "] "
                       " subscription not added since already exists");
        }
    }

    m_lmcpObjectMessageSenderPipe.initializePush(m_messageSourceGroup, m_entityId, m_networkId);

    UXAS_LOG_DEBUGGING(m_clientName, "::initializesendAddressedAttributedMessageNetworkClient method END");
    return (true);
};

void
LmcpObjectNetworkClientBase::executeNetworkClient(LmcpObjectMessageProcessor& msgProcessor)
{
    try
    {
        UXAS_LOG_DEBUGGING(m_clientName, "::executeNetworkClient method START");
        UXAS_LOG_DEBUGGING(m_clientName, "::executeNetworkClient method START infinite while loop");
        m_isThreadStarted = true;
        while (!m_isTerminateNetworkClient)
        {
            try
            {
                // get the next LMCP message (if any) from the LMCP network server
                UXAS_LOG_DEBUG_VERBOSE_MESSAGING(m_clientName, "::executeNetworkClient calling m_lmcpObjectMessageReceiverPipe.getNextMessageObject()");
                std::unique_ptr<uxas::communications::data::LmcpMessage> receivedLmcpMessage
                        = m_lmcpObjectMessageReceiverPipe.getNextMessageObject();
                UXAS_LOG_DEBUG_VERBOSE_MESSAGING(m_clientName, "::executeNetworkClient completed calling m_lmcpObjectMessageReceiverPipe.getNextMessageObject()");

                if (receivedLmcpMessage)
                {
                    UXAS_LOG_DEBUG_VERBOSE_MESSAGING(m_clientName, "::executeNetworkClient processing received LMCP message");
                    UXAS_LOG_DEBUG_VERBOSE_MESSAGING("ContentType:      [", receivedLmcpMessage->m_attributes->getContentType(), "]");
                    UXAS_LOG_DEBUG_VERBOSE_MESSAGING("Descriptor:       [", receivedLmcpMessage->m_attributes->getDescriptor(), "]");
                    UXAS_LOG_DEBUG_VERBOSE_MESSAGING("SourceGroup:      [", receivedLmcpMessage->m_attributes->getSourceGroup(), "]");
                    UXAS_LOG_DEBUG_VERBOSE_MESSAGING("SourceEntityId:   [", receivedLmcpMessage->m_attributes->getSourceEntityId(), "]");
                    UXAS_LOG_DEBUG_VERBOSE_MESSAGING("SourceServiceId:  [", receivedLmcpMessage->m_attributes->getSourceServiceId(), "]");
                    UXAS_LOG_DEBUG_VERBOSE_MESSAGING("AttributesString: [", receivedLmcpMessage->m_attributes->getString(), "]");
                    if (uxas::messages::uxnative::isKillService(receivedLmcpMessage->m_object)
                            && (m_networkId == (std::static_pointer_cast<uxas::messages::uxnative::KillService>(receivedLmcpMessage->m_object)->getServiceID()))
                            || msgProcessor.processReceivedLmcpMessage(std::move(receivedLmcpMessage)))
                    {
                        UXAS_LOG_INFORM(m_clientName, "::executeNetworkClient starting termination since received [", uxas::messages::uxnative::KillService::TypeName, "] message ");
                        m_isTerminateNetworkClient = true;
                    }
                }
            }
            catch (std::exception& ex)
            {
                UXAS_LOG_ERROR(m_clientName, "::executeNetworkClient continuing infinite while loop after EXCEPTION: ", ex.what());
            }
        }
        
        UXAS_LOG_DEBUGGING(m_clientName, "::executeNetworkClient method END infinite while loop");

        m_isBaseClassTerminationFinished = true;

        uint32_t subclassTerminateDuration_ms{0};
        while (true)
        {
            m_isSubclassTerminationFinished = msgProcessor.terminate();
            if (m_isSubclassTerminationFinished)
            {
                UXAS_LOG_INFORM(m_clientName, "::executeNetworkClient terminated subclass processing after [", subclassTerminateDuration_ms, "] milliseconds on thread [", std::this_thread::get_id(), "]");
                break;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(m_subclassTerminationAttemptPeriod_ms));
            subclassTerminateDuration_ms += m_subclassTerminationAttemptPeriod_ms;
            if (subclassTerminateDuration_ms > m_subclassTerminationWarnDuration_ms)
            {
                UXAS_LOG_WARN(m_clientName, "::executeNetworkClient has not terminated subclass processing after [", subclassTerminateDuration_ms, "] milliseconds on thread [", std::this_thread::get_id(), "]");
            }
            else if (subclassTerminateDuration_ms > m_subclassTerminationAbortDuration_ms)
            {
                UXAS_LOG_ERROR(m_clientName, "::executeNetworkClient aborting termination of subclass processing after [", subclassTerminateDuration_ms, "] milliseconds on thread [", std::this_thread::get_id(), "]");
                break;
            }
        }
        UXAS_LOG_INFORM(m_clientName, "::executeNetworkClient exiting infinite loop thread [", std::this_thread::get_id(), "]");
        UXAS_LOG_DEBUGGING(m_clientName, "::executeNetworkClient method END");
    }
    catch (std::exception& ex)
    {
        UXAS_LOG_ERROR(m_clientName, "::executeNetworkClient EXCEPTION: ", ex.what());
    }
};

void
LmcpObjectNetworkClientBase::executeSerializedNetworkClient(LmcpObjectMessageProcessor& msgProcessor)
{
    try
    {
        UXAS_LOG_DEBUGGING(m_clientName, "::executeSerializedNetworkClient method START");
        UXAS_LOG_DEBUGGING(m_clientName, "::executeSerializedNetworkClient method START infinite while loop");
        m_isThreadStarted = true;
        while (!m_isTerminateNetworkClient)
        {
            // get the next serialized LMCP object message (if any) from the LMCP network server
            std::unique_ptr<uxas::communications::data::AddressedAttributedMessage>
                    nextReceivedSerializedLmcpObject
                    = m_lmcpObjectMessageReceiverPipe.getNextSerializedMessage();

            if (nextReceivedSerializedLmcpObject)
            {
                UXAS_LOG_DEBUG_VERBOSE_MESSAGING(m_clientName, "::executeSerializedNetworkClient processing received LMCP message");
                UXAS_LOG_DEBUG_VERBOSE_MESSAGING("Address:          [", nextReceivedSerializedLmcpObject->getAddress(), "]");
                UXAS_LOG_DEBUG_VERBOSE_MESSAGING("ContentType:      [", nextReceivedSerializedLmcpObject->getMessageAttributesReference()->getContentType(), "]");
                UXAS_LOG_DEBUG_VERBOSE_MESSAGING("Descriptor:       [", nextReceivedSerializedLmcpObject->getMessageAttributesReference()->getDescriptor(), "]");
                UXAS_LOG_DEBUG_VERBOSE_MESSAGING("SourceGroup:      [", nextReceivedSerializedLmcpObject->getMessageAttributesReference()->getSourceGroup(), "]");
                UXAS_LOG_DEBUG_VERBOSE_MESSAGING("SourceEntityId:   [", nextReceivedSerializedLmcpObject->getMessageAttributesReference()->getSourceEntityId(), "]");
                UXAS_LOG_DEBUG_VERBOSE_MESSAGING("SourceServiceId:  [", nextReceivedSerializedLmcpObject->getMessageAttributesReference()->getSourceServiceId(), "]");
                UXAS_LOG_DEBUG_VERBOSE_MESSAGING("AttributesString: [", nextReceivedSerializedLmcpObject->getMessageAttributesReference()->getString(), "]");
                UXAS_LOG_DEBUG_VERBOSE_MESSAGING("getPayload:       [", nextReceivedSerializedLmcpObject->getPayload(), "]");
                UXAS_LOG_DEBUG_VERBOSE_MESSAGING("getString:        [", nextReceivedSerializedLmcpObject->getString(), "]");

                if (nextReceivedSerializedLmcpObject->getMessageAttributesReference()->getDescriptor().rfind(uxas::messages::uxnative::KillService::Subscription) != std::string::npos)
                {
                    // reconstitute LMCP object
                    std::shared_ptr<avtas::lmcp::Object> lmcpObject = deserializeMessage(nextReceivedSerializedLmcpObject->getPayload());
                    // check KillService serviceID == my serviceID
                    if (uxas::messages::uxnative::isKillService(lmcpObject)
                            && (m_networkId == (std::static_pointer_cast<uxas::messages::uxnative::KillService>(lmcpObject)->getServiceID())))
                    {
                        UXAS_LOG_INFORM(m_clientName, "::executeSerializedNetworkClient starting termination since received [", uxas::messages::uxnative::KillService::TypeName, "] message ");
                        m_isTerminateNetworkClient = true;
                    }
                }
                else if (msgProcessor.processReceivedSerializedLmcpMessage(std::move(nextReceivedSerializedLmcpObject)))
                {
                    m_isTerminateNetworkClient = true;
                }
            }
        }

        UXAS_LOG_DEBUGGING(m_clientName, "::executeSerializedNetworkClient method END infinite while loop");
        
        m_isBaseClassTerminationFinished = true;

        uint32_t subclassTerminateDuration_ms{0};
        while (true)
        {
            m_isSubclassTerminationFinished = msgProcessor.terminate();
            if (m_isSubclassTerminationFinished)
            {
                UXAS_LOG_INFORM(m_clientName, "::executeSerializedNetworkClient terminated subclass processing after [", subclassTerminateDuration_ms, "] milliseconds on thread [", std::this_thread::get_id(), "]");
                break;
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(m_subclassTerminationAttemptPeriod_ms));
            subclassTerminateDuration_ms += m_subclassTerminationAttemptPeriod_ms;
            if (subclassTerminateDuration_ms > m_subclassTerminationWarnDuration_ms)
            {
                UXAS_LOG_WARN(m_clientName, "::executeSerializedNetworkClient has not terminated subclass processing after [", subclassTerminateDuration_ms, "] milliseconds on thread [", std::this_thread::get_id(), "]");
            }
            else if (subclassTerminateDuration_ms > m_subclassTerminationAbortDuration_ms)
            {
                UXAS_LOG_ERROR(m_clientName, "::executeSerializedNetworkClient aborting termination of subclass processing after [", subclassTerminateDuration_ms, "] milliseconds on thread [", std::this_thread::get_id(), "]");
                m_isSubclassTerminationFinished = true;
                break;
            }
        }
        UXAS_LOG_INFORM(m_clientName, "::executeSerializedNetworkClient exiting infinite loop thread [", std::this_thread::get_id(), "]");
        UXAS_LOG_DEBUGGING(m_clientName, "::executeSerializedNetworkClient method END");
    }
    catch (std::exception& ex)
    {
        UXAS_LOG_ERROR(m_clientName, "::executeSerializedNetworkClient EXCEPTION: ", ex.what());
    }
};

void
LmcpObjectNetworkClientBase::sendLmcpObjectBroadcastMessage(std::unique_ptr<avtas::lmcp::Object> lmcpObject)
{
    uxas::communications::getUniqueEntitySendMessageId();
    m_lmcpObjectMessageSenderPipe.sendBroadcastMessage(std::move(lmcpObject));
};

void
LmcpObjectNetworkClientBase::sendLmcpObjectLimitedCastMessage(const std::string& castAddress, std::unique_ptr<avtas::lmcp::Object> lmcpObject)
{
    uxas::communications::getUniqueEntitySendMessageId();
    m_lmcpObjectMessageSenderPipe.sendLimitedCastMessage(castAddress, std::move(lmcpObject));
};

void
LmcpObjectNetworkClientBase::sendSerializedLmcpObjectMessage(std::unique_ptr<uxas::communications::data::AddressedAttributedMessage> serializedLmcpObject)
{
    uxas::communications::getUniqueEntitySendMessageId();
    m_lmcpObjectMessageSenderPipe.sendSerializedMessage(std::move(serializedLmcpObject));
};

void
LmcpObjectNetworkClientBase::sendSharedLmcpObjectBroadcastMessage(const std::shared_ptr<avtas::lmcp::Object>& lmcpObject)
{
    uxas::communications::getUniqueEntitySendMessageId();
    m_lmcpObjectMessageSenderPipe.sendSharedBroadcastMessage(lmcpObject);
};

void
LmcpObjectNetworkClientBase::sendSharedLmcpObjectLimitedCastMessage(const std::string& castAddress, const std::shared_ptr<avtas::lmcp::Object>& lmcpObject)
{
    uxas::communications::getUniqueEntitySendMessageId();
    m_lmcpObjectMessageSenderPipe.sendSharedLimitedCastMessage(castAddress, lmcpObject);
};

const std::string& LmcpObjectNetworkClientBase::getClientName() const
{
    return m_clientName;
}

}; //namespace communications
}; //namespace uxas
