/*
 * #%L
 * %%
 * Copyright (C) 2011 - 2013 BMW Car IT GmbH
 * %%
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
 * #L%
 */

#include "joynr/JoynrMessageSender.h"
#include "joynr/IMessaging.h"
#include "joynr/IDispatcher.h"
#include "joynr/Request.h"
#include "joynr/MessageRouter.h"

#include <cassert>

namespace joynr
{

INIT_LOGGER(JoynrMessageSender);

JoynrMessageSender::JoynrMessageSender(std::shared_ptr<MessageRouter> messageRouter)
        : dispatcher(nullptr), messageRouter(messageRouter), messageFactory()
{
}

void JoynrMessageSender::registerDispatcher(IDispatcher* dispatcher)
{
    this->dispatcher = dispatcher;
}

void JoynrMessageSender::sendRequest(const std::string& senderParticipantId,
                                     const std::string& receiverParticipantId,
                                     const MessagingQos& qos,
                                     const Request& request,
                                     std::shared_ptr<IReplyCaller> callback)
{
    assert(dispatcher != nullptr);

    try {
        dispatcher->addReplyCaller(request.getRequestReplyId(), callback, qos);
        JoynrMessage message = messageFactory.createRequest(
                senderParticipantId, receiverParticipantId, qos, request);
        assert(messageRouter);
        messageRouter->route(message);
    } catch (std::invalid_argument exception) {
        throw joynr::exceptions::MethodInvocationException(exception.what());
    }
}

void JoynrMessageSender::sendReply(const std::string& senderParticipantId,
                                   const std::string& receiverParticipantId,
                                   const MessagingQos& qos,
                                   const Reply& reply)
{
    try {
        JoynrMessage message =
                messageFactory.createReply(senderParticipantId, receiverParticipantId, qos, reply);
        assert(messageRouter);
        messageRouter->route(message);
    } catch (std::invalid_argument exception) {
        throw joynr::exceptions::MethodInvocationException(exception.what());
    }
}

void JoynrMessageSender::sendSubscriptionRequest(const std::string& senderParticipantId,
                                                 const std::string& receiverParticipantId,
                                                 const MessagingQos& qos,
                                                 const SubscriptionRequest& subscriptionRequest)
{
    try {
        JoynrMessage message = messageFactory.createSubscriptionRequest(
                senderParticipantId, receiverParticipantId, qos, subscriptionRequest);
        assert(messageRouter);
        messageRouter->route(message);
    } catch (std::invalid_argument exception) {
        throw joynr::exceptions::MethodInvocationException(exception.what());
    }
}

void JoynrMessageSender::sendBroadcastSubscriptionRequest(
        const std::string& senderParticipantId,
        const std::string& receiverParticipantId,
        const MessagingQos& qos,
        const BroadcastSubscriptionRequest& subscriptionRequest)
{
    try {
        JoynrMessage message = messageFactory.createBroadcastSubscriptionRequest(
                senderParticipantId, receiverParticipantId, qos, subscriptionRequest);
        assert(messageRouter);
        messageRouter->route(message);
    } catch (std::invalid_argument exception) {
        throw joynr::exceptions::MethodInvocationException(exception.what());
    }
}

void JoynrMessageSender::sendSubscriptionReply(const std::string& senderParticipantId,
                                               const std::string& receiverParticipantId,
                                               const MessagingQos& qos,
                                               const SubscriptionReply& subscriptionReply)
{
    try {
        JoynrMessage message = messageFactory.createSubscriptionReply(
                senderParticipantId, receiverParticipantId, qos, subscriptionReply);
        assert(messageRouter);
        messageRouter->route(message);
    } catch (std::invalid_argument exception) {
        throw joynr::exceptions::MethodInvocationException(exception.what());
    }
}

void JoynrMessageSender::sendSubscriptionStop(const std::string& senderParticipantId,
                                              const std::string& receiverParticipantId,
                                              const MessagingQos& qos,
                                              const SubscriptionStop& subscriptionStop)
{
    try {
        JoynrMessage message = messageFactory.createSubscriptionStop(
                senderParticipantId, receiverParticipantId, qos, subscriptionStop);
        assert(messageRouter);
        messageRouter->route(message);
    } catch (std::invalid_argument exception) {
        throw joynr::exceptions::MethodInvocationException(exception.what());
    }
}

void JoynrMessageSender::sendSubscriptionPublication(
        const std::string& senderParticipantId,
        const std::string& receiverParticipantId,
        const MessagingQos& qos,
        const SubscriptionPublication& subscriptionPublication)
{
    try {
        JoynrMessage message = messageFactory.createSubscriptionPublication(
                senderParticipantId, receiverParticipantId, qos, subscriptionPublication);
        assert(messageRouter);
        messageRouter->route(message);
    } catch (std::invalid_argument exception) {
        throw joynr::exceptions::MethodInvocationException(exception.what());
    }
}

} // namespace joynr
