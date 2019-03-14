// ===============================================================================
// Authors: AFRL/RQQA
// Organization: Air Force Research Laboratory, Aerospace Systems Directorate, Power and Control Division
// 
// Copyright (c) 2017 Government of the United State of America, as represented by
// the Secretary of the Air Force.  No copyright is claimed in the United States under
// Title 17, U.S. Code.  All Other Rights Reserved.
// ===============================================================================

/* 
 * File:   Component_TaskTracker.cpp
 * Author: derek
 * 
 * Created on Aug 2, 2015, 8:21 AM
 */

#include "TaskTrackerService.h"

#include "UxAS_Log.h"
#include "pugixml.hpp"
#include "UnitConversions.h"
#include "uxas/messages/task/UXTASK.h"

#include "afrl/cmasi/Task.h"
#include "afrl/cmasi/TaskDescendants.h"

#include "afrl/cmasi/EntityState.h"
#include "afrl/cmasi/EntityStateDescendants.h"

#include "afrl/impact/PointOfInterest.h"
#include "afrl/impact/LineOfInterest.h"
#include "afrl/impact/AreaOfInterest.h"

#include <map>
#include <algorithm>

namespace uxas
{
namespace service
{
namespace task
{
TaskTrackerService::ServiceBase::CreationRegistrar<TaskTrackerService>
TaskTrackerService::s_registrar(TaskTrackerService::s_registryServiceTypeNames());

TaskTrackerService::TaskTrackerService(std::shared_ptr<uxas::communications::LmcpObjectNetworkClient> pLmcpObjectNetworkClient)
: ServiceBase(TaskTrackerService::s_typeName(), TaskTrackerService::s_directoryName(), pLmcpObjectNetworkClient) { };

TaskTrackerService::~TaskTrackerService() { };

bool
TaskTrackerService::configure(const pugi::xml_node& serviceXmlNode)
{
    (void)serviceXmlNode; // -Wunused-parameter

    // track all vehicles
    m_pLmcpObjectNetworkClient->addSubscriptionAddress(afrl::cmasi::EntityState::Subscription);
    std::vector< std::string > childstates = afrl::cmasi::EntityStateDescendants();
    for(auto child : childstates)
        m_pLmcpObjectNetworkClient->addSubscriptionAddress(child);

    m_pLmcpObjectNetworkClient->addSubscriptionAddress(afrl::impact::AreaOfInterest::Subscription);
    m_pLmcpObjectNetworkClient->addSubscriptionAddress(afrl::impact::LineOfInterest::Subscription);
    m_pLmcpObjectNetworkClient->addSubscriptionAddress(afrl::impact::PointOfInterest::Subscription);
    
    // Subscribe to Task and all derivatives of Task
    m_pLmcpObjectNetworkClient->addSubscriptionAddress(afrl::cmasi::Task::Subscription);
    std::vector< std::string > childtasks = afrl::cmasi::TaskDescendants();
    for(auto child : childtasks)
        m_pLmcpObjectNetworkClient->addSubscriptionAddress(child);

    // Register tasks that vehicles are attempting
    m_pLmcpObjectNetworkClient->addSubscriptionAddress(afrl::cmasi::AutomationResponse::Subscription);
    m_pLmcpObjectNetworkClient->addSubscriptionAddress(afrl::cmasi::MissionCommand::Subscription);

    return true;
}

bool
TaskTrackerService::processReceivedLmcpMessage(std::unique_ptr<uxas::communications::data::LmcpMessage> receivedLmcpMessage)
{
    if (afrl::cmasi::isEntityState(receivedLmcpMessage->m_object))
    {
        auto entityState = std::static_pointer_cast<afrl::cmasi::EntityState>(receivedLmcpMessage->m_object);
        m_states[entityState->getID()] = entityState;
        HandleVehicleState(entityState);
    }
    else if (afrl::cmasi::isAutomationResponse(receivedLmcpMessage->m_object))
    {
        auto request = std::static_pointer_cast<afrl::cmasi::AutomationResponse>(receivedLmcpMessage->m_object);
        for (size_t k = 0; k < request->getMissionCommandList().size(); k++)
        {
            std::shared_ptr<afrl::cmasi::MissionCommand> mish(request->getMissionCommandList().at(k)->clone());
            HandleMissionCommand(mish);
        }
    }
    else if (afrl::cmasi::isMissionCommand(receivedLmcpMessage->m_object))
    {
        std::shared_ptr<afrl::cmasi::MissionCommand> mish(static_cast<afrl::cmasi::MissionCommand*>(receivedLmcpMessage->m_object->clone()));
        HandleMissionCommand(mish);
    }
    else if (afrl::cmasi::isLoiterTask(receivedLmcpMessage->m_object) ||
            afrl::cmasi::isMustFlyTask(receivedLmcpMessage->m_object) ||
            afrl::cmasi::isAreaSearchTask(receivedLmcpMessage->m_object) ||
            afrl::cmasi::isLineSearchTask(receivedLmcpMessage->m_object) ||
            afrl::cmasi::isPointSearchTask(receivedLmcpMessage->m_object) ||
            afrl::impact::isImpactPointSearchTask(receivedLmcpMessage->m_object) ||
            afrl::impact::isImpactLineSearchTask(receivedLmcpMessage->m_object) ||
            afrl::impact::isPatternSearchTask(receivedLmcpMessage->m_object) ||
            afrl::impact::isAngledAreaSearchTask(receivedLmcpMessage->m_object) ||
            afrl::impact::isCommRelayTask(receivedLmcpMessage->m_object) ||
            afrl::impact::isBlockadeTask(receivedLmcpMessage->m_object) ||
            afrl::impact::isCordonTask(receivedLmcpMessage->m_object) ||
            afrl::impact::isEscortTask(receivedLmcpMessage->m_object) ||
            afrl::impact::isMultiVehicleWatchTask(receivedLmcpMessage->m_object) ||
            afrl::impact::isWatchTask(receivedLmcpMessage->m_object))
    {
        auto task = std::static_pointer_cast<afrl::cmasi::Task>(receivedLmcpMessage->m_object);
        m_tasks.insert(task->getTaskID());

        // check for task complete eligibility
        if (afrl::cmasi::isLoiterTask(receivedLmcpMessage->m_object))
        {
            // loiter only eligible if non-infinite duration
            auto ltask = std::static_pointer_cast<afrl::cmasi::LoiterTask>(receivedLmcpMessage->m_object);
            if (ltask->getDesiredAction()->getDuration() > 0)
            {
                isTaskEligible.insert(task->getTaskID());
            }
        }
        else if (afrl::impact::isImpactPointSearchTask(receivedLmcpMessage->m_object))
        {
            // loiter at point search location only eligible if non-infinite duration
            auto ptask = std::static_pointer_cast<afrl::impact::ImpactPointSearchTask>(receivedLmcpMessage->m_object);
            if (ptask->getDesiredAction() && ptask->getDesiredAction()->getDuration() > 0)
            {
                isTaskEligible.insert(task->getTaskID());
            }
        }
        else if (!afrl::impact::isWatchTask(receivedLmcpMessage->m_object) &&
                !afrl::impact::isBlockadeTask(receivedLmcpMessage->m_object) &&
                !afrl::impact::isCordonTask(receivedLmcpMessage->m_object) &&
                !afrl::impact::isMultiVehicleWatchTask(receivedLmcpMessage->m_object) &&
                !afrl::impact::isEscortTask(receivedLmcpMessage->m_object))
        {
            // infinite duration tasks are not eligible
            isTaskEligible.insert(task->getTaskID());
        }
    }
    return (false); // always false implies never terminating service from here
}

void TaskTrackerService::HandleVehicleState(std::shared_ptr<afrl::cmasi::EntityState> state)
{
    // check override
    if (m_override.find(state->getID()) != m_override.end())
    {
        if (state->getCurrentWaypoint() == m_override[state->getID()].second)
        {
            if (isTaskEligible.find(m_override[state->getID()].first) != isTaskEligible.end())
            {
                // send task complete message
                auto pTaskComplete = std::make_shared<uxas::messages::task::TaskComplete>();
                pTaskComplete->getEntitiesInvolved().push_back(state->getID());
                pTaskComplete->setTaskID(m_override[state->getID()].first);
                m_pLmcpObjectNetworkClient->sendSharedLmcpObjectBroadcastMessage(pTaskComplete);
            }

            // clear off so as not to spam
            if (m_tasksInProgress.find(state->getID()) != m_tasksInProgress.end())
            {
                m_tasksInProgress[state->getID()].erase(m_override[state->getID()].first);
            }
            m_override.erase(state->getID());

            return;
        }
    }

    // ensure a mission command has previously been sent defining waypoints
    if (m_missions.find(state->getID()) == m_missions.end())
        return;

    // ensure a working task set exists to check against
    if (m_tasksInProgress.find(state->getID()) == m_tasksInProgress.end())
    {
        m_tasksInProgress[state->getID()].clear(); // initialize to not on task
    }

    // look up the waypoint
    afrl::cmasi::Waypoint* wp = nullptr;
    for (size_t k = 0; k < m_missions[state->getID()]->getWaypointList().size(); k++)
    {
        afrl::cmasi::Waypoint* candidateWp = m_missions[state->getID()]->getWaypointList().at(k);
        if (candidateWp->getNumber() == state->getCurrentWaypoint())
        {
            wp = candidateWp;
            break;
        }
    }

    if (wp)
    {
        int64_t taskFromWp = wp->getNumber() / 10000;
        int64_t internalTaskWp = wp->getNumber() % 10000;
        if (taskFromWp > 0)
        {
            m_tasksInProgress[state->getID()].insert(taskFromWp);
            if (!wp->getAssociatedTasks().empty())
            {
                m_taskEmit[taskFromWp] = wp->getAssociatedTasks().front();
            }
        }

        // add currently executing tasks to in-progress list
        for (size_t t = 0; t < wp->getAssociatedTasks().size(); t++)
        {
            int64_t taskId = wp->getAssociatedTasks().at(t);
            m_tasksInProgress[state->getID()].insert(taskId);
        }

        auto i = m_tasksInProgress[state->getID()].begin();
        auto endLoop = m_tasksInProgress[state->getID()].end();
        while (i != endLoop)
        {
            int64_t taskId = *i;

            // check to see if an in-progress task is NOT in the current waypoint task list
            bool isComplete = true;
            for (size_t t = 0; t < wp->getAssociatedTasks().size(); t++)
            {
                if (taskId == wp->getAssociatedTasks().at(t))
                {
                    isComplete = false;
                    break;
                }
            }

            if (taskId == taskFromWp)
            {
                // normally this means we are still working this, so not complete
                isComplete = false;

                // however, if we've stayed on the same task but decreased in internal waypoints
                if (internalTaskWp < m_taskWp[state->getID()][taskId])
                    isComplete = true;
                m_taskWp[state->getID()][taskId] = internalTaskWp;
            }

            if (isComplete)
            {
                if (m_taskEmit.find(taskId) != m_taskEmit.end())
                {
                    taskId = m_taskEmit[taskId];
                    m_taskEmit.erase(*i);
                }

                if (isTaskEligible.find(taskId) != isTaskEligible.end())
                {
                    // send task complete messages (both impact and uxas)
                    auto pTaskComplete = std::make_shared<uxas::messages::task::TaskComplete>();
                    pTaskComplete->getEntitiesInvolved().push_back(state->getID());
                    pTaskComplete->setTaskID(taskId);
                    m_pLmcpObjectNetworkClient->sendSharedLmcpObjectBroadcastMessage(pTaskComplete);
                }

                // clear off working task
                m_tasksInProgress[state->getID()].erase(i++);
                m_taskWp[state->getID()].clear();
            }
            else
            {
                ++i;
            }
        }
    }
}

void TaskTrackerService::HandleMissionCommand(std::shared_ptr<afrl::cmasi::MissionCommand> mish)
{
    // new command for this vehicle, clear any working tasks
    m_tasksInProgress[mish->getVehicleID()].clear();
    // save waypoints for task tracking
    m_missions[mish->getVehicleID()] = mish;

    // clear override
    m_override.erase(mish->getVehicleID());

    // check to see if override should be added
    if (!mish->getWaypointList().empty() &&
            (mish->getWaypointList().back()->getNumber() == mish->getWaypointList().back()->getNextWaypoint()))
    {
        int64_t taskId = 0;
        auto w = mish->getWaypointList().rbegin();
        for (; w != mish->getWaypointList().rend(); w++)
        {
            if ((*w)->getAssociatedTasks().empty())
            {
                if ((*w)->getNumber() >= 10000)
                {
                    taskId = (*w)->getNumber() / 10000;
                    break;
                }
            }
            else
            {
                taskId = (*w)->getAssociatedTasks().front();
                break;
            }
        }

        if (taskId > 0)
        {
            m_override[mish->getVehicleID()] = std::make_pair(taskId, mish->getWaypointList().back()->getNumber());
        }
    }
}


}; //namespace task
}; //namespace service
}; //namespace uxas
