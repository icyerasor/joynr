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

import joynr.types.CustomParameter;
import joynr.types.DiscoveryEntry;

public class KeywordArbitrationStrategyFunction extends ArbitrationStrategyFunction {

    @Override
    public DiscoveryEntry select(Map<String, String> parameters, Collection<DiscoveryEntry> capabilities) {
        String requestedKeyword = parameters.get(ArbitrationConstants.KEYWORD_PARAMETER);
        DiscoveryEntry capabilityWithKeyword = null;

        for (DiscoveryEntry discoveryEntry : capabilities) {
            // Search for a matching keyword parameter
            CustomParameter keywordParameter = findQosParameter(discoveryEntry, ArbitrationConstants.KEYWORD_PARAMETER);
            if (keywordParameter != null) {
                String currentKeyword = keywordParameter.getValue();
                if (currentKeyword.equals(requestedKeyword)) {
                    capabilityWithKeyword = discoveryEntry;
                    break;
                }
            }
        }

        return capabilityWithKeyword;
    }
}
