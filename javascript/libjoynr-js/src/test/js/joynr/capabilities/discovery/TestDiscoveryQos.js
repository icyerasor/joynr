/*global joynrTestRequire: true */

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

joynrTestRequire("joynr/capabilities/discovery/TestDisoveryQos", [
    "joynr/proxy/DiscoveryQos",
    "joynr/types/ArbitrationStrategyCollection",
    "joynr/types/DiscoveryScope"
], function(DiscoveryQos, ArbitrationStrategyCollection, DiscoveryScope) {

    describe("libjoynr-js.joynr.capabilities.arbitration.DiscoveryQos", function() {
        it("is instantiable", function() {
            expect(new DiscoveryQos()).toBeDefined();
            expect(new DiscoveryQos({
                discoveryTimeout : 5000,
                discoveryRetryDelay : 0,
                arbitrationStrategy : ArbitrationStrategyCollection.HighestPriority,
                cacheMaxAge : 0,
                discoveryScope : DiscoveryScope.LOCAL_AND_GLOBAL,
                additionalParameters : {}
            })).toBeDefined();
        });

        it("is of correct type", function() {
            var emptyDiscoveryQos = new DiscoveryQos();
            expect(emptyDiscoveryQos).toBeDefined();
            expect(emptyDiscoveryQos).not.toBeNull();
            expect(typeof emptyDiscoveryQos === "object").toBeTruthy();
            expect(emptyDiscoveryQos instanceof DiscoveryQos).toBeTruthy();

            var defaultDiscoveryQos = new DiscoveryQos({
                discoveryTimeout : 30000,
                discoveryRetryDelay : 1000,
                arbitrationStrategy : ArbitrationStrategyCollection.HighestPriority,
                cacheMaxAge : 0,
                discoveryScope : DiscoveryScope.LOCAL_THEN_GLOBAL,
                additionalParameters : {}
            });
            expect(defaultDiscoveryQos).toBeDefined();
            expect(defaultDiscoveryQos).not.toBeNull();
            expect(typeof defaultDiscoveryQos === "object").toBeTruthy();
            expect(defaultDiscoveryQos instanceof DiscoveryQos).toEqual(true);
        });

        it("constructs correct default object", function() {
            expect(new DiscoveryQos()).toEqual(new DiscoveryQos({
                discoveryTimeout : 30000,
                discoveryRetryDelay : 1000,
                arbitrationStrategy : ArbitrationStrategyCollection.HighestPriority,
                cacheMaxAge : 0,
                discoveryScope : DiscoveryScope.LOCAL_THEN_GLOBAL,
                additionalParameters : {}
            }));
        });

        it("constructs with correct member values", function() {
            var discoveryQos = new DiscoveryQos({
                discoveryTimeout : 12345,
                discoveryRetryDelay : 123456,
                arbitrationStrategy : ArbitrationStrategyCollection.HighestPriority,
                cacheMaxAge : 1234,
                discoveryScope : DiscoveryScope.LOCAL_AND_GLOBAL,
                additionalParameters : {
                    testKey : "testValue"
                }
            });
            expect(discoveryQos.discoveryTimeout).toEqual(12345);
            expect(discoveryQos.discoveryRetryDelay).toEqual(123456);
            expect(discoveryQos.arbitrationStrategy).toEqual(
                    ArbitrationStrategyCollection.HighestPriority);
            expect(discoveryQos.cacheMaxAge).toEqual(1234);
            expect(discoveryQos.discoveryScope).toEqual(DiscoveryScope.LOCAL_AND_GLOBAL);
            expect(discoveryQos.additionalParameters).toEqual({
                testKey : "testValue"
            });
        });
    });

}); // require
