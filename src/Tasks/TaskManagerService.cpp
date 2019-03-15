// ===============================================================================
// Authors: AFRL/RQQA
// Organization: Air Force Research Laboratory, Aerospace Systems Directorate, Power and Control Division
// 
// Copyright (c) 2017 Government of the United State of America, as represented by
// the Secretary of the Air Force.  No copyright is claimed in the United States under
// Title 17, U.S. Code.  All Other Rights Reserved.
// ===============================================================================

/* 
 * File:   Component_TaskManager.cpp
 * Author: steve
 * 
 * Created on August 31, 2015, 6:17 PM
 */


#include "TaskManagerService.h"
#include "TaskServiceBase.h"


#include "afrl/cmasi/EntityConfiguration.h"
#include "afrl/cmasi/EntityConfigurationDescendants.h"
#include "afrl/cmasi/EntityState.h"
#include "afrl/cmasi/EntityStateDescendants.h"
#include "afrl/cmasi/AutomationRequest.h"
#include "afrl/cmasi/AutomationResponse.h"
#include "uxas/messages/task/UniqueAutomationRequest.h"
#include "afrl/cmasi/Task.h"
#include "afrl/cmasi/TaskDescendants.h"
#include "afrl/cmasi/RemoveTasks.h"
#include "afrl/cmasi/FollowPathCommand.h"
#include "uxas/messages/uxnative/CreateNewService.h"
#include "uxas/messages/uxnative/KillService.h"
#include "avtas/lmcp/LmcpXMLReader.h"
#include "afrl/cmasi/FollowPathCommand.h"      

#include "afrl/impact/PointOfInterest.h"
#include "afrl/impact/LineOfInterest.h"
#include "afrl/impact/AreaOfInterest.h"



#include "pugixml.hpp"

#include <sstream>      //std::stringstream
#include <iostream>     // std::cout, cerr, etc
#include <fstream>     // std::ifstream
#include <cstdint>
#include <memory>      //int64_t


#define STRING_XML_TYPE "Type"
#define STRING_XML_TASKOPTIONS "TaskOptions"
#define STRING_XML_TASKID "TaskId"
#define STRING_XML_TASKTYPE "TaskType"
#define STRING_XML_OPTION "Option"
#define STRING_XML_OPTIONNAME "OptionName"
#define STRING_XML_VALUE "Value"

#define COUT_INFO_MSG(MESSAGE) std::cout << "<>TaskManager::" << MESSAGE << std::endl;std::cout.flush();
#define COUT_FILE_LINE_MSG(MESSAGE) std::cout << "<>TaskManager::" << __FILE__ << ":" << __LINE__ << ":" << MESSAGE << std::endl;std::cout.flush();
#define CERR_FILE_LINE_MSG(MESSAGE) std::cerr << "<>TaskManager::" << __FILE__ << ":" << __LINE__ << ":" << MESSAGE << std::endl;std::cerr.flush();


namespace uxas
{
namespace service
{
namespace task
{
TaskManagerService::ServiceBase::CreationRegistrar<TaskManagerService>
TaskManagerService::s_registrar(TaskManagerService::s_registryServiceTypeNames());

TaskManagerService::TaskManagerService(std::shared_ptr<uxas::communications::LmcpObjectNetworkClient> pLmcpObjectNetworkClient)
    : ServiceBase(TaskManagerService::s_typeName(), TaskManagerService::s_directoryName(), pLmcpObjectNetworkClient)
{
}

TaskManagerService::~TaskManagerService() { };

bool
TaskManagerService::configure(const pugi::xml_node& ndComponent)

{
    std::string strBasePath = m_workDirectoryPath;
    std::stringstream sstrErrors;

    std::string strComponentType = ndComponent.attribute(STRING_XML_TYPE).value();
    //assert(strComponentType==STRING_XML_COMPONENT_TYPE)

    for (pugi::xml_node ndCurrent = ndComponent.first_child(); ndCurrent; ndCurrent = ndCurrent.next_sibling())
    {
        if (std::string(STRING_XML_TASKOPTIONS) == ndCurrent.name())
        {
            int64_t taskId = 0;
            std::string taskType = m_noTaskTypeString;
            if (!ndCurrent.attribute(STRING_XML_TASKID).empty())
            {
                taskId = ndCurrent.attribute(STRING_XML_TASKID).as_int64();
            }
            else if (!ndCurrent.attribute(STRING_XML_TASKTYPE).empty()) // only use type is there is no taskid
            {
                taskType = ndCurrent.attribute(STRING_XML_TASKTYPE).value();
            }

            std::string taskOptions;

            for (pugi::xml_node ndOption = ndCurrent.first_child(); ndOption; ndOption = ndOption.next_sibling())
            {
                if (std::string(STRING_XML_OPTION) == ndOption.name())
                {
                    if (!ndOption.attribute(STRING_XML_OPTIONNAME).empty())
                    {
                        std::string optionName = ndOption.attribute(STRING_XML_OPTIONNAME).value();
                        std::string value; // value is not required
                        if (!ndOption.attribute(STRING_XML_VALUE).empty())
                        {
                            value = ndOption.attribute(STRING_XML_VALUE).value();
                            taskOptions += "<" + optionName + ">" + value + "</" + optionName + ">";
                        }
                        else
                        {
                            taskOptions += "<" + optionName + "/>";
                        }
                    }
                }
            }
            if (!taskOptions.empty())
            {
                m_TaskIdVsTaskTypeVsOptions[taskId][taskType].push_back(taskOptions);
            }
        } //                    if (std::string(STRING_XML_SUBSCRIBE_TO_MESSAGES) == ndCurrent.name())
    } //for (pugi::xml_node ndCurrent = ndConfigurationEntries.first_child(); ndCurrent; ndCurrent = ndCurrent.next_sibling())

    m_pLmcpObjectNetworkClient->addSubscriptionAddress(afrl::cmasi::RemoveTasks::Subscription);
    
    //ENTITY CONFIGURATIONS
    m_pLmcpObjectNetworkClient->addSubscriptionAddress(afrl::cmasi::EntityConfiguration::Subscription);
    std::vector< std::string > childconfigs = afrl::cmasi::EntityConfigurationDescendants();
    for(auto child : childconfigs)
        m_pLmcpObjectNetworkClient->addSubscriptionAddress(child);
    
    // ENTITY STATES
    m_pLmcpObjectNetworkClient->addSubscriptionAddress(afrl::cmasi::EntityState::Subscription);
    std::vector< std::string > childstates = afrl::cmasi::EntityStateDescendants();
    for(auto child : childstates)
        m_pLmcpObjectNetworkClient->addSubscriptionAddress(child);

    m_pLmcpObjectNetworkClient->addSubscriptionAddress(afrl::cmasi::MissionCommand::Subscription);
    m_pLmcpObjectNetworkClient->addSubscriptionAddress(afrl::cmasi::AutomationResponse::Subscription);
    m_pLmcpObjectNetworkClient->addSubscriptionAddress(afrl::cmasi::FollowPathCommand::Subscription);

    m_pLmcpObjectNetworkClient->addSubscriptionAddress(afrl::impact::AreaOfInterest::Subscription);
    m_pLmcpObjectNetworkClient->addSubscriptionAddress(afrl::impact::LineOfInterest::Subscription);
    m_pLmcpObjectNetworkClient->addSubscriptionAddress(afrl::impact::PointOfInterest::Subscription);
    
    // Subscribe to Task and all derivatives of Task
    m_pLmcpObjectNetworkClient->addSubscriptionAddress(afrl::cmasi::Task::Subscription);
    std::vector< std::string > childtasks = afrl::cmasi::TaskDescendants();
    for(auto child : childtasks)
        m_pLmcpObjectNetworkClient->addSubscriptionAddress(child);

    m_pLmcpObjectNetworkClient->addSubscriptionAddress(afrl::cmasi::KeepInZone::Subscription);
    m_pLmcpObjectNetworkClient->addSubscriptionAddress(afrl::cmasi::KeepOutZone::Subscription);
    m_pLmcpObjectNetworkClient->addSubscriptionAddress(afrl::cmasi::OperatingRegion::Subscription);

    return true;
}

bool
TaskManagerService::processReceivedLmcpMessage(std::unique_ptr<uxas::communications::data::LmcpMessage> receivedLmcpMessage)
//example: if (afrl::cmasi::isServiceStatus(receivedLmcpObject))
{
    std::stringstream sstrError;

    if (afrl::cmasi::isTask(receivedLmcpMessage->m_object))
    {
        auto baseTask = std::static_pointer_cast<afrl::cmasi::Task>(receivedLmcpMessage->m_object);
        bool isGoodTask = true;

        int64_t taskId = baseTask->getTaskID();

        auto itServiceId = m_TaskIdVsServiceId.find(taskId);
        if (itServiceId != m_TaskIdVsServiceId.end())
        {
            // kill the current service
            auto killServiceMessage = std::make_shared<uxas::messages::uxnative::KillService>();
            killServiceMessage->setServiceID(itServiceId->second);
            m_pLmcpObjectNetworkClient->sendSharedLmcpObjectBroadcastMessage(killServiceMessage);
            m_TaskIdVsServiceId.erase(itServiceId);
            UXAS_LOG_WARN("taskID ", taskId, " already exists. Killing previous task");
        }
        //COUT_INFO_MSG("Adding Task[" << taskId << "]")

        std::string taskOptions;

        // FIRST CHECK FOR GLOBAL OPTIONS
        auto itTaskTypeVsOptions = m_TaskIdVsTaskTypeVsOptions.find(0);
        if (itTaskTypeVsOptions != m_TaskIdVsTaskTypeVsOptions.end())
        {
            auto itOptions = itTaskTypeVsOptions->second.find(m_noTaskTypeString);
            if (itOptions != itTaskTypeVsOptions->second.end())
            {
                for (auto& option : itOptions->second)
                {
                    taskOptions += option;
                }
            }
        }

        // NEXT CHECK SPECIFIC OPTIONS 
        itTaskTypeVsOptions = m_TaskIdVsTaskTypeVsOptions.find(taskId);
        if (itTaskTypeVsOptions != m_TaskIdVsTaskTypeVsOptions.end())
        {
            auto itOptions = itTaskTypeVsOptions->second.find(m_noTaskTypeString);
            if (itOptions != itTaskTypeVsOptions->second.end())
            {
                for (auto& option : itOptions->second)
                {
                    taskOptions += option;
                }
            }
        }
        else //if(itTaskTypeVsOptions != m_TaskIdVsTaskTypeVsOptions.end())
        {
            auto itTaskTypeVsOptions = m_TaskIdVsTaskTypeVsOptions.find(0);
            if (itTaskTypeVsOptions != m_TaskIdVsTaskTypeVsOptions.end())
            {
                auto itOptions = itTaskTypeVsOptions->second.find(baseTask->getFullLmcpTypeName());
                if (itOptions != itTaskTypeVsOptions->second.end())
                {
                    for (auto& option : itOptions->second)
                    {
                        taskOptions += option;
                    }
                }
            }
        } //if(itTaskTypeVsOptions != m_TaskIdVsTaskTypeVsOptions.end())

        std::string xmlTaskOptions;
        if (!taskOptions.empty())
        {
            xmlTaskOptions = "<" + TaskServiceBase::m_taskOptions_XmlTag + ">" + taskOptions + "</" + TaskServiceBase::m_taskOptions_XmlTag + ">";
            //COUT_INFO_MSG("INFO:: TaskId[" << taskId << "] xmlTaskOptions[" << xmlTaskOptions << "]")
        }

        auto createNewServiceMessage = std::make_shared<uxas::messages::uxnative::CreateNewService>();
        auto serviceId = uxas::communications::getUniqueId();
        createNewServiceMessage->setServiceID(serviceId);
        std::string xmlConfigStr = "<Service Type=\"" + baseTask->getFullLmcpTypeName() + "\">" +
                " <TaskRequest>" + baseTask->toXML() + "</TaskRequest>\n" + xmlTaskOptions;
        uxas::common::StringUtil::ReplaceAll(xmlConfigStr, "<", "&lt;");
        uxas::common::StringUtil::ReplaceAll(xmlConfigStr, ">", "&gt;");
        createNewServiceMessage->setXmlConfiguration(xmlConfigStr);

        // add all existing entities for new service initialization
        for (auto& entityConfiguration : m_idVsEntityConfiguration)
        {
            createNewServiceMessage->getEntityConfigurations().push_back(entityConfiguration.second->clone());
        }
        
        // add all existing entities for new service initialization
        for (auto& entityState : m_idVsEntityState)
        {
            createNewServiceMessage->getEntityStates().push_back(entityState.second->clone());
        }

        for (auto kiz : m_idVsKeepInZone)
        {
          createNewServiceMessage->getKeepInZones().push_back(kiz.second->clone());
        }
        for (auto koz : m_idVsKeepOutZone)
        {
          createNewServiceMessage->getKeepOutZones().push_back(koz.second->clone());
        }
        for (auto opr : m_idVsOperatingRegion)
        {
          createNewServiceMessage->getOperatingRegions().push_back(opr .second->clone());
        }

        // add the appropriate area/line/point of interest if new task requires knowledge of it
        // TODO: simply send all areas/lines/points to all tasks and let each one find the necessary information
        if (afrl::impact::isAngledAreaSearchTask(receivedLmcpMessage->m_object))
        {
            auto angledAreaSearchTask = std::static_pointer_cast<afrl::impact::AngledAreaSearchTask>(receivedLmcpMessage->m_object);
            auto itAreaOfInterest = m_idVsAreaOfInterest.find(angledAreaSearchTask->getSearchAreaID());
            if (itAreaOfInterest != m_idVsAreaOfInterest.end())
            {
                createNewServiceMessage->getAreas().push_back(itAreaOfInterest->second->clone());
            }
            else
            {
                isGoodTask = false;
                CERR_FILE_LINE_MSG("ERROR:: could not find SearchArea[" << angledAreaSearchTask->getSearchAreaID()
                                   << "] for requested AngledAreaSearchTask[" << taskId << "]")
            }
        }
        else if (afrl::impact::isImpactLineSearchTask(receivedLmcpMessage->m_object))
        {
            auto impactLineSearchTask = std::static_pointer_cast<afrl::impact::ImpactLineSearchTask>(receivedLmcpMessage->m_object);
            auto itLine = m_idVsLineOfInterest.find(impactLineSearchTask->getLineID());
            if (itLine != m_idVsLineOfInterest.end())
            {
                createNewServiceMessage->getLines().push_back(itLine->second->clone());
            }
            else
            {
                isGoodTask = false;
                CERR_FILE_LINE_MSG("ERROR:: could not find Line[" << impactLineSearchTask->getLineID()
                                   << "] for requested ImpactLineSearchTask[" << taskId << "]")
            }
        }
        else if (afrl::impact::isImpactPointSearchTask(receivedLmcpMessage->m_object))
        {
            auto impactPointSearchTask = std::static_pointer_cast<afrl::impact::ImpactPointSearchTask>(receivedLmcpMessage->m_object);
            if (impactPointSearchTask->getSearchLocationID() > 0)
            {
                auto itPoint = m_idVsPointOfInterest.find(impactPointSearchTask->getSearchLocationID());
                if (itPoint != m_idVsPointOfInterest.end())
                {
                    createNewServiceMessage->getPoints().push_back(itPoint->second->clone());
                }
                else
                {
                    isGoodTask = false;
                    CERR_FILE_LINE_MSG("ERROR:: could not find SearchLocation[" << impactPointSearchTask->getSearchLocationID()
                                       << "] for requested ImpactPointSearchTask[" << taskId << "]")
                }
            }
        }
        else if (afrl::impact::isPatternSearchTask(receivedLmcpMessage->m_object))
        {
            auto patternSearchTask = std::static_pointer_cast<afrl::impact::PatternSearchTask>(receivedLmcpMessage->m_object);
            if (patternSearchTask->getSearchLocationID() > 0)
            {
                auto itPoint = m_idVsPointOfInterest.find(patternSearchTask->getSearchLocationID());
                if (itPoint != m_idVsPointOfInterest.end())
                {
                    createNewServiceMessage->getPoints().push_back(itPoint->second->clone());
                }
                else
                {
                    isGoodTask = false;
                    CERR_FILE_LINE_MSG("ERROR:: could not find SearchLocation[" << patternSearchTask->getSearchLocationID()
                                       << "] for requested PatternSearchTask[" << taskId << "]")
                }
            }
        }
        else if (afrl::impact::isEscortTask(receivedLmcpMessage->m_object))
        {
            // escort attempts to determine 'supported entity' route from all lines of interest or mission commands
            for (auto line : m_idVsLineOfInterest)
            {
                createNewServiceMessage->getLines().push_back(line.second->clone());
            }
            for (auto missionCommand : m_vehicleIdVsCurrentMission)
            {
                createNewServiceMessage->getMissionCommands().push_back(missionCommand.second->clone());
            }
        }

        if (isGoodTask)
        {
            m_TaskIdVsServiceId[taskId] = serviceId;
            m_pLmcpObjectNetworkClient->sendSharedLmcpObjectBroadcastMessage(createNewServiceMessage);
            //CERR_FILE_LINE_MSG("Added Task[" << taskId << "]")
        }
    }
    else if (afrl::cmasi::isEntityConfiguration(receivedLmcpMessage->m_object))
    {
        auto entityConfiguration = std::static_pointer_cast<afrl::cmasi::EntityConfiguration>(receivedLmcpMessage->m_object);
        m_idVsEntityConfiguration[entityConfiguration->getID()] = entityConfiguration;
    }
    else if (afrl::cmasi::isEntityState(receivedLmcpMessage->m_object))
    {
        auto entityState = std::static_pointer_cast<afrl::cmasi::EntityState>(receivedLmcpMessage->m_object);
        m_idVsEntityState[entityState->getID()] = entityState;
    }
    else if (afrl::impact::isAreaOfInterest(receivedLmcpMessage->m_object))
    {
        auto areaOfInterest = std::static_pointer_cast<afrl::impact::AreaOfInterest>(receivedLmcpMessage->m_object);
        m_idVsAreaOfInterest[areaOfInterest->getAreaID()] = areaOfInterest;
    }
    else if (afrl::impact::isLineOfInterest(receivedLmcpMessage->m_object))
    {
        auto lineOfInterest = std::static_pointer_cast<afrl::impact::LineOfInterest>(receivedLmcpMessage->m_object);
        m_idVsLineOfInterest[lineOfInterest->getLineID()] = lineOfInterest;
    }
    else if (afrl::impact::isPointOfInterest(receivedLmcpMessage->m_object))
    {
        auto pointOfInterest = std::static_pointer_cast<afrl::impact::PointOfInterest>(receivedLmcpMessage->m_object);
        m_idVsPointOfInterest[pointOfInterest->getPointID()] = pointOfInterest;
    }
    else if (afrl::cmasi::isAutomationRequest(receivedLmcpMessage->m_object))
    {
        auto automationRequest = std::static_pointer_cast<afrl::cmasi::AutomationRequest>(receivedLmcpMessage->m_object);
        auto uniqueAutomationRequest = std::make_shared<uxas::messages::task::UniqueAutomationRequest>();
        uniqueAutomationRequest->setOriginalRequest(automationRequest->clone());
        uniqueAutomationRequest->setRequestID(m_automationRequestId);
        m_automationRequestId++;
        m_pLmcpObjectNetworkClient->sendSharedLmcpObjectBroadcastMessage(uniqueAutomationRequest);
    }
    else if (afrl::cmasi::isAutomationResponse(receivedLmcpMessage->m_object))
    {
        auto ares = std::static_pointer_cast<afrl::cmasi::AutomationResponse>(receivedLmcpMessage->m_object);
        for (auto v : ares->getMissionCommandList())
        {
            m_vehicleIdVsCurrentMission[v->getVehicleID()].reset(v->clone());
        }
    }
    else if (afrl::cmasi::isMissionCommand(receivedLmcpMessage->m_object))
    {
        auto mish = std::static_pointer_cast<afrl::cmasi::MissionCommand>(receivedLmcpMessage->m_object);
        m_vehicleIdVsCurrentMission[mish->getVehicleID()] = mish;
    }
    else if (afrl::cmasi::isKeepInZone(receivedLmcpMessage->m_object))
    {
        auto kiz = std::static_pointer_cast<afrl::cmasi::KeepInZone>(receivedLmcpMessage->m_object);
        m_idVsKeepInZone[kiz->getZoneID()] = kiz;
    }
    else if (afrl::cmasi::isKeepOutZone(receivedLmcpMessage->m_object))
    {
        auto koz = std::static_pointer_cast<afrl::cmasi::KeepOutZone>(receivedLmcpMessage->m_object);
        m_idVsKeepOutZone[koz->getZoneID()] = koz;
    }
    else if (afrl::cmasi::isOperatingRegion(receivedLmcpMessage->m_object))
    {
        auto opr = std::static_pointer_cast<afrl::cmasi::OperatingRegion>(receivedLmcpMessage->m_object);
        m_idVsOperatingRegion[opr ->getID()] = opr ;
    }
    else if (afrl::cmasi::isFollowPathCommand(receivedLmcpMessage->m_object))
    {
        auto fpc = std::static_pointer_cast<afrl::cmasi::FollowPathCommand>(receivedLmcpMessage->m_object);
        auto path = std::make_shared<afrl::cmasi::MissionCommand>();
        for (auto wp : fpc->getWaypointList())
        {
            path->getWaypointList().push_back(wp->clone());
        }
        m_vehicleIdVsCurrentMission[fpc->getVehicleID()] = path;
    }
    else if (afrl::cmasi::isRemoveTasks(receivedLmcpMessage->m_object))
    {
        auto removeTasks = std::static_pointer_cast<afrl::cmasi::RemoveTasks>(receivedLmcpMessage->m_object);
        auto countBefore = m_TaskIdVsServiceId.size();
        for (auto itTaskId = removeTasks->getTaskList().begin(); itTaskId != removeTasks->getTaskList().end(); itTaskId++)
        {
                auto itServiceId = m_TaskIdVsServiceId.find(*itTaskId);
                if (itServiceId != m_TaskIdVsServiceId.end())
                {
                    // kill the current service
                    auto killServiceMessage = std::make_shared<uxas::messages::uxnative::KillService>();
                    killServiceMessage->setServiceID(itServiceId->second);
                    m_pLmcpObjectNetworkClient->sendSharedLmcpObjectBroadcastMessage(killServiceMessage);
                    m_TaskIdVsServiceId.erase(itServiceId);
                    UXAS_LOG_INFORM("Removed Task[", *itTaskId, "]");
                }
                else
                {
                    CERR_FILE_LINE_MSG("ERROR:: Tried to kill service, but could not find ServiceId for TaskId[" << *itTaskId << "]")
                }
        }
        std::string taskList = "[";
        for (auto taskID : removeTasks->getTaskList())
        {
            taskList += std::to_string(taskID) + " ";
        }
        taskList += "]";
        IMPACT_INFORM("Removed ", countBefore - m_TaskIdVsServiceId.size(), " tasks containing ", taskList, ". ", m_TaskIdVsServiceId.size(), " Still Exist.");
    }
    else
    {
        //CERR_FILE_LINE_MSG("WARNING::TaskManagerService::ProcessMessage: MessageType [" << receivedLmcpMessage->m_object->getFullLmcpTypeName() << "] not processed.")
    }
    return (false); // always false implies never terminating service from here
};

std::string TaskManagerService::GetTaskStringIdFromId(const int64_t& taskId)
{
    return ("TASK_" + std::to_string(taskId));
}


}; //namespace task
}; //namespace service
}; //namespace uxas
