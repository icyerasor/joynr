/*
 * #%L
 * %%
 * Copyright (C) 2011 - 2016 BMW Car IT GmbH
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
#include "joynr/FixedParticipantArbitrator.h"
#include "joynr/ArbitrationStatus.h"
#include "joynr/system/IDiscovery.h"
#include "joynr/types/DiscoveryEntry.h"
#include "joynr/system/RoutingTypes/ChannelAddress.h"
#include "joynr/DiscoveryQos.h"
#include "joynr/exceptions/JoynrException.h"
#include "joynr/TypeUtil.h"

#include <cassert>

namespace joynr
{

INIT_LOGGER(FixedParticipantArbitrator);

FixedParticipantArbitrator::FixedParticipantArbitrator(
        const std::string& domain,
        const std::string& interfaceName,
        joynr::system::IDiscoverySync& discoveryProxy,
        const DiscoveryQos& discoveryQos)
        : ProviderArbitrator(domain, interfaceName, discoveryProxy, discoveryQos),
          participantId(discoveryQos.getCustomParameter("fixedParticipantId").getValue()),
          reqCacheDataFreshness(discoveryQos.getCacheMaxAge())
{
}

void FixedParticipantArbitrator::attemptArbitration()
{
    joynr::types::DiscoveryEntry result;
    try {
        discoveryProxy.lookup(result, participantId);
        joynr::types::CommunicationMiddleware::Enum preferredConnection(
                selectPreferredCommunicationMiddleware(result.getConnections()));
        updateArbitrationStatusParticipantIdAndAddress(
                ArbitrationStatus::ArbitrationSuccessful, participantId, preferredConnection);
    } catch (exceptions::JoynrException& e) {
        JOYNR_LOG_ERROR(logger,
                        "Unable to lookup provider (domain: {}, interface: {}) "
                        "from discovery. Error: {}",
                        domain,
                        interfaceName,
                        e.getMessage());
    }
}

} // namespace joynr
