/*
 * #%L
 * %%
 * Copyright (C) 2011 - 2014 BMW Car IT GmbH
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
#include "JoynrMessagingStubFactory.h"

#include "joynr/system/RoutingTypes/ChannelAddress.h"
#include "joynr/IMessageSender.h"
#include "cluster-controller/messaging/joynr-messaging/JoynrMessagingStub.h"

namespace joynr
{

JoynrMessagingStubFactory::JoynrMessagingStubFactory(std::shared_ptr<IMessageSender> messageSender,
                                                     std::string receiveChannelId)
        : messageSender(messageSender), receiveChannelId(receiveChannelId)
{
}

bool JoynrMessagingStubFactory::canCreate(const joynr::system::RoutingTypes::Address& destAddress)
{
    return dynamic_cast<const system::RoutingTypes::ChannelAddress*>(&destAddress);
}

std::shared_ptr<IMessaging> JoynrMessagingStubFactory::create(
        const joynr::system::RoutingTypes::Address& destAddress)
{
    const system::RoutingTypes::ChannelAddress* channelAddress =
            dynamic_cast<const system::RoutingTypes::ChannelAddress*>(&destAddress);
    return std::shared_ptr<IMessaging>(new JoynrMessagingStub(
            messageSender, channelAddress->getChannelId(), receiveChannelId));
}

} // namespace joynr
