// ===============================================================================
// Authors: AFRL/RQQA
// Organization: Air Force Research Laboratory, Aerospace Systems Directorate, Power and Control Division
//
// Copyright (c) 2017 Government of the United State of America, as represented by
// the Secretary of the Air Force.  No copyright is claimed in the United States under
// Title 17, U.S. Code.  All Other Rights Reserved.
// ===============================================================================

#include "UxAS_Zyre.h"

#include <memory>

namespace n_ZMQ
{

void zmsgPopstr(zmsg_t *self, std::string& strMessage)
{
    assert(self);
    strMessage = "";
    uint32_t ui32Length(zframe_size(zmsg_first(self)));
    std::unique_ptr<char> pString(zmsg_popstr(self));
    if (pString)
    {
        strMessage = std::string(pString.get(), ui32Length);
    }
}

int s_SendString(void *socket, bool more, const std::string& strString)
{
    assert(socket);

    int len = strString.size();
    zmq_msg_t message;
    zmq_msg_init_size(&message,len);
    memcpy(zmq_msg_data(&message), strString.c_str(), len);
    int rc = zmq_sendmsg(socket, &message, more ? ZMQ_SNDMORE : 0);

    return rc == -1 ? -1 : 0;
}

void ZhashLookup(zhash_t *headers, const std::string& strKey, std::string& strValue)
{
    strValue = "";
    const char* pcValue = static_cast<const char*>(zhash_lookup(headers, strKey.c_str()));
    if (pcValue)
    {
        strValue = pcValue;
    }
}

void zyreSetHeaderEntry(zyre_t* pZyreNode, const std::string& strKey, const std::string& strValue)
{
    zyre_set_header(pZyreNode, strKey.c_str(), "%s", strValue.c_str());
}

void zyreJoin(zyre_t* pZyreNode, const std::string& strZyreGroup)
{
    zyre_join(pZyreNode, strZyreGroup.c_str());
}

int zyreWhisper(zyre_t* pZyreNode, const std::string& strPeer, const std::string& strMessage)
{
    assert(pZyreNode);
    assert(!strPeer.empty());

    zmsg_t *msg = zmsg_new();
    zmsg_addmem(msg, strMessage.data(), strMessage.length());
    zyre_whisper(pZyreNode, strPeer.c_str(), &msg);

    return 0;
}

int zyreWhisper2(zyre_t* pZyreNode, const std::string& strPeer, const std::string& strMessage)
{
    assert(pZyreNode);
    assert(!strPeer.empty());

    zmsg_t *msg = zmsg_new();
    zmsg_addmem(msg,strMessage.data(), strMessage.length());
    zyre_whisper(pZyreNode, strPeer.c_str(), &msg);

    return 0;
}

int zyreShout(zyre_t* pZyreNode, const std::string& strGroup, const std::string& strMessage)
{
    assert(pZyreNode);
    assert(!strGroup.empty());

    zmsg_t *msg = zmsg_new();
    zmsg_addmem(msg, strMessage.data(), strMessage.length());
    zyre_shout(pZyreNode, strGroup.c_str(), &msg);

    return 0;
}

} // namespace n_ZMQ