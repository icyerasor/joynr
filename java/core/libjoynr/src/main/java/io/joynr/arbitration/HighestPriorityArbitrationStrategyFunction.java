package io.joynr.arbitration;

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

import java.util.Collection;
import java.util.Map;

import joynr.types.DiscoveryEntry;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * Arbitrator using a custom parameter in the QoS map called "priority". The provider with the highest priority value is
 * chosen.
 *
 */
public class HighestPriorityArbitrationStrategyFunction extends ArbitrationStrategyFunction {
    private static final Logger logger = LoggerFactory.getLogger(HighestPriorityArbitrationStrategyFunction.class);

    @Override
    public final DiscoveryEntry select(Map<String, String> parameters, final Collection<DiscoveryEntry> capabilities) {
        logger.trace("starting select Provider by priority");
        DiscoveryEntry highestPriorityCapability = null;
        long highestPriority = -1L;
        for (DiscoveryEntry discoveryEntry : capabilities) {
            // Search for the provider with the highest priority
            Long priority = discoveryEntry.getQos().getPriority();
            logger.trace("Looking at capability with priority " + priority.toString());
            if (highestPriority < priority) {
                highestPriority = priority;
                highestPriorityCapability = discoveryEntry;
            }
        }
        logger.trace("capability with highest priority: " + highestPriority + "\r\n" + highestPriorityCapability);

        return highestPriorityCapability;
    }
}
