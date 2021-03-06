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
#include "joynr/ProviderArbitratorFactory.h"
#include "joynr/exceptions/JoynrException.h"
#include "joynr/system/IDiscovery.h"

namespace joynr
{

ProviderArbitrator* ProviderArbitratorFactory::createArbitrator(
        const std::string& domain,
        const std::string& interfaceName,
        joynr::system::IDiscoverySync& discoveryProxy,
        const DiscoveryQos& discoveryQos)
{
    DiscoveryQos::ArbitrationStrategy strategy = discoveryQos.getArbitrationStrategy();
    switch (strategy) {
    case DiscoveryQos::ArbitrationStrategy::NOT_SET:
        return new DefaultArbitrator(domain, interfaceName, discoveryProxy, discoveryQos);
        break;
    case DiscoveryQos::ArbitrationStrategy::FIXED_PARTICIPANT:
        return new FixedParticipantArbitrator(domain, interfaceName, discoveryProxy, discoveryQos);
    case DiscoveryQos::ArbitrationStrategy::LOCAL_ONLY:
        throw exceptions::DiscoveryException("Arbitration: Local-only not implemented yet.");
    case DiscoveryQos::ArbitrationStrategy::HIGHEST_PRIORITY:
        return new QosArbitrator(domain, interfaceName, discoveryProxy, discoveryQos);
    case DiscoveryQos::ArbitrationStrategy::KEYWORD:
        if (discoveryQos.getCustomParameters().count("keyword") == 0) {
            throw exceptions::DiscoveryException(
                    "KeywordArbitrator creation failed: keyword not set");
        }
        return new KeywordArbitrator(domain, interfaceName, discoveryProxy, discoveryQos);
    default:
        throw exceptions::DiscoveryException("Arbitrator creation failed: Invalid strategy!");
    }
}

} // namespace joynr
