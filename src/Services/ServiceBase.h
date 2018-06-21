// ===============================================================================
// Authors: AFRL/RQQA
// Organization: Air Force Research Laboratory, Aerospace Systems Directorate, Power and Control Division
// 
// Copyright (c) 2017 Government of the United State of America, as represented by
// the Secretary of the Air Force.  No copyright is claimed in the United States under
// Title 17, U.S. Code.  All Other Rights Reserved.
// ===============================================================================

#ifndef UXAS_SERVICE_SERVICE_BASE_H
#define UXAS_SERVICE_SERVICE_BASE_H

#include "LmcpObjectNetworkClient.h"
#include "LmcpObjectMessageProcessor.h"

#include "AddressedAttributedMessage.h"
#include "LmcpMessage.h"
#include "UxAS_Log.h"

#include "pugixml.hpp"

#include <memory>
#include <string>

namespace uxas
{
namespace service
{

/**
 * \brief The <B><i>getUniqueId</i></B> returns a unique service ID.
 * 
 * @return unique service ID.
 */
int64_t getUniqueId();

/** \class ServiceBase
 * 
 * \par The <B><i>ServiceBase</i></B> is the base class for all UxAS service classes. 
 * Service class constructors are registered in the <B><i>ServiceBase</i></B> 
 * creation registry.
 * 
 * @n
 */
class ServiceBase : public uxas::communications::LmcpObjectMessageProcessor
{
public:
    ServiceBase(const std::string& serviceType, const std::string& workDirectoryName,
        std::unique_ptr<uxas::communications::LmcpObjectNetworkClient> pLmcpObjectNetworkClient);

    virtual ~ServiceBase() { }

    /** \brief The <B><i>configureService</i></B> method performs service configuration. 
     * It must be invoked before calling the <B><i>initializeAndStartService</i></B>. 
     * 
     * @param parentOfWorkDirectory parent directory where work directory will be created
     * @param serviceXml XML configuration
     * @return true if configuration succeeds; false if configuration fails.
     */
    bool
    configureService(const std::string& parentOfWorkDirectory, const std::string& serviceXml);

    /** \brief The <B><i>configureService</i></B> method performs service configuration. 
     * It must be invoked before calling the <B><i>initializeAndStartService</i></B>. 
     * 
     * @param parentOfWorkDirectory parent directory where work directory will be created
     * @param serviceXmlNode XML configuration
     * @return true if configuration succeeds; false if configuration fails.
     */
    bool
    configureService(const std::string& parentWorkDirectory, const pugi::xml_node& serviceXmlNode);

    /** \brief The <B><i>initializeAndStartService</i></B> method performs service 
     * initialization and startup. It must be invoked after calling the 
     * <B><i>configureService</i></B> method. Do not use for 
     * <B><i>ServiceManager</i></B>, instead invoke the 
     * <B><i>initializeAndStart</i></B> method.
     * 
     * @return true if all initialization and startup succeeds; false if initialization or startup fails.
     */
    bool
    initializeAndStartService();

    inline bool addSubscriptionAddress(const std::string& address)
    {
        return m_pLmcpObjectNetworkClient->addSubscriptionAddress(address);
    }

    inline bool removeSubscriptionAddress(const std::string& address)
    {
        return m_pLmcpObjectNetworkClient->removeSubscriptionAddress(address);
    }

    inline bool removeAllSubscriptionAddresses()
    {
        return m_pLmcpObjectNetworkClient->removeAllSubscriptionAddresses();
    }

    inline void sendLmcpObjectLimitedCastMessage(const std::string& castAddress, std::unique_ptr<avtas::lmcp::Object> lmcpObject)
    {
        m_pLmcpObjectNetworkClient->sendLmcpObjectLimitedCastMessage(castAddress, std::move(lmcpObject));
    }

    inline void sendLmcpObjectBroadcastMessage(std::unique_ptr<avtas::lmcp::Object> lmcpObject)
    {
        m_pLmcpObjectNetworkClient->sendLmcpObjectBroadcastMessage(std::move(lmcpObject));
    }

    inline void sendSerializedLmcpObjectMessage(std::unique_ptr<uxas::communications::data::AddressedAttributedMessage> serializedLmcpObject)
    {
        m_pLmcpObjectNetworkClient->sendSerializedLmcpObjectMessage(std::move(serializedLmcpObject));
    }

    inline void sendSharedLmcpObjectBroadcastMessage(const std::shared_ptr<avtas::lmcp::Object>& lmcpObject)
    {
        m_pLmcpObjectNetworkClient->sendSharedLmcpObjectBroadcastMessage(lmcpObject);
    }

    inline void sendSharedLmcpObjectLimitedCastMessage(const std::string& castAddress, const std::shared_ptr<avtas::lmcp::Object>& lmcpObject)
    {
        m_pLmcpObjectNetworkClient->sendSharedLmcpObjectLimitedCastMessage(castAddress, lmcpObject);
    }

    /** \brief unique ID of the component.  */
    std::uint32_t m_serviceId;

    /** \brief  */
    std::string m_serviceType;

    /** \brief  */
    std::string m_workDirectoryName;

    std::unique_ptr<uxas::communications::LmcpObjectNetworkClient> m_pLmcpObjectNetworkClient;

    /** \brief Unique ID for UxAS entity instance; value read from configuration XML */
    uint32_t m_entityId;

    /** \brief String representation of the unique ID for UxAS entity instance; value read from configuration XML */
    std::string m_entityIdString;

    /** \brief Type of UxAS entity instance; value read from configuration XML */
    std::string m_entityType;

    /** \brief Unique ID of the <b>LMCP</b> object communication network actor (e.g., bridge or service). */
    int64_t m_networkId;

    /** \brief String representation of the unique ID of the <b>LMCP</b> object communication network actor (e.g., bridge or service). */
    std::string m_networkIdString;

    /** \brief Name of subclass used for logging/messaging. */
    std::string m_networkClientTypeName;

    inline int64_t getUniqueEntitySendMessageId() { return uxas::communications::getUniqueEntitySendMessageId(); }

    inline std::string getEntityServicesCastAllAddress(const uint32_t entityId)
    {
        return uxas::communications::getEntityServicesCastAllAddress(entityId);
    }

    inline std::string getNetworkClientUnicastAddress(const uint32_t entityId, const int64_t networkClientId)
    {
        return uxas::communications::getNetworkClientUnicastAddress(entityId, networkClientId);
    }

    inline std::string getNetworkClientUnicastAddress(const uint32_t entityId, const std::string networkClientId)
    {
        return uxas::communications::getNetworkClientUnicastAddress(entityId, networkClientId);
    }

    inline std::string getNetworkClientUnicastAddress(const std::string& entityId, const std::string& networkClientId)
    {
        return uxas::communications::getNetworkClientUnicastAddress(entityId, networkClientId);
    }

    inline bool getIsTerminationFinished()
    {
        return m_pLmcpObjectNetworkClient->getIsTerminationFinished();
    }

protected:
    /** \brief  */
    bool m_isConfigured{false};

    /** \brief  */
    std::string m_workDirectoryPath;

    uxas::communications::LmcpObjectNetworkClient::ReceiveProcessingType m_receiveProcessingType{uxas::communications::LmcpObjectNetworkClient::ReceiveProcessingType::LMCP};

private:
    /** \brief Copy construction not permitted */
    ServiceBase(ServiceBase const&) = delete;

    /** \brief Copy assignment operation not permitted */
    void operator=(ServiceBase const&) = delete;

    // <editor-fold defaultstate="collapsed" desc="Static Service Registry">
public:
    /** \brief The <B><i>instantiateService</i></B> method creates an instance 
     * of a service class that inherits from <B><i>ServiceBase</i></B>
     * 
     * @param serviceType type name of the service to be instantiated
     * @return instantiated service
     */
    static
    std::unique_ptr<ServiceBase>
    instantiateService(const std::string& serviceType, std::unique_ptr<uxas::communications::LmcpObjectNetworkClient> pLmcpObjectNetworkClient)
    {
        auto it = createFunctionByServiceType().find(serviceType);
        ServiceBase * newService(it == createFunctionByServiceType().end() ? nullptr : (it->second)(std::move(pLmcpObjectNetworkClient)));
        std::unique_ptr<ServiceBase> service(newService);
        return (service);
    }

protected:
    /** \brief type representing a pointer to a service creation function.  */
    using serviceCreationFunctionPointer = ServiceBase* (*)(std::unique_ptr<uxas::communications::LmcpObjectNetworkClient>);

    /** \brief static service creation function implemented that is implemented by subclasses.  */
    static
    ServiceBase*
    create() { return nullptr; }

    /** \brief registers service type name, alias type names and it's create() function for a subclass.  */
    static
    void
    registerServiceCreationFunctionPointers(const std::vector<std::string>& registryServiceTypeNames, serviceCreationFunctionPointer functionPointer)
    {
        for (auto& serviceTypeName : registryServiceTypeNames)
        {
            auto it = createFunctionByServiceType().find(serviceTypeName);
            if (it != createFunctionByServiceType().end())
            {
                UXAS_LOG_WARN("ServiceBase::registerServiceCreationFunctionPointers is overwriting existing service creation function pointer ", serviceTypeName);
            }
            createFunctionByServiceType()[serviceTypeName] = functionPointer;
        }
    }

    template <typename T>
    struct CreationRegistrar
    {
        explicit
        CreationRegistrar(const std::vector<std::string>& registryServiceTypeNames)
        {
            ServiceBase::registerServiceCreationFunctionPointers(registryServiceTypeNames, &T::create);
        }
    };

private:
    /** \brief function to access static map of create functions keyed by serviceType.
     * 
     * @return service type to create function map.
     */
    static
    std::unordered_map<std::string, ServiceBase::serviceCreationFunctionPointer>&
    createFunctionByServiceType()
    {
        static std::unordered_map<std::string, ServiceBase::serviceCreationFunctionPointer> createFncPtrMap;
        return createFncPtrMap;
    }
    // </editor-fold>
};

}; //namespace service
}; //namespace uxas

#endif /* UXAS_SERVICE_SERVICE_BASE_H */
