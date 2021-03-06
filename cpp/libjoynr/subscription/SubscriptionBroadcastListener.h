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
#ifndef SUBSCRIPTIONBROADCASTLISTENER_H
#define SUBSCRIPTIONBROADCASTLISTENER_H

#include "joynr/JoynrExport.h"
#include "joynr/IBroadcastListener.h"

#include <string>
#include <memory>
#include <vector>

#include "joynr/Variant.h"

namespace joynr
{

class PublicationManager;
class IBroadcastFilter;

/**
 * An attribute listener used for broadcast subscriptions
 */
class JOYNR_EXPORT SubscriptionBroadcastListener : public IBroadcastListener
{
public:
    /**
     * Create an broadcast listener linked to a subscription
     */
    SubscriptionBroadcastListener(const std::string& subscriptionId,
                                  PublicationManager& publicationManager);

    // Implementation of IBroadcastListener::receive
    void broadcastOccurred(const std::vector<Variant>& values,
                           const std::vector<std::shared_ptr<IBroadcastFilter>>& filters) override;

private:
    std::string subscriptionId;
    PublicationManager& publicationManager;
};

} // namespace joynr

#endif // SUBSCRIPTIONBROADCASTLISTENER_H
