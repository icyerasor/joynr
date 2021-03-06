package io.joynr.generator.cpp.inprocess
/*
 * !!!
 *
 * Copyright (C) 2011 - 2016 BMW Car IT GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import com.google.inject.Inject
import io.joynr.generator.cpp.util.CppInterfaceUtil
import io.joynr.generator.cpp.util.CppStdTypeUtil
import io.joynr.generator.cpp.util.InterfaceSubscriptionUtil
import io.joynr.generator.cpp.util.JoynrCppGeneratorExtensions
import io.joynr.generator.cpp.util.TemplateBase
import io.joynr.generator.templates.InterfaceTemplate
import io.joynr.generator.templates.util.AttributeUtil
import io.joynr.generator.templates.util.NamingUtil
import org.franca.core.franca.FInterface

class InterfaceInProcessConnectorHTemplate implements InterfaceTemplate{

	@Inject private extension TemplateBase
	@Inject private extension CppStdTypeUtil
	@Inject private extension JoynrCppGeneratorExtensions
	@Inject private extension CppInterfaceUtil
	@Inject private extension NamingUtil
	@Inject private extension AttributeUtil
	@Inject private extension InterfaceSubscriptionUtil

	override  generate(FInterface serviceInterface)
'''
«val interfaceName = serviceInterface.joynrName»
«val headerGuard = ("GENERATED_INTERFACE_"+getPackagePathWithJoynrPrefix(serviceInterface, "_")+
	"_"+interfaceName+"InProcessConnector_h").toUpperCase»
«warning()»

#ifndef «headerGuard»
#define «headerGuard»

#include "joynr/PrivateCopyAssign.h"
#include "«getPackagePathWithJoynrPrefix(serviceInterface, "/")»/I«interfaceName»Connector.h"
#include "joynr/InProcessPublicationSender.h"
#include "joynr/InProcessConnectorFactory.h"
#include "joynr/SubscriptionRequest.h"
#include "joynr/BroadcastSubscriptionRequest.h"
#include "joynr/SubscriptionQos.h"
#include "joynr/OnChangeSubscriptionQos.h"
#include "joynr/Logger.h"
#include "joynr/TypeUtil.h"

«FOR parameterType: getRequiredIncludesFor(serviceInterface).addElements(includeForString)»
	#include «parameterType»
«ENDFOR»
#include <memory>
#include <functional>

namespace joynr {
	class InProcessAddress;
	class ISubscriptionManager;
	class PublicationManager;
	template <class ... Ts> class Future;
	template <typename... Ts> class ISubscriptionListener;

namespace exceptions
{
	class JoynrException;
	class JoynrRuntimeException;
} // namespace exceptions

} // namespace joynr

«getNamespaceStarter(serviceInterface)»

/** @brief InProcessConnector class for interface «interfaceName» */
class «interfaceName»InProcessConnector : public I«interfaceName»Connector {
private:
«FOR attribute: getAttributes(serviceInterface).filter[attribute | attribute.notifiable]»
	«val returnType = attribute.typeName»
	std::string subscribeTo«attribute.joynrName.toFirstUpper»(
				std::shared_ptr<joynr::ISubscriptionListener<«returnType»> > subscriptionListener,
				const joynr::SubscriptionQos& subscriptionQos,
				SubscriptionRequest& subscriptionRequest);
«ENDFOR»
«FOR broadcast: serviceInterface.broadcasts»
«val returnTypes = broadcast.commaSeparatedOutputParameterTypes»
«val broadcastName = broadcast.joynrName»
	std::string subscribeTo«broadcastName.toFirstUpper»Broadcast(
			std::shared_ptr<joynr::ISubscriptionListener<«returnTypes» > > subscriptionListener,
			const joynr::OnChangeSubscriptionQos& subscriptionQos,
			BroadcastSubscriptionRequest& subscriptionRequest);
«ENDFOR»
public:

	/**
	 * @brief Parameterized constructor
	 * @param subscriptionManager Subscription manager instance
	 * @param publicationManager Publication manager instance
	 * @param inProcessPublicationSender InProcessPublicationSender instance,
	 * used to transfer publications from the PublicationManager to the (local) SubscriptionManager.
	 * @param proxyParticipantId The participant id of the proxy
	 * @param providerParticipantId The participant id of the provider
	 * @param address The address
	 */
	«interfaceName»InProcessConnector(
				joynr::ISubscriptionManager* subscriptionManager,
				joynr::PublicationManager* publicationManager,
				joynr::InProcessPublicationSender* inProcessPublicationSender,
				const std::string& proxyParticipantId,
				const std::string& providerParticipantId,
				std::shared_ptr<joynr::InProcessAddress> address
	);

	/**
	 * @brief Checks whether cluster controller is used
	 * @return true, if cluster controller is used
	 */
	bool usesClusterController() const override;

	«produceSyncGetterDeclarations(serviceInterface, false)»
	«produceSyncSetterDeclarations(serviceInterface, false)»
	«produceSyncMethodDeclarations(serviceInterface, false)»
	«produceAsyncGetterDeclarations(serviceInterface, false)»
	«produceAsyncSetterDeclarations(serviceInterface, false)»
	«produceAsyncMethodDeclarations(serviceInterface, false, true)»

	«produceSubscribeUnsubscribeMethods(serviceInterface, false)»

private:
	ADD_LOGGER(«interfaceName»InProcessConnector);

	DISALLOW_COPY_AND_ASSIGN(«interfaceName»InProcessConnector);
	std::string proxyParticipantId;
	std::string providerParticipantId;
	std::shared_ptr<joynr::InProcessAddress> address;
	joynr::ISubscriptionManager* subscriptionManager;
	joynr::PublicationManager* publicationManager;
	joynr::InProcessPublicationSender* inProcessPublicationSender;
};
«getNamespaceEnder(serviceInterface)»

namespace joynr {

«var packagePrefix = getPackagePathWithJoynrPrefix(serviceInterface, "::")»

// Helper class for use by the InProcessConnectorFactory
// This class creates instances of «interfaceName»InProcessConnector
template <>
class InProcessConnectorFactoryHelper <«packagePrefix»::I«interfaceName»Connector> {
public:
	«packagePrefix»::«interfaceName»InProcessConnector* create(
			ISubscriptionManager* subscriptionManager,
			PublicationManager* publicationManager,
			InProcessPublicationSender* inProcessPublicationSender,
			const std::string& proxyParticipantId,
			const std::string& providerParticipantId,
			std::shared_ptr<InProcessAddress> address
		) {
		return new «packagePrefix»::«interfaceName»InProcessConnector(
				subscriptionManager,
				publicationManager,
				inProcessPublicationSender,
				proxyParticipantId,
				providerParticipantId,
				address
		);
	}
};
} // namespace joynr

#endif // «headerGuard»
'''

}
