// ===============================================================================
// Authors: AFRL/RQQA
// Organization: Air Force Research Laboratory, Aerospace Systems Directorate, Power and Control Division
//
// Copyright (c) 2017 Government of the United State of America, as represented by
// the Secretary of the Air Force.  No copyright is claimed in the United States under
// Title 17, U.S. Code.  All Other Rights Reserved.
// ===============================================================================

/* 
 * File:   UxAS_Zyre.h
 * Author: steve
 *
 * Created on February 24, 2014, 6:15 PM
 */

#ifndef UXAS_ZYRE_H
#define	UXAS_ZYRE_H

#include "czmq.h"
#include "zyre.h"

#include <cstring>
#include <string>

namespace n_ZMQ
{

//// ZYRE HELPER FUNCTIONS
//    typedef std::shared_ptr<zyre_t> PTR_ZYRE_t;
//    typedef std::shared_ptr<zctx_t> PTR_ZCTX_t;
void zmsgPopstr(zmsg_t *self, std::string& strMessage);

int s_SendString(void *socket, bool more, const std::string& strString);

//  ---------------------------------------------------------------------
//  Return the item at the specified key, or null
void ZhashLookup(zhash_t *headers, const std::string& strKey, std::string& strValue);

void zyreSetHeaderEntry(zyre_t* pZyreNode, const std::string& strKey, const std::string& strValue);

//  ---------------------------------------------------------------------
//  Join a named group; after joining a group you can send messages to
//  the group and all Zyre nodes in that group will receive them.
void zyreJoin(zyre_t* pZyreNode, const std::string& strZyreGroup);

//  ---------------------------------------------------------------------
//  Send message to single peer, specified as a UUID string
//  Destroys message after sending
int zyreWhisper(zyre_t* pZyreNode, const std::string& strPeer, const std::string& strMessage);

int zyreWhisper2(zyre_t* pZyreNode, const std::string& strPeer, const std::string& strMessage);

//  ---------------------------------------------------------------------
//  Send message to a named group
//  Destroys message after sending
int zyreShout(zyre_t* pZyreNode, const std::string& strGroup, const std::string& strMessage);

} //namespace n_ZMQ

#endif	/* UXAS_ZYRE_H */