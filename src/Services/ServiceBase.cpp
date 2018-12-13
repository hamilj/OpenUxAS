// ===============================================================================
// Authors: AFRL/RQQA
// Organization: Air Force Research Laboratory, Aerospace Systems Directorate, Power and Control Division
// 
// Copyright (c) 2017 Government of the United State of America, as represented by
// the Secretary of the Air Force.  No copyright is claimed in the United States under
// Title 17, U.S. Code.  All Other Rights Reserved.
// ===============================================================================

#include "ServiceBase.h"

#include "LmcpObjectNetworkClient.h"

#include "UxAS_ConfigurationManager.h"
#include "Constants/UxAS_String.h"

#include "FileSystemUtilities.h"

#include <sstream>

namespace uxas
{
namespace service
{

ServiceBase::ServiceBase(const std::string& serviceType, const std::string& workDirectoryName,
    std::shared_ptr<uxas::communications::LmcpObjectNetworkClient> pLmcpObjectNetworkClient)
    : m_serviceType(serviceType), m_workDirectoryName(workDirectoryName), m_pLmcpObjectNetworkClient(pLmcpObjectNetworkClient)
{ }

bool
ServiceBase::configureService(const std::string& parentOfWorkDirectory, const std::string& serviceXml)
{
    pugi::xml_document xmlDoc;
    if (xmlDoc.load(serviceXml.c_str()))
    {
    	return (configureService(parentOfWorkDirectory, xmlDoc.root()));
	}

    return false;
}

bool
ServiceBase::configureService(const std::string& parentWorkDirectory, const pugi::xml_node& serviceXmlNode)
{
    UXAS_LOG_DEBUGGING(m_serviceType, "::configureService method START");
    bool isSuccess{false};

    if (!m_workDirectoryName.empty())
    {
        
        m_workDirectoryPath = parentWorkDirectory + ((*(parentWorkDirectory.rbegin()) == '/') ? "" : "/")
                + m_workDirectoryName + ((*(m_workDirectoryName.rbegin()) == '/') ? "" : "/");

    }
    else
    {
        m_workDirectoryPath = "";
    }

    isSuccess = m_pLmcpObjectNetworkClient->configureNetworkClient(m_serviceType, serviceXmlNode, *this);

    //
    // DESIGN 20150911 RJT message addressing - service group (multi-cast)
    // - sent messages always include source group value (empty or non-empty)
    // - services can have empty or non-empty source group value
    // - bridges always have empty source group value
    // - services with non-empty source group value are automatically subscribed to the source group value (to receive multi-cast messages)
    //
    if (!serviceXmlNode.attribute(uxas::common::StringConstant::MessageGroup().c_str()).empty())
    {
        // set source group value that will be assigned to source group field of sent messages
        const std::string group = serviceXmlNode.attribute(uxas::common::StringConstant::MessageGroup().c_str()).value();
        m_pLmcpObjectNetworkClient->setMessageSourceGroup(group);
        UXAS_LOG_INFORM(m_serviceType, "::configureService setting messageSourceGroup to [", group, "] from XML configuration");
        // subscribe to messages addressed to non-empty source group value
        if (!group.empty())
        {
            m_pLmcpObjectNetworkClient->addSubscriptionAddress(group);
        }
    }
    else
    {
        UXAS_LOG_INFORM(m_serviceType, "::configureService did not find ", uxas::common::StringConstant::MessageGroup(), " value in XML configuration");
    }
    
    if (isSuccess)
    {
        m_isConfigured = true;
        UXAS_LOG_INFORM(m_serviceType, "::configureService succeeded - service ID ", getServiceId());
    }
    else
    {
        UXAS_LOG_ERROR(m_serviceType, "::configureService failed - service ID ", getServiceId());
    }

    UXAS_LOG_DEBUGGING(m_serviceType, "::configureService method END");
    return (isSuccess);
}

bool
ServiceBase::initializeAndStartService()
{
    bool isSuccess{false};

    if (m_isConfigured)
    {
        if (!m_workDirectoryPath.empty())
        {
            std::stringstream errors;
            isSuccess = uxas::common::utilities::c_FileSystemUtilities::bCreateDirectory(m_workDirectoryPath, errors);
            if (isSuccess)
            {
                UXAS_LOG_INFORM(m_serviceType, "::initializeAndStartService created work directory ", m_workDirectoryPath, " - service ID ", getServiceId());
            }
            else
            {
                UXAS_LOG_ERROR(m_serviceType, "::initializeAndStartService failed to create work directory ", m_workDirectoryPath, " - service ID ", getServiceId());
            }
        }
        else
        {
            isSuccess = true;
            UXAS_LOG_INFORM(m_serviceType, "::initializeAndStartService skipping work directory creation - service ID ", getServiceId());
        }

        if (isSuccess)
        {
            isSuccess = m_pLmcpObjectNetworkClient->initializeAndStart(*this);
        }

        if (isSuccess)
        {
            UXAS_LOG_INFORM(m_serviceType, "::initializeAndStartService succeeded - service ID ", getServiceId());
        }
        else
        {
            UXAS_LOG_ERROR(m_serviceType, "::initializeAndStartService failed - service ID ", getServiceId());
        }
    }
    else
    {
        UXAS_LOG_ERROR(m_serviceType, "::initializeAndStartService failed since configure method has not been invoked");
    }

    return (isSuccess);
}

void ServiceBase::updateNetworkId(int64_t networkId)
{
    m_pLmcpObjectNetworkClient->removeSubscriptionAddress(uxas::communications::getNetworkClientUnicastAddress(getEntityId(), m_pLmcpObjectNetworkClient->m_networkId));
    m_pLmcpObjectNetworkClient->m_networkId = networkId;
    m_pLmcpObjectNetworkClient->addSubscriptionAddress(uxas::communications::getNetworkClientUnicastAddress(getEntityId(), m_pLmcpObjectNetworkClient->m_networkId));
}

bool ServiceBase::getIsTerminationFinished() const
{
    return m_pLmcpObjectNetworkClient->getIsTerminationFinished();
}

uint32_t ServiceBase::getEntityId() const
{
    return m_pLmcpObjectNetworkClient->m_entityId;
}

const std::string& ServiceBase::getEntityIdString() const
{
    return m_pLmcpObjectNetworkClient->m_entityIdString;
}

uint32_t ServiceBase::getServiceId() const
{
    return m_pLmcpObjectNetworkClient->m_networkId;
}

} //namespace service
} //namespace uxas
