// ===============================================================================
// Authors: AFRL/RQQA
// Organization: Air Force Research Laboratory, Aerospace Systems Directorate, Power and Control Division
// 
// Copyright (c) 2018 Government of the United State of America, as represented by
// the Secretary of the Air Force.  No copyright is claimed in the United States under
// Title 17, U.S. Code.  All Other Rights Reserved.
// ===============================================================================

#ifndef UXAS_MESSAGE_LMCP_OBJECT_NETWORK_BRIDGE_H
#define UXAS_MESSAGE_LMCP_OBJECT_NETWORK_BRIDGE_H

#include "LmcpObjectMessageProcessor.h"
#include "LmcpObjectNetworkClientBase.h"

namespace uxas
{
namespace communications
{

class LmcpObjectNetworkBridge : public LmcpObjectNetworkClientBase, public LmcpObjectMessageProcessor
{
public:
    virtual ~LmcpObjectNetworkBridge() { }
};

}; // namespace communications
}; // namespace uxas

#endif /* UXAS_MESSAGE_LMCP_OBJECT_NETWORK_BRIDGE_H */