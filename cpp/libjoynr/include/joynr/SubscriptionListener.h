/*
 * #%L
 * %%
 * Copyright (C) 2011 - 2015 BMW Car IT GmbH
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
#ifndef SUBSCRIPTIONLISTENER_H
#define SUBSCRIPTIONLISTENER_H

#include "joynr/ISubscriptionListener.h"

namespace joynr
{

/**
 * @brief Base class providing an emtpy implementation of ISubscriptionListener
 *
 * Subscription listeners that are not interested in all callbacks could extend
 * this class.
 */
template <typename T, typename... Ts>
class SubscriptionListener : public ISubscriptionListener<T, Ts...>
{
public:
    /** Default Constructor */
    SubscriptionListener()
    {
    }

    /** Default Destructor */
    virtual ~SubscriptionListener() = default;

    /**
     * @brief Method to be called on receiving publication
     * @param value First output parameter of the broadcast | attribute value
     * @param values Optional 2nd..nth output parameter in case of a broadcast
     */
    virtual void onReceive(T value, Ts... values)
    {
    }

    /**
     * @brief Method to be called on missing a publication
     */
    virtual void onError(const exceptions::JoynrRuntimeException& error)
    {
    }
};

} // namespace joynr
#endif // SUBSCRIPTIONLISTENER_H
