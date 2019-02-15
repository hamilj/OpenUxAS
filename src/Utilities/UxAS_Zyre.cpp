// ===============================================================================
// Authors: AFRL/RQQA
// Organization: Air Force Research Laboratory, Aerospace Systems Directorate, Power and Control Division
//
// Copyright (c) 2017 Government of the United State of America, as represented by
// the Secretary of the Air Force.  No copyright is claimed in the United States under
// Title 17, U.S. Code.  All Other Rights Reserved.
// ===============================================================================

#include "UxAS_Zyre.h"

namespace n_ZMQ
{

void zmsgPopstr(zmsg_t *self, std::string& strMessage)
{
    assert(self);
    strMessage = "";
    uint32_t ui32Length(zframe_size(zmsg_first(self)));
    char* pString  = zmsg_popstr(self);
    if (pString)
    {
        strMessage = std::string(pString, ui32Length);
        delete pString;
        pString = 0;
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
    std::string strReturn;
    char * cstrKey = new char [strKey.length()+1];
    std::strcpy(cstrKey, strKey.c_str());
    strValue = "";
    const char* pcValue = static_cast<const char*>(zhash_lookup(headers, cstrKey));
    if (pcValue)
    {
        strValue = pcValue;
    }
    delete[] cstrKey;
    cstrKey = 0;
}

void zyreSetHeaderEntry(zyre_t* pZyreNode, const std::string& strKey, const std::string& strValue)
{
    char * cstrKey = new char [strKey.length()+1];
    std::strcpy(cstrKey, strKey.c_str());
    char * cstrValue = new char [strValue.length()+1];
    std::strcpy(cstrValue, strValue.c_str());
    zyre_set_header(pZyreNode, cstrKey, "%s", cstrValue);
    delete[] cstrKey;
    cstrKey = 0;
    delete[] cstrValue;
    cstrValue = 0;
}

void zyreJoin(zyre_t* pZyreNode, const std::string& strZyreGroup)
{
    char * cstrZyreGroup = new char [strZyreGroup.length()+1];
    std::strcpy(cstrZyreGroup, strZyreGroup.c_str());
    zyre_join(pZyreNode, cstrZyreGroup);
    delete[] cstrZyreGroup;
    cstrZyreGroup = 0;
}

int zyreWhisper(zyre_t* pZyreNode, const std::string& strPeer, const std::string& strMessage)
{
    assert(pZyreNode);
    assert(!strPeer.empty());

    zmsg_t *msg = zmsg_new();
    zmsg_addmem(msg, strMessage.data(), strMessage.length());
    zyre_whisper(pZyreNode, const_cast<char*>(strPeer.c_str()), &msg);

    return 0;
}

int zyreWhisper2(zyre_t* pZyreNode, const std::string& strPeer, const std::string& strMessage)
{
    assert(pZyreNode);
    assert(!strPeer.empty());

    zmsg_t *msg = zmsg_new();
    zmsg_addmem(msg,strMessage.data(), strMessage.length());
    zyre_whisper(pZyreNode, const_cast<char*>(strPeer.c_str()), &msg);

    return 0;
}

int zyreShout(zyre_t* pZyreNode, const std::string& strGroup, const std::string& strMessage)
{
    assert(pZyreNode);
    assert(!strGroup.empty());

    zmsg_t *msg = zmsg_new();
    zmsg_addmem(msg, strMessage.data(), strMessage.length());
    zyre_shout(pZyreNode, const_cast<char*>(strGroup.c_str()), &msg);

    return 0;
}

} // namespace n_ZMQ