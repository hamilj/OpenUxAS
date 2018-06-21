// ===============================================================================
// Authors: AFRL/RQQA
// Organization: Air Force Research Laboratory, Aerospace Systems Directorate, Power and Control Division
// 
// Copyright (c) 2017 Government of the United State of America, as represented by
// the Secretary of the Air Force.  No copyright is claimed in the United States under
// Title 17, U.S. Code.  All Other Rights Reserved.
// ===============================================================================

#ifndef UXAS_MESSAGE_LMCP_OBJECT_NETWORK_CLIENT_BASE_H
#define UXAS_MESSAGE_LMCP_OBJECT_NETWORK_CLIENT_BASE_H

#include "LmcpObjectNetworkClient.h"

#include "LmcpObjectMessageReceiverPipe.h"
#include "LmcpObjectMessageSenderPipe.h"

#include "avtas/lmcp/Factory.h"

#include <cstdint>
#include <memory>
#include <set>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

namespace uxas
{
namespace communications
{

class LmcpObjectMessageProcessor;

/** \class LmcpObjectNetworkClientBase
 * 
 * \par Overview:     
 * The <B><i>LmcpObjectNetworkClientBase</i></B> is the base class for all classes 
 * that communicate across an <b>LMCP</b> network.  It has the following behaviors and characteristics: 
 * <ul style="padding-left:1em;margin-left:0">
 * <li>Configuration
 * <li>Initialization and Startup
 * <li>Threading
 * <li>Receiving <b>LMCP</b> object messages
 * <li>Sending <b>LMCP</b> object messages
 * <li>Termination
 * </ul>
 * 
 * \par Behaviors and Characteristics:
 * <ul style="padding-left:1em;margin-left:0">
 * <li><i>\u{Configuration}</i> first executes logic specific to configuration of this class, then calls 
 * the <B><i>configure</i></B> virtual method. Adding message subscription addresses 
 * is an example of logic that might be present in the <B><i>configure</i></B> method 
 * of an inheriting class.
 * 
 * <li><i>\u{Initialization and Startup}</i> consists of the following logical sequence 
 * (1) initialize this class; 
 * (2) initialize the inheriting class by calling the <B><i>initialize</i></B> virtual method; 
 * (3) start the inheriting class by calling the <B><i>start</i></B> virtual method; 
 * (4) start this class.
 * 
 * <li><i>\u{Threading}</i> consists of a single thread within this class and zero-many 
 * threads in an inheriting class. Inheriting classes that have their own threads 
 * must implement the <B><i>terminate</i></B> virtual method to achieve clean destruction.
 * 
 * <li><i>\u{Receiving <b>LMCP</b> object messages}</i> are processed by calling either the 
 * <B><i>processReceivedLmcpMessage</i></B> virtual method or the 
 * <B><i>processReceivedSerializedLmcpMessage</i></B> virtual method (as determined 
 * by configuration). Receiving any <b>LMCP</b> object message requires the appropriate 
 * configuration of message addresses. 
 * Uni-cast, multi-cast and broadcast messages are supported.
 * 
 * <li><i>\u{Sending <b>LMCP</b> object messages}</i> can be performed by inheriting classes 
 * by calling one of three methods: 
 * <B><i>sendLmcpObjectBroadcastMessage</i></B>, 
 * <B><i>sendLmcpObjectLimitedCastMessage</i></B> or 
 * <B><i>sendSerializedLmcpObjectMessage</i></B>. Uni-cast, multi-cast and broadcast 
 * messages are supported.
 * 
 * <li><i>\u{Termination}</i> can occur in two different ways. If true is returned by either the 
 * <B><i>processReceivedLmcpMessage</i></B> virtual method or the 
 * <B><i>processReceivedSerializedLmcpMessage</i></B> virtual method, then termination 
 * logic is executed. Alternatively, receiving the the appropriate <b>LMCP</b> object 
 * termination message results in termination. During termination, 
 * <B><i>terminate</i></B> virtual method is called that to execute logic that is 
 * specific to the inheriting class (e.g., thread joining).
 * </ul>
 * 
 * 
 * @n
 */
class LmcpObjectNetworkClientBase : public LmcpObjectNetworkClient
{
protected:
    
    /** \brief static entity service cast address.  */
    static std::string s_entityServicesCastAllAddress;
            
public:

    /** \brief Type name for the <B><i>LmcpObjectNetworkClientBase</i></B> class */
    static const std::string&
    s_typeName() { static std::string s_string("LmcpObjectNetworkClientBase"); return (s_string); };

    LmcpObjectNetworkClientBase();

    virtual
    ~LmcpObjectNetworkClientBase();

private:

    /** \brief Copy construction not permitted */
    LmcpObjectNetworkClientBase(LmcpObjectNetworkClientBase const&) = delete;

    /** \brief Copy assignment operation not permitted */
    void operator=(LmcpObjectNetworkClientBase const&) = delete;

public:

    // TODO consider LmcpObjectNetworkBridgeManager friend declaration and 
    // make configureNetworkClient protected
    /** \brief The <B><i>configureNetworkClient</i></B> method must be invoked 
     * before calling the <B><i>initializeAndStart</i></B> 
     * method.  It performs <B><i>LmcpObjectNetworkClientBase</i></B>-specific configuration 
     * and invokes the <B><i>configure</i></B> virtual method. 
     * 
     * @param subclassTypeName type name of the inheriting class.
     * @param receiveProcessingType enumeration determining whether or not received <b>LMCP</b> message will be de-serialized.
     * @param networkXmlNode XML node containing object configurations.
     * @return true if configuration succeeds; false if configuration fails.
     */
    bool
    configureNetworkClient(const std::string& subclassTypeName, ReceiveProcessingType receiveProcessingType,
        const pugi::xml_node& networkClientXmlNode, LmcpObjectMessageProcessor& msgProcessor) override;

    /** \brief The <B><i>initializeAndStart</i></B> must be invoked 
     * after calling the protected <B><i>configureNetworkClient</i></B> method.  
     * It performs the following steps:
     * <ul style="padding-left:1em;margin-left:0">
     * <li><B><i>LmcpObjectNetworkClientBase</i></B>-specific initialization
     * <li>inheriting class initialization (calls <B><i>initialize</i></B> virtual method)
     * <li>inheriting class startup (calls <B><i>start</i></B> virtual method)
     * <li><B><i>LmcpObjectNetworkClientBase</i></B>-specific startup
     * </ul>
     * 
     * @return true if all initialization and startup succeeds; false if initialization or startup fails.
     */
    bool
    initializeAndStart(LmcpObjectMessageProcessor& msgProcessor) override;
    
public:
    /** \brief The <B><i>addSubscriptionAddress</i></B> can be invoked 
     * at any time to add specified message subscription address. 
     * 
     * @param address message subscription value
     * @return true if address is added; false if address is not added.
     */
    bool
    addSubscriptionAddress(const std::string& address) override;

    /** \brief The <B><i>removeSubscriptionAddress</i></B> can be invoked 
     * at any time to remove specified message subscription address. 
     * 
     * @param address message subscription address
     * @return true if address is removed; false if address is not removed.
     */
    bool
    removeSubscriptionAddress(const std::string& address) override;
    
    /** \brief The <B><i>removeAllSubscriptionAddresses</i></B> can be invoked 
     * at any time to remove message subscription addresses. 
     * 
     * @param address message subscription address
     * @return true if address is removed; false if address is not removed.
     */
    bool
    removeAllSubscriptionAddresses() override;
    
    /** \brief The <B><i>sendLmcpObjectLimitedCastMessage</i></B> method can be 
     * invoked to send a uni-cast or multi-cast <b>LMCP</b> object message to the <b>LMCP</b> network. 
     * 
     * @param castAddress message publish address
     * @param lmcpObject <b>LMCP</b> object to be uni-casted/multi-casted.
     */
    void
    sendLmcpObjectLimitedCastMessage(const std::string& castAddress, std::unique_ptr<avtas::lmcp::Object> lmcpObject) override;
    
protected:
    /** \brief The <B><i>sendLmcpObjectBroadcastMessage</i></B> method can be 
     * invoked to broadcast a <b>LMCP</b> object message on the <b>LMCP</b> network. 
     * 
     * @param lmcpObject <b>LMCP</b> object to be broadcasted. The message publish 
     * address is derived from the full <b>LMCP</b> object name.
     */
    void
    sendLmcpObjectBroadcastMessage(std::unique_ptr<avtas::lmcp::Object> lmcpObject) override;

    /** \brief The <B><i>sendSerializedLmcpObjectMessage</i></B> method can be 
     * invoked to send a <B><i>AddressedAttributedMessage</i></B> to the <b>LMCP</b> network. The 
     * <B><i>AddressedAttributedMessage</i></B> payload must be a serialized <b>LMCP</b> object string. 
     * 
     * @param serializedLmcpObject <b>LMCP</b> object to be sent (uni-cast/multi-cast/broadcast).
     */
    void
    sendSerializedLmcpObjectMessage(std::unique_ptr<uxas::communications::data::AddressedAttributedMessage> serializedLmcpObject) override;
    
    /** \brief The <B><i>sendSharedLmcpObjectBroadcastMessage</i></B> method can be 
     * invoked to broadcast a <b>LMCP</b> object message on the <b>LMCP</b> network. 
     * 
     * @param lmcpObject <b>LMCP</b> object to be broadcasted. The message publish 
     * address is derived from the full <b>LMCP</b> object name.
     */
    void
    sendSharedLmcpObjectBroadcastMessage(const std::shared_ptr<avtas::lmcp::Object>& lmcpObject) override;

    /** \brief The <B><i>sendSharedLmcpObjectLimitedCastMessage</i></B> method can be 
     * invoked to send a uni-cast or multi-cast <b>LMCP</b> object message to the <b>LMCP</b> network. 
     * 
     * @param castAddress message publish address
     * @param lmcpObject <b>LMCP</b> object to be uni-casted/multi-casted.
     */
    void
    sendSharedLmcpObjectLimitedCastMessage(const std::string& castAddress, const std::shared_ptr<avtas::lmcp::Object>& lmcpObject) override;

private:
    
    /** \brief The <B><i>initializeNetworkClient</i></B> method is invoked by 
     * the <B><i>initializeAndStart</i></B> method to perform 
     * <B><i>LmcpObjectNetworkClientBase</i></B>-specific initialization 
     * 
     * @return true if initialization succeeds; false if initialization fails.
     */
    bool
    initializeNetworkClient();

    /** \brief If <B><i>m_receiveProcessingType</i></B> == 
     * <B><i>ReceiveProcessingType::LMCP</i></B>, then 
     * the <B><i>executeNetworkClient</i></B> method repeatedly invokes 
     * the <B><i>processReceivedLmcpMessage</i></B> in an infinite loop until 
     * termination. Otherwise, the <B><i>executeNetworkClient</i></B> method is 
     * not invoked.
     */
    void
    executeNetworkClient(LmcpObjectMessageProcessor& msgProcessor);

    /** \brief If <B><i>m_receiveProcessingType</i></B> == 
     * <B><i>ReceiveProcessingType::SERIALIZED_LMCP</i></B>, then 
     * the <B><i>executeSerializedNetworkClient</i></B> method repeatedly invokes 
     * the <B><i>processReceivedSerializedLmcpMessage</i></B> in an infinite loop until 
     * termination. Otherwise, the <B><i>executeSerializedNetworkClient</i></B> method is 
     * not invoked.
     */
    void
    executeSerializedNetworkClient(LmcpObjectMessageProcessor& msgProcessor);

    /** \brief The <B><i>deserializeMessage</i></B> method deserializes an LMCP 
     * string into an LMCP object.
     * 
     * @return unique pointer to LMCP object if succeeds; unique pointer with 
     * unassigned native pointer. 
     */
    std::shared_ptr<avtas::lmcp::Object>
    deserializeMessage(const std::string& payload);

private:
    
    /** \brief  */
    bool m_isConfigured{false};

    /** \brief  */
    bool m_isThreadStarted{false};

    /** \brief  this is the unique ID for the entity represented by this instance of the UxAS software, configured in component manager XML*/
    ReceiveProcessingType m_receiveProcessingType;

    /** \brief Pointer to the component's thread.  */
    std::unique_ptr<std::thread> m_networkClientThread;

    uxas::communications::LmcpObjectMessageReceiverPipe m_lmcpObjectMessageReceiverPipe;
    std::set<std::string> m_preStartLmcpSubscriptionAddresses;

    uxas::communications::LmcpObjectMessageSenderPipe m_lmcpObjectMessageSenderPipe;
    
};

}; //namespace communications
}; //namespace uxas

#endif /* UXAS_MESSAGE_LMCP_OBJECT_NETWORK_CLIENT_BASE_H */
