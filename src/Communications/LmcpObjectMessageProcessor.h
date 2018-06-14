// ===============================================================================
// Authors: AFRL/RQQA
// Organization: Air Force Research Laboratory, Aerospace Systems Directorate, Power and Control Division
// 
// Copyright (c) 2018 Government of the United State of America, as represented by
// the Secretary of the Air Force.  No copyright is claimed in the United States under
// Title 17, U.S. Code.  All Other Rights Reserved.
// ===============================================================================

#ifndef UXAS_MESSAGE_LMCP_OBJECT_MESSAGE_PROCESSOR_H
#define UXAS_MESSAGE_LMCP_OBJECT_MESSAGE_PROCESSOR_H

#include <memory>

namespace uxas
{
namespace communications
{

namespace data
{
class AddressedAttributedMessage;
class LmcpMessage;
} // namespace data

class LmcpObjectMessageProcessor
{
public:
    virtual ~LmcpObjectMessageProcessor() { }

    /** \brief The virtual <B><i>processReceivedLmcpMessage</i></B> is 
     * repeatedly invoked by the <B><i>LmcpObjectNetworkClientBase</i></B> class in an 
     * infinite loop until termination. 
     * 
     * @param receivedLmcpObject received <b>LMCP</b> object.
     * @return true if object is to terminate; false if object is to continue processing.
     */
    virtual bool processReceivedLmcpMessage(std::unique_ptr<uxas::communications::data::LmcpMessage> receivedLmcpMessage) { return false; };

    /** \brief The virtual <B><i>processReceivedSerializedLmcpMessage</i></B> is 
     * repeatedly invoked by the <B><i>LmcpObjectNetworkClientBase</i></B> class in an 
     * infinite loop until termination. The payload of the <B><i>AddressedAttributedMessage</i></B> 
     * is a serialized <b>LMCP</b> object. 
     * 
     * @param receivedSerializedLmcpObject received AddressedAttributedMessage object with serialized <b>LMCP</b> object payload.
     * @return true if object is to terminate; false if object is to continue processing.
     */
    virtual bool processReceivedSerializedLmcpMessage(std::unique_ptr<uxas::communications::data::AddressedAttributedMessage> receivedSerializedLmcpMessage) { return false; };
};

}; // namespace communications
}; // namespace uxas

#endif /* UXAS_MESSAGE_LMCP_OBJECT_MESSAGE_PROCESSOR_H */