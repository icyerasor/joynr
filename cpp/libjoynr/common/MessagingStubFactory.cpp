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
#include "joynr/MessagingStubFactory.h"

#include "joynr/IMiddlewareMessagingStubFactory.h"

namespace joynr
{

MessagingStubFactory::~MessagingStubFactory()
{
    while (!factoryList.empty()) {
        auto it = factoryList.begin();
        delete *it;
        factoryList.erase(it);
    }
}

MessagingStubFactory::MessagingStubFactory()
        : address2MessagingStubDirectory("MessagingStubFactory-MessagingStubDirectory"),
          factoryList(),
          mutex()
{
}

std::shared_ptr<IMessaging> MessagingStubFactory::create(
        const joynr::system::RoutingTypes::Address& destinationAddress)
{
    {
        std::lock_guard<std::mutex> lock(this->mutex);

        if (!address2MessagingStubDirectory.contains(destinationAddress)) {
            // search for the corresponding factory
            for (std::vector<IMiddlewareMessagingStubFactory*>::iterator it =
                         this->factoryList.begin();
                 it != factoryList.end();
                 ++it) {
                if ((*it)->canCreate(destinationAddress)) {
                    std::shared_ptr<IMessaging> stub = (*it)->create(destinationAddress);
                    address2MessagingStubDirectory.add(destinationAddress, stub);

                    return stub;
                }
            }
        }
    }

    return address2MessagingStubDirectory.lookup(destinationAddress);
}

void MessagingStubFactory::remove(const joynr::system::RoutingTypes::Address& destinationAddress)
{
    address2MessagingStubDirectory.remove(destinationAddress);
}

bool MessagingStubFactory::contains(const joynr::system::RoutingTypes::Address& destinationAddress)
{
    return address2MessagingStubDirectory.contains(destinationAddress);
}

void MessagingStubFactory::registerStubFactory(IMiddlewareMessagingStubFactory* factory)
{
    this->factoryList.push_back(factory);
}

} // namespace joynr
