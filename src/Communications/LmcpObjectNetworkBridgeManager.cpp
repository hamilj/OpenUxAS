// ===============================================================================
// Authors: AFRL/RQQA
// Organization: Air Force Research Laboratory, Aerospace Systems Directorate, Power and Control Division
// 
// Copyright (c) 2017 Government of the United State of America, as represented by
// the Secretary of the Air Force.  No copyright is claimed in the United States under
// Title 17, U.S. Code.  All Other Rights Reserved.
// ===============================================================================

#include "LmcpObjectNetworkBridgeManager.h"

#include "LmcpObjectNetworkSerialBridge.h"
#include "LmcpObjectNetworkTcpBridge.h"
#include "LmcpObjectNetworkSubscribePushBridge.h"
#include "LmcpObjectNetworkPublishPullBridge.h"
#include "LmcpObjectNetworkZeroMqZyreBridge.h"

#include "UxAS_ConfigurationManager.h"
#include "Constants/UxAS_String.h"
#include "UxAS_Log.h"
#include "uxas/messages/uxnative/KillService.h"

#include "stdUniquePtr.h"

#include "LmcpObjectNetworkSerialBridge.h"
#include "ImpactSubscribePushBridge.h"

#include "pugixml.hpp"

#include <fstream>
#include <stdexcept>

namespace
{

std::unique_ptr<uxas::communications::LmcpObjectNetworkBridge> createBridge(const std::string& type, const pugi::xml_node& bridgeXmlNode)
{
    // attempt instantiation of new bridge
    std::unique_ptr<uxas::communications::LmcpObjectNetworkBridge> newBridgeFinal;
    if (uxas::common::StringConstant::Bridge().compare(bridgeXmlNode.name()) == 0)
    {
        std::string bridgeType = bridgeXmlNode.attribute(uxas::common::StringConstant::Type().c_str()).value();
        UXAS_LOG_INFORM(type, "::createBridge adding ", bridgeType);

        std::unique_ptr<uxas::communications::LmcpObjectNetworkBridge> newBridge;
        if (uxas::communications::LmcpObjectNetworkSerialBridge::s_typeName().compare(bridgeType) == 0)
        {
            newBridge = uxas::stduxas::make_unique<uxas::communications::LmcpObjectNetworkSerialBridge>();
        }
        else if (uxas::communications::LmcpObjectNetworkTcpBridge::s_typeName().compare(bridgeType) == 0)
        {
            newBridge = uxas::stduxas::make_unique<uxas::communications::LmcpObjectNetworkTcpBridge>();
        }
        else if (uxas::communications::LmcpObjectNetworkSubscribePushBridge::s_typeName().compare(bridgeType) == 0)
        {
            newBridge = uxas::stduxas::make_unique<uxas::communications::LmcpObjectNetworkSubscribePushBridge>();
        }
        else if (uxas::communications::LmcpObjectNetworkPublishPullBridge::s_typeName().compare(bridgeType) == 0)
        {
            newBridge = uxas::stduxas::make_unique<uxas::communications::LmcpObjectNetworkPublishPullBridge>();
        }
        else if (uxas::communications::LmcpObjectNetworkZeroMqZyreBridge::s_typeName().compare(bridgeType) == 0)
        {
            newBridge = uxas::stduxas::make_unique<uxas::communications::LmcpObjectNetworkZeroMqZyreBridge>();
        }
        else if (uxas::communications::ImpactSubscribePushBridge::s_typeName().compare(bridgeType) == 0)
        {
            newBridge = uxas::stduxas::make_unique<uxas::communications::ImpactSubscribePushBridge>();
        }
        else
        {
            UXAS_LOG_ERROR(type, "::createBridge cannot construct ", bridgeType);
        }

        if (newBridge)
        {
            UXAS_LOG_INFORM(type, "::createBridge instantiated bridge ", bridgeType, " network ID ", newBridge->getNetworkId());

            if (newBridge->configureNetworkClient(bridgeType, bridgeXmlNode, *newBridge))
            {
                UXAS_LOG_INFORM(type, "::createBridge configured ", newBridge->getClientName(), " entity ID ", newBridge->getEntityId(), " network ID ", newBridge->getNetworkId());
                if (newBridge->initializeAndStart(*newBridge))
                {
                    newBridgeFinal = std::move(newBridge);
                    UXAS_LOG_INFORM(type, "::createBridge initialized and started ", newBridgeFinal->getClientName(), " network ID ", newBridgeFinal->getNetworkId());
                }
                else
                {
                    UXAS_LOG_ERROR(type, "::createBridge failed to initialize and start ", newBridge->getClientName(), " network ID ", newBridge->getNetworkId());
                    newBridge.reset();
                }
            }
            else
            {
                UXAS_LOG_ERROR(type, "::createBridge failed to configure ", newBridge->getClientName(), " network ID ", newBridge->getNetworkId());
            }
        }
        else
        {
            UXAS_LOG_WARN(type, "::createBridge failed to instantiate ", bridgeType);
        }
    }
    else
    {
        UXAS_LOG_WARN(type, "::createBridge ignoring ", bridgeXmlNode.name(), " XML node - expecting ", uxas::common::StringConstant::Bridge(), " XML node");
    }

    return (newBridgeFinal);
}

} // namespace

namespace uxas
{
namespace communications
{

std::unique_ptr<LmcpObjectNetworkBridgeManager> LmcpObjectNetworkBridgeManager::s_instance = nullptr;

LmcpObjectNetworkBridgeManager&
LmcpObjectNetworkBridgeManager::getInstance()
{
    // first time/one time creation
    if (LmcpObjectNetworkBridgeManager::s_instance == nullptr)
    {
        s_instance.reset(new LmcpObjectNetworkBridgeManager);
    }

    return *s_instance;
};

void
LmcpObjectNetworkBridgeManager::terminateAllBridges(std::shared_ptr<uxas::communications::LmcpObjectNetworkClient> pLmcpObjectNetworkClient)
{
    for (auto svcIt = m_bridgesByNetworkId.cbegin(), serviceItEnd = m_bridgesByNetworkId.cend(); svcIt != serviceItEnd; svcIt++)
    {
        if (svcIt->second && !svcIt->second->getIsTerminationFinished())
        {
            UXAS_LOG_INFORM(s_typeName(), "::terminateAllBridges sending [", uxas::messages::uxnative::KillService::TypeName, "] message to ", svcIt->second->getClientName(), " having entity ID [", svcIt->second->getEntityId(), "] and network ID [", svcIt->second->getNetworkId(), "]");

            std::cout << std::endl << s_typeName() << "::terminateAllBridges sending [" << uxas::messages::uxnative::KillService::TypeName << "] message to " << svcIt->second->getClientName() << " having entity ID [" << svcIt->second->getEntityId() << "] and network ID [" << svcIt->second->getNetworkId() << "]" << std::endl;
            auto killService = uxas::stduxas::make_unique<uxas::messages::uxnative::KillService>();
            killService->setServiceID(svcIt->second->getNetworkId());
            pLmcpObjectNetworkClient->sendLmcpObjectLimitedCastMessage(getNetworkClientUnicastAddress(svcIt->second->getEntityId(), svcIt->second->getNetworkId()), std::move(killService));
        }
        else
        {
            UXAS_LOG_INFORM(s_typeName(), "::terminateAllBridges unexpectedly found empty pointer (hosting a bridge object)");
        }
    }
}

void
LmcpObjectNetworkBridgeManager::removeTerminatedBridges(uint32_t &runningSvcCnt, uint32_t &terminatedSvcCnt)
{
    for (auto svcIt = m_bridgesByNetworkId.begin(); svcIt != m_bridgesByNetworkId.end();)
    {
        if (svcIt->second->getIsTerminationFinished())
        {
            UXAS_LOG_INFORM(s_typeName(), "::removeTerminatedServices removing reference to terminated ", svcIt->second->getClientName(), " ID ", svcIt->second->getNetworkId());
            terminatedSvcCnt++;
            svcIt = m_bridgesByNetworkId.erase(svcIt); // remove finished service (enables destruction)
        }
        else
        {
            UXAS_LOG_DEBUGGING(s_typeName(), "::removeTerminatedServices retaining reference to non-terminated ", svcIt->second->getClientName(), " ID ", svcIt->second->getNetworkId());
            runningSvcCnt++;
            svcIt++;
        }
    }
}

bool
LmcpObjectNetworkBridgeManager::initialize()
{
    bool isSuccess{false};
    if (!m_isInitializedBridges)
    {
        uint32_t addedBridgeXmlNodeCount = 0;
        uint32_t failedBridgeXmlNodeCount = 0;
        pugi::xml_node uxasEnabledBridgesXml = uxas::common::ConfigurationManager::getInstance().getEnabledBridges();
        if (!uxasEnabledBridgesXml.empty())
        {
            for (pugi::xml_node bridgeNode = uxasEnabledBridgesXml.first_child(); bridgeNode; bridgeNode = bridgeNode.next_sibling())
            {
                std::unique_ptr<LmcpObjectNetworkBridge> newBridge = createBridge(s_typeName(), bridgeNode);
                if (newBridge)
                {
                    addedBridgeXmlNodeCount++;
                    m_bridgesByNetworkId.emplace(newBridge->getNetworkId(), std::move(newBridge));
                    UXAS_LOG_INFORM(s_typeName(), "::initialize number of added bridges is ", addedBridgeXmlNodeCount);
                }
                else
                {
                    failedBridgeXmlNodeCount++;
                    UXAS_LOG_WARN(s_typeName(), "::initialize number of failed bridge add attempts is ", failedBridgeXmlNodeCount);
                }
            }
        }
        else
        {
            UXAS_LOG_WARN(s_typeName(), "::initialize did not find any enabled bridges");
        }
        m_isInitializedBridges = true;
        isSuccess = true;
    }
    else
    {
        UXAS_LOG_WARN(s_typeName(), "::initialize ignoring second attempt to instantiate bridges from XML");
    }

    return (isSuccess);
}

} //namespace communications
} //namespace uxas
