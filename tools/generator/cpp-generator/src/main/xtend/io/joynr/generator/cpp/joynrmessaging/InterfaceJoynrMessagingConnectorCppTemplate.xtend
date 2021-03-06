package io.joynr.generator.cpp.joynrmessaging
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
import io.joynr.generator.cpp.util.JoynrCppGeneratorExtensions
import io.joynr.generator.cpp.util.TemplateBase
import io.joynr.generator.templates.InterfaceTemplate
import io.joynr.generator.templates.util.AttributeUtil
import io.joynr.generator.templates.util.BroadcastUtil
import io.joynr.generator.templates.util.MethodUtil
import io.joynr.generator.templates.util.NamingUtil
import java.io.File
import org.franca.core.franca.FInterface
import org.franca.core.franca.FMethod

class InterfaceJoynrMessagingConnectorCppTemplate implements InterfaceTemplate{

	@Inject private extension TemplateBase
	@Inject private extension CppStdTypeUtil
	@Inject private extension JoynrCppGeneratorExtensions
	@Inject private extension NamingUtil
	@Inject private extension AttributeUtil
	@Inject private extension MethodUtil
	@Inject private extension BroadcastUtil
	@Inject private extension CppInterfaceUtil

	def produceParameterSetters(FMethod method)
'''
joynr::Request internalRequestObject;
internalRequestObject.setMethodName("«method.joynrName»");
«FOR param : getInputParameters(method)»
	«IF isEnum(param.type) && isArray(param)»
		internalRequestObject.addParam(joynr::TypeUtil::toVariant(Util::convertEnumVectorToVariantVector<«getTypeNameOfContainingClass(param.type.derived)»>(«param.name»)), "«getJoynrTypeName(param)»");
	«ELSEIF isEnum(param.type)»
		internalRequestObject.addParam(Variant::make<«getTypeName(param)»>(«param.name»), "«getJoynrTypeName(param)»");
	«ELSEIF isArray(param)»
		internalRequestObject.addParam(joynr::TypeUtil::toVariant<«getTypeName(param.type)»>(«param.name»), "«getJoynrTypeName(param)»");
	«ELSEIF isCompound(param.type)»
		internalRequestObject.addParam(Variant::make<«getTypeName(param)»>(«param.name»), "«getJoynrTypeName(param)»");
	«ELSEIF isMap(param.type)»
		internalRequestObject.addParam(Variant::make<«getTypeName(param)»>(«param.name»), "«getJoynrTypeName(param)»");
	«ELSEIF isByteBuffer(param.type)»
		internalRequestObject.addParam(joynr::TypeUtil::toVariant(«param.name»), "«getJoynrTypeName(param)»");
	«ELSE»
		internalRequestObject.addParam(Variant::make<«getTypeName(param)»>(«param.name»), "«getJoynrTypeName(param)»");
	«ENDIF»
«ENDFOR»
'''

	override generate(FInterface serviceInterface)
'''
«val interfaceName = serviceInterface.joynrName»
«val methodToErrorEnumName = serviceInterface.methodToErrorEnumName()»
«warning()»

#include "«getPackagePathWithJoynrPrefix(serviceInterface, "/")»/«interfaceName»JoynrMessagingConnector.h"
#include "joynr/ReplyCaller.h"
#include "joynr/JoynrMessageSender.h"
#include "joynr/ISubscriptionManager.h"
#include "joynr/SubscriptionCallback.h"
#include "joynr/BroadcastSubscriptionRequest.h"
#include "joynr/Util.h"
#include "joynr/TypeUtil.h"
#include "joynr/SubscriptionStop.h"
#include "joynr/Future.h"
#include <chrono>
#include <cstdint>
#include "joynr/SubscriptionUtil.h"
#include "joynr/exceptions/JoynrException.h"

«FOR method : getMethods(serviceInterface)»
	«IF method.hasErrorEnum»
		«var enumType = method.errors»
		«IF enumType != null»
			«enumType.name = methodToErrorEnumName.get(method)»
		«ELSE»
			«{enumType = method.errorEnum; ""}»
		«ENDIF»
		#include "«enumType.getPackagePathWithJoynrPrefix(File::separator, true) + File::separator + enumType.joynrName».h"
	«ENDIF»
«ENDFOR»

«FOR datatype: getAllComplexTypes(serviceInterface)»
	«IF isCompound(datatype) || isMap(datatype)»
		#include «getIncludeOf(datatype)»
	«ENDIF»
«ENDFOR»

«getNamespaceStarter(serviceInterface)»
«val className = interfaceName + "JoynrMessagingConnector"»
«className»::«className»(
		joynr::IJoynrMessageSender* joynrMessageSender,
		joynr::ISubscriptionManager* subscriptionManager,
		const std::string &domain,
		const std::string proxyParticipantId,
		const std::string& providerParticipantId,
		const joynr::MessagingQos &qosSettings,
		joynr::IClientCache *cache,
		bool cached)
	: joynr::AbstractJoynrMessagingConnector(joynrMessageSender, subscriptionManager, domain, INTERFACE_NAME(), proxyParticipantId, providerParticipantId, qosSettings, cache, cached)
{
}

bool «className»::usesClusterController() const{
	return joynr::AbstractJoynrMessagingConnector::usesClusterController();
}

«FOR attribute: getAttributes(serviceInterface)»
	«val returnType = getTypeName(attribute)»
	«val attributeName = attribute.joynrName»
	«IF attribute.readable»
		«produceSyncGetterSignature(attribute, className)»
		{
			std::shared_ptr<joynr::Future<«returnType»> > future(new joynr::Future<«returnType»>());

			std::function<void(const «returnType»& «attributeName»)> onSuccess =
					[future] (const «returnType»& «attributeName») {
						future->onSuccess(«attributeName»);
					};

			std::function<void(const exceptions::JoynrException& error)> onError =
					[future] (const exceptions::JoynrException& error) {
						future->onError(error);
					};

			std::shared_ptr<joynr::IReplyCaller> replyCaller(new joynr::ReplyCaller<«returnType»>(
					onSuccess,
					onError));
			attributeRequest<«returnType»>("get«attributeName.toFirstUpper»", replyCaller);
			future->get(«attributeName»);
		}

		«produceAsyncGetterSignature(attribute, className)»
		{
			std::shared_ptr<joynr::Future<«returnType»> > future(new joynr::Future<«returnType»>());

			std::function<void(const «returnType»& «attributeName»)> onSuccessWrapper =
					[future, onSuccess] (const «returnType»& «attributeName») {
						future->onSuccess(«attributeName»);
						if (onSuccess){
							onSuccess(«attributeName»);
						}
					};

			std::function<void(const exceptions::JoynrException& error)> onErrorWrapper =
					[future, onError] (const exceptions::JoynrException& error) {
						future->onError(error);
						if (onError){
							onError(static_cast<const exceptions::JoynrRuntimeException&>(error));
						}
					};

			std::shared_ptr<joynr::IReplyCaller> replyCaller(new joynr::ReplyCaller<«returnType»>(
					onSuccessWrapper,
					onErrorWrapper));
			attributeRequest<«returnType»>("get«attributeName.toFirstUpper»", replyCaller);

			return future;
		}

	«ENDIF»
	«IF attribute.writable»
		«produceAsyncSetterSignature(attribute, className)»
		{
			joynr::Request internalRequestObject;
			internalRequestObject.setMethodName("set«attributeName.toFirstUpper»");
			«IF isEnum(attribute.type) && isArray(attribute)»
				internalRequestObject.addParam(joynr::TypeUtil::toVariant(Util::convertEnumVectorToVariantVector<«getTypeNameOfContainingClass(attribute.type.derived)»>(«attributeName»)), "«getJoynrTypeName(attribute)»");
			«ELSEIF isEnum(attribute.type)»
				internalRequestObject.addParam(Variant::make<«getTypeName(attribute)»>(«attributeName»), "«getJoynrTypeName(attribute)»");
			«ELSEIF isArray(attribute)»
				internalRequestObject.addParam(joynr::TypeUtil::toVariant<«getTypeName(attribute.type)»>(«attributeName»), "«getJoynrTypeName(attribute)»");
			«ELSEIF isCompound(attribute.type)»
				internalRequestObject.addParam(Variant::make<«getTypeName(attribute)»>(«attributeName»), "«getJoynrTypeName(attribute)»");
			«ELSEIF isMap(attribute.type)»
				internalRequestObject.addParam(Variant::make<«getTypeName(attribute)»>(«attributeName»), "«getJoynrTypeName(attribute)»");
			«ELSEIF isByteBuffer(attribute.type)»
				internalRequestObject.addParam(joynr::TypeUtil::toVariant(«attributeName»), "«getJoynrTypeName(attribute)»");
			«ELSE»
				internalRequestObject.addParam(Variant::make<«getTypeName(attribute)»>(«attributeName»), "«getJoynrTypeName(attribute)»");
			«ENDIF»

			std::shared_ptr<joynr::Future<void>> future(new joynr::Future<void>());

			std::function<void()> onSuccessWrapper =
					[future, onSuccess] () {
						future->onSuccess();
						if (onSuccess) {
							onSuccess();
						}
					};

			std::function<void(const exceptions::JoynrException& error)> onErrorWrapper =
				[future, onError] (const exceptions::JoynrException& error) {
					future->onError(error);
					if (onError) {
						onError(static_cast<const exceptions::JoynrRuntimeException&>(error));
					}
				};

			std::shared_ptr<joynr::IReplyCaller> replyCaller(new joynr::ReplyCaller<void>(
					onSuccessWrapper,
					onErrorWrapper));
			operationRequest(replyCaller, internalRequestObject);
			return future;
		}

		«produceSyncSetterSignature(attribute, className)»
		{
			joynr::Request internalRequestObject;
			internalRequestObject.setMethodName("set«attributeName.toFirstUpper»");
			«IF isEnum(attribute.type) && isArray(attribute)»
				internalRequestObject.addParam(joynr::TypeUtil::toVariant(Util::convertEnumVectorToVariantVector<«getTypeNameOfContainingClass(attribute.type.derived)»>(«attributeName»)), "«getJoynrTypeName(attribute)»");
			«ELSEIF isEnum(attribute.type)»
				internalRequestObject.addParam(Variant::make<«getTypeName(attribute)»>(«attributeName»), "«getJoynrTypeName(attribute)»");
			«ELSEIF isArray(attribute)»
				internalRequestObject.addParam(joynr::TypeUtil::toVariant<«getTypeName(attribute.type)»>(«attributeName»), "«getJoynrTypeName(attribute)»");
			«ELSEIF isCompound(attribute.type)»
				internalRequestObject.addParam(Variant::make<«getTypeName(attribute)»>(«attributeName»), "«getJoynrTypeName(attribute)»");
			«ELSEIF isMap(attribute.type)»
				internalRequestObject.addParam(Variant::make<«getTypeName(attribute)»>(«attributeName»), "«getJoynrTypeName(attribute)»");
			«ELSEIF isByteBuffer(attribute.type)»
				internalRequestObject.addParam(joynr::TypeUtil::toVariant(«attributeName»), "«getJoynrTypeName(attribute)»");
			«ELSE»
				internalRequestObject.addParam(Variant::make<«getTypeName(attribute)»>(«attributeName»), "«getJoynrTypeName(attribute)»");
			«ENDIF»

			std::shared_ptr<joynr::Future<void> > future( new joynr::Future<void>());

			std::function<void()> onSuccess =
					[future] () {
						future->onSuccess();
					};

			std::function<void(const exceptions::JoynrException& error)> onError =
					[future] (const exceptions::JoynrException& error) {
						future->onError(error);
					};

			std::shared_ptr<joynr::IReplyCaller> replyCaller(new joynr::ReplyCaller<void>(
					onSuccess,
					onError));
			operationRequest(replyCaller, internalRequestObject);
			future->get();
		}

	«ENDIF»
	«IF attribute.notifiable»
		std::string «className»::subscribeTo«attributeName.toFirstUpper»(
					std::shared_ptr<joynr::ISubscriptionListener<«returnType» > > subscriptionListener,
					const joynr::SubscriptionQos& subscriptionQos
		) {
			joynr::SubscriptionRequest subscriptionRequest;
			return subscribeTo«attributeName.toFirstUpper»(subscriptionListener, subscriptionQos, subscriptionRequest);
		}

		std::string «className»::subscribeTo«attributeName.toFirstUpper»(
					std::shared_ptr<joynr::ISubscriptionListener<«returnType» > > subscriptionListener,
					const joynr::SubscriptionQos& subscriptionQos,
					std::string& subscriptionId
		) {

			joynr::SubscriptionRequest subscriptionRequest;
			subscriptionRequest.setSubscriptionId(subscriptionId);
			return subscribeTo«attributeName.toFirstUpper»(subscriptionListener, subscriptionQos, subscriptionRequest);
		}

		std::string «className»::subscribeTo«attributeName.toFirstUpper»(
					std::shared_ptr<joynr::ISubscriptionListener<«returnType»> > subscriptionListener,
					const joynr::SubscriptionQos& subscriptionQos,
					SubscriptionRequest& subscriptionRequest
		) {
			JOYNR_LOG_DEBUG(logger, "Subscribing to «attributeName».");
			std::string attributeName("«attributeName»");
			joynr::MessagingQos clonedMessagingQos(qosSettings);
			if (subscriptionQos.getExpiryDate() == joynr::SubscriptionQos::NO_EXPIRY_DATE()) {
				clonedMessagingQos.setTtl(joynr::SubscriptionQos::NO_EXPIRY_DATE_TTL());
			}
			else{
				std::int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
				clonedMessagingQos.setTtl(subscriptionQos.getExpiryDate() - now);
			}

			std::shared_ptr<joynr::SubscriptionCallback<«returnType»>> subscriptionCallback(new joynr::SubscriptionCallback<«returnType»>(subscriptionListener));
			subscriptionManager->registerSubscription(
						attributeName,
						subscriptionCallback,
						SubscriptionUtil::getVariant(subscriptionQos),
						subscriptionRequest);
			JOYNR_LOG_DEBUG(logger, subscriptionRequest.toString());
			joynrMessageSender->sendSubscriptionRequest(
						proxyParticipantId,
						providerParticipantId,
						clonedMessagingQos,
						subscriptionRequest
			);
			return subscriptionRequest.getSubscriptionId();
		}

		void «className»::unsubscribeFrom«attributeName.toFirstUpper»(
				std::string& subscriptionId
		) {
			joynr::SubscriptionStop subscriptionStop;
			subscriptionStop.setSubscriptionId(subscriptionId);

			subscriptionManager->unregisterSubscription(subscriptionId);
			joynrMessageSender->sendSubscriptionStop(
						proxyParticipantId,
						providerParticipantId,
						qosSettings,
						subscriptionStop
			);
		}

	«ENDIF»
«ENDFOR»

«FOR method: getMethods(serviceInterface)»
	«var outputTypedConstParamList = getCommaSeperatedTypedConstOutputParameterList(method)»
	«val outputParameters = getCommaSeparatedOutputParameterTypes(method)»
	«var outputUntypedParamList = getCommaSeperatedUntypedOutputParameterList(method)»

	«produceSyncMethodSignature(method, className)»
	{
		«produceParameterSetters(method)»
		std::shared_ptr<joynr::Future<«outputParameters»> > future(
				new joynr::Future<«outputParameters»>());

		std::function<void(«outputTypedConstParamList»)> onSuccess =
				[future] («outputTypedConstParamList») {
					future->onSuccess(«outputUntypedParamList»);
				};

		std::function<void(const exceptions::JoynrException& error)> onError =
			[future] (const exceptions::JoynrException& error) {
				future->onError(error);
			};

		std::shared_ptr<joynr::IReplyCaller> replyCaller(new joynr::ReplyCaller<«outputParameters»>(
				onSuccess,
				onError));
		operationRequest(replyCaller, internalRequestObject);
		future->get(«getCommaSeperatedUntypedOutputParameterList(method)»);
	}

	«produceAsyncMethodSignature(serviceInterface, method, className)»
	{
		«produceParameterSetters(method)»

		std::shared_ptr<joynr::Future<«outputParameters»> > future(
				new joynr::Future<«outputParameters»>());

		std::function<void(«outputTypedConstParamList»)> onSuccessWrapper =
				[future, onSuccess] («outputTypedConstParamList») {
					future->onSuccess(«outputUntypedParamList»);
					if (onSuccess) {
						onSuccess(«outputUntypedParamList»);
					}
				};

		std::function<void(const exceptions::JoynrException& error)> onErrorWrapper =
				[future, onRuntimeError«IF method.hasErrorEnum», onApplicationError«ENDIF»] (const exceptions::JoynrException& error) {
				future->onError(error);
				«produceApplicationRuntimeErrorSplitForOnErrorWrapper(serviceInterface, method)»
			};

		std::shared_ptr<joynr::IReplyCaller> replyCaller(new joynr::ReplyCaller<«outputParameters»>(
				onSuccessWrapper,
				onErrorWrapper));
		operationRequest(replyCaller, internalRequestObject);
		return future;
	}

«ENDFOR»

«FOR broadcast: serviceInterface.broadcasts»
	«val returnTypes = getCommaSeparatedOutputParameterTypes(broadcast)»
	«val broadcastName = broadcast.joynrName»
	«IF isSelective(broadcast)»
		std::string «className»::subscribeTo«broadcastName.toFirstUpper»Broadcast(
					const «interfaceName.toFirstUpper»«broadcastName.toFirstUpper»BroadcastFilterParameters& filterParameters,
					std::shared_ptr<joynr::ISubscriptionListener<«returnTypes» > > subscriptionListener,
					const joynr::OnChangeSubscriptionQos& subscriptionQos
	«ELSE»
		std::string «className»::subscribeTo«broadcastName.toFirstUpper»Broadcast(
					std::shared_ptr<joynr::ISubscriptionListener<«returnTypes» > > subscriptionListener,
					const joynr::OnChangeSubscriptionQos& subscriptionQos
	«ENDIF»
	) {
		joynr::BroadcastSubscriptionRequest subscriptionRequest;
		«IF isSelective(broadcast)»
			subscriptionRequest.setFilterParameters(filterParameters);
		«ENDIF»
		return subscribeTo«broadcastName.toFirstUpper»Broadcast(subscriptionListener, subscriptionQos, subscriptionRequest);
	}

	«IF isSelective(broadcast)»
		std::string «className»::subscribeTo«broadcastName.toFirstUpper»Broadcast(
					const «interfaceName.toFirstUpper»«broadcastName.toFirstUpper»BroadcastFilterParameters& filterParameters,
					std::shared_ptr<joynr::ISubscriptionListener<«returnTypes» > > subscriptionListener,
					const joynr::OnChangeSubscriptionQos& subscriptionQos,
					std::string& subscriptionId
	«ELSE»
		std::string «className»::subscribeTo«broadcastName.toFirstUpper»Broadcast(
					std::shared_ptr<joynr::ISubscriptionListener<«returnTypes» > > subscriptionListener,
					const joynr::OnChangeSubscriptionQos& subscriptionQos,
					std::string& subscriptionId
	«ENDIF»
	) {
		joynr::BroadcastSubscriptionRequest subscriptionRequest;
		«IF isSelective(broadcast)»
			subscriptionRequest.setFilterParameters(filterParameters);
		«ENDIF»
		subscriptionRequest.setSubscriptionId(subscriptionId);
		return subscribeTo«broadcastName.toFirstUpper»Broadcast(subscriptionListener, subscriptionQos, subscriptionRequest);
	}

	std::string «className»::subscribeTo«broadcastName.toFirstUpper»Broadcast(
				std::shared_ptr<joynr::ISubscriptionListener<«returnTypes» > > subscriptionListener,
				const joynr::OnChangeSubscriptionQos& subscriptionQos,
				BroadcastSubscriptionRequest& subscriptionRequest
	) {
		JOYNR_LOG_DEBUG(logger, "Subscribing to «broadcastName» broadcast.");
		std::string broadcastName("«broadcastName»");
		joynr::MessagingQos clonedMessagingQos(qosSettings);
		if (subscriptionQos.getExpiryDate() == joynr::SubscriptionQos::NO_EXPIRY_DATE()) {
			clonedMessagingQos.setTtl(joynr::SubscriptionQos::NO_EXPIRY_DATE_TTL());
		}
		else{
			std::int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
			clonedMessagingQos.setTtl(subscriptionQos.getExpiryDate() - now);
		}

		std::shared_ptr<joynr::SubscriptionCallback<«returnTypes»>> subscriptionCallback(
					new joynr::SubscriptionCallback<«returnTypes»>(subscriptionListener));
		subscriptionManager->registerSubscription(
					broadcastName,
					subscriptionCallback,
					Variant::make<OnChangeSubscriptionQos>(subscriptionQos),
					subscriptionRequest);
		JOYNR_LOG_DEBUG(logger, subscriptionRequest.toString());
		joynrMessageSender->sendBroadcastSubscriptionRequest(
					proxyParticipantId,
					providerParticipantId,
					clonedMessagingQos,
					subscriptionRequest
		);
		return subscriptionRequest.getSubscriptionId();
	}

	void «className»::unsubscribeFrom«broadcastName.toFirstUpper»Broadcast(
			std::string& subscriptionId
	) {
		joynr::SubscriptionStop subscriptionStop;
		subscriptionStop.setSubscriptionId(subscriptionId);

		subscriptionManager->unregisterSubscription(subscriptionId);
		joynrMessageSender->sendSubscriptionStop(
					proxyParticipantId,
					providerParticipantId,
					qosSettings,
					subscriptionStop
		);
	}

«ENDFOR»
«getNamespaceEnder(serviceInterface)»
'''
}

