// ===============================================================================
// Authors: AFRL/RQQA
// Organization: Air Force Research Laboratory, Aerospace Systems Directorate, Power and Control Division
//
// Copyright (c) 2018 Government of the United State of America, as represented by
// the Secretary of the Air Force.  No copyright is claimed in the United States under
// Title 17, U.S. Code.  All Other Rights Reserved.
// ===============================================================================

#include "LmcpObjectNetworkBridge.h"

namespace uxas
{
namespace communications
{

LmcpObjectNetworkBridge::LmcpObjectNetworkBridge()
    : m_entityIdNetworkIdUnicastString(getNetworkClientUnicastAddress(m_entityId, m_networkId))
{ }

} // namespace communications
} // namespace uxas