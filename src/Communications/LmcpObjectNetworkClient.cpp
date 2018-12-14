// ===============================================================================
// Authors: AFRL/RQQA
// Organization: Air Force Research Laboratory, Aerospace Systems Directorate, Power and Control Division
// 
// Copyright (c) 2018 Government of the United State of America, as represented by
// the Secretary of the Air Force.  No copyright is claimed in the United States under
// Title 17, U.S. Code.  All Other Rights Reserved.
// ===============================================================================

#include "LmcpObjectNetworkClient.h"

namespace
{

// leading characters for indicating that an entity ID follows
const std::string entityIdPrefix("eid");

//leading characters for indicating that a service ID follows
const std::string serviceIdPrefix(".sid");

//trailing characters that are appended to entity cast address to form cast-to-all services of a specific entity
const std::string serviceIdAllSuffix(".sidall");

} // namespace

namespace uxas
{
namespace communications
{

int64_t getUniqueId()
{
    static uint64_t nextId = 10;
    return ++nextId;
}

int64_t getUniqueEntitySendMessageId()
{
    static int64_t uniqueEntitySendMessageId = 1;
    return uniqueEntitySendMessageId++;
}

std::string getEntityCastAddress(const uint32_t entityId)
{
    return (entityIdPrefix + std::to_string(entityId));
}

std::string getEntityServicesCastAllAddress(const uint32_t entityId)
{
    return (getEntityCastAddress(entityId) + serviceIdAllSuffix);
}

std::string getNetworkClientUnicastAddress(const uint32_t entityId, const int64_t networkClientId)
{
    return (getEntityCastAddress(entityId) + serviceIdPrefix + std::to_string(networkClientId));
}

std::string getEntityCastAddress(const std::string entityId)
{
    return (entityIdPrefix + entityId);
}

std::string getNetworkClientUnicastAddress(const uint32_t entityId, const std::string networkClientId)
{
    return (getEntityCastAddress(entityId) + serviceIdPrefix + networkClientId);
}

std::string getNetworkClientUnicastAddress(const std::string& entityId, const std::string& networkClientId)
{
    return (getEntityCastAddress(entityId) + serviceIdPrefix + networkClientId);
}

std::string getNetworkClientUnicastAddress(const std::string& entityId, const int64_t& networkClientId)
{
    return (getEntityCastAddress(entityId) + serviceIdPrefix + std::to_string(networkClientId));
}

LmcpObjectNetworkClient::LmcpObjectNetworkClient()
    : m_networkId(getUniqueId()), m_receiveProcessingType(ReceiveProcessingType::LMCP),
    m_isTerminateNetworkClient(false), m_isBaseClassTerminationFinished(false)
{ }

void LmcpObjectNetworkClient::terminate()
{
    m_isTerminateNetworkClient = true;
}

bool LmcpObjectNetworkClient::getIsTerminationFinished() const
{
    return m_isBaseClassTerminationFinished;
}

void LmcpObjectNetworkClient::setMessageSourceGroup(const std::string& messageSourceGroup)
{
    m_messageSourceGroup = messageSourceGroup;
}

void LmcpObjectNetworkClient::setReceiveProcessingType(ReceiveProcessingType type)
{
    m_receiveProcessingType = type;
}

} // namespace communications
} // namespace uxas