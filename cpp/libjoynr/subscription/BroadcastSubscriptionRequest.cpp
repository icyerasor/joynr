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
#include "joynr/BroadcastSubscriptionRequest.h"
#include "joynr/JsonSerializer.h"

#include <cassert>

namespace joynr
{

INIT_LOGGER(BroadcastSubscriptionRequest);

// Register the BroadcastSubscriptionRequest type id
static const bool isBroadcastSubscriptionRequestRegistered =
        Variant::registerType<BroadcastSubscriptionRequest>("joynr.BroadcastSubscriptionRequest");

BroadcastSubscriptionRequest::BroadcastSubscriptionRequest() : filterParameters()
{
}

BroadcastSubscriptionRequest::BroadcastSubscriptionRequest(
        const BroadcastSubscriptionRequest& subscriptionRequest)
        : SubscriptionRequest(subscriptionRequest),
          filterParameters(subscriptionRequest.getFilterParameters())
{
}

BroadcastSubscriptionRequest& BroadcastSubscriptionRequest::operator=(
        const BroadcastSubscriptionRequest& subscriptionRequest)
{
    SubscriptionRequest::operator=(subscriptionRequest);
    filterParameters = subscriptionRequest.getFilterParameters();
    return *this;
}

bool BroadcastSubscriptionRequest::operator==(
        const BroadcastSubscriptionRequest& subscriptionRequest) const
{

    bool equal = getQos() == subscriptionRequest.getQos() &&
                 getFilterParameters() == subscriptionRequest.getFilterParameters();
    return getSubscriptionId() == subscriptionRequest.getSubscriptionId() &&
           getSubscribeToName() == subscriptionRequest.getSubscribeToName() && equal;
}

std::string BroadcastSubscriptionRequest::toString() const
{
    return JsonSerializer::serialize(*this);
}

void BroadcastSubscriptionRequest::setQos(const OnChangeSubscriptionQos& qos)
{
    Variant qosVariant = Variant::make<OnChangeSubscriptionQos>(qos);
    SubscriptionRequest::setQos(qosVariant);
}

BroadcastFilterParameters BroadcastSubscriptionRequest::getFilterParameters() const
{
    return filterParameters;
}

void BroadcastSubscriptionRequest::setFilterParameters(
        const BroadcastFilterParameters& filterParameters)
{
    this->filterParameters = filterParameters;
}

} // namespace joynr
