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

namespace pugi
{
class xml_node;
} // namespace pugi

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

    /** \brief The virtual <B><i>configure</i></B> method is invoked by the 
     * <B><i>LmcpObjectNetworkClient</i></B> class after completing its own 
     * configuration. 
     * 
     * @param xmlNode XML node containing object configurations.
     * @return true if configuration succeeds; false if configuration fails.
     */
    virtual bool configure(const pugi::xml_node& xmlNode) { return true; };

    /** \brief The virtual <B><i>initialize</i></B> method is invoked by the 
     * <B><i>LmcpObjectNetworkClientBase</i></B> class after completing 
     * configurations and before startup. 
     * 
     * @return true if initialization succeeds; false if initialization fails.
     */
    virtual bool initialize() { return true; };

    /** \brief The virtual <B><i>start</i></B> method is invoked by the 
     * <B><i>LmcpObjectNetworkClientBase</i></B> class after initialization and
     * before starting its own thread. 
     * 
     * @return true if start succeeds; false if start fails.
     */
    virtual bool start() { return true; };

    /** \brief The virtual <B><i>terminate</i></B> method can be implemented by 
     * inheriting classes to perform inheriting class termination logic 
     * (e.g., thread joining). 
     */
    virtual void terminate() { }

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