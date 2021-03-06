/*global joynrTestRequire: true */
/*jslint es5: true */

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

//TODO: some of this relies on the dummy implementation, change accordingly when implementating
joynrTestRequire(
        "joynr/capabilities/TestCapabilitiesRegistrar",
        [
            "global/Promise",
            "joynr/capabilities/CapabilitiesRegistrar",
            "joynr/types/ProviderQos",
            "joynr/types/CapabilityInformation",
            "joynr/provider/ProviderAttributeNotifyReadWrite",
            "joynr/types/DiscoveryEntry",
            "joynr/types/ProviderScope",
            "uuid",
            "joynr/types/CommunicationMiddleware"
        ],
        function(
                Promise,
                CapabilitiesRegistrar,
                ProviderQos,
                CapabilityInformation,
                ProviderAttributeNotifyReadWrite,
                DiscoveryEntry,
                ProviderScope,
                uuid,
                CommunicationMiddleware) {
            describe(
                    "libjoynr-js.joynr.capabilities.CapabilitiesRegistrar",
                    function() {
                        var capabilitiesRegistrar;
                        var requestReplyManagerSpy;
                        var publicationManagerSpy;
                        var participantId;
                        var domain;
                        var authToken;
                        var participantIdStorageSpy;
                        var discoveryStubSpy;
                        var messageRouterSpy;
                        var loggingManagerSpy;
                        var libjoynrMessagingAddress;
                        var provider;
                        var capability;
                        var localChannelId;
                        var providerQos;
                        var checkImplementation;

                        beforeEach(function() {

                            // default checkImplemenation, can be overwritten by individual tests as
                            // needed
                            checkImplementation = function checkImplementationDefault() {
                                return [];
                            };

                            publicationManagerSpy = jasmine.createSpyObj("PublicationManager", [
                                "addPublicationProvider",
                                "registerOnChangedProvider"
                            ]);

                            provider = {
                                id : uuid(),
                                interfaceName : "myInterfaceName",
                                checkImplementation : checkImplementation
                            };

                            spyOn(provider, "checkImplementation").andCallThrough();

                            providerQos =
                                    new ProviderQos([], 1, Date.now(), ProviderScope.GLOBAL, true);

                            provider.myAttribute = new ProviderAttributeNotifyReadWrite(provider, {
                                dependencies : {
                                    publicationManager : publicationManagerSpy
                                }
                            }, "myAttribute", "Boolean");

                            localChannelId = "localChannelId";
                            domain = "testdomain";
                            authToken = "authToken";
                            participantId = "myParticipantId";
                            participantIdStorageSpy =
                                    jasmine.createSpyObj(
                                            "participantIdStorage",
                                            [ "getParticipantId"
                                            ]);
                            participantIdStorageSpy.getParticipantId.andReturn(participantId);
                            requestReplyManagerSpy =
                                    jasmine.createSpyObj(
                                            "RequestReplyManager",
                                            [ "addRequestCaller"
                                            ]);
                            discoveryStubSpy = jasmine.createSpyObj("discoveryStub", [ "add"
                            ]);
                            discoveryStubSpy.add.andReturn(Promise.resolve());
                            messageRouterSpy = jasmine.createSpyObj("messageRouter", [ "addNextHop"
                            ]);

                            messageRouterSpy.addNextHop.andReturn(Promise.resolve());
                            libjoynrMessagingAddress = {
                                "someKey" : "someValue",
                                "toBe" : "a",
                                "object" : {}
                            };
                            loggingManagerSpy =
                                    jasmine.createSpyObj("loggingManager", [ "setLoggingContext"
                                    ]);

                            capabilitiesRegistrar = new CapabilitiesRegistrar({
                                discoveryStub : discoveryStubSpy,
                                messageRouter : messageRouterSpy,
                                participantIdStorage : participantIdStorageSpy,
                                libjoynrMessagingAddress : libjoynrMessagingAddress,
                                requestReplyManager : requestReplyManagerSpy,
                                publicationManager : publicationManagerSpy,
                                localChannelId : localChannelId,
                                loggingManager : loggingManagerSpy
                            });

                            capability = new CapabilityInformation({
                                domain : domain,
                                interfaceName : provider.interfaceName,
                                providerQos : providerQos,
                                channelId : localChannelId,
                                participantId : participantId
                            });
                        });

                        it("is instantiable", function() {
                            expect(capabilitiesRegistrar).toBeDefined();
                            expect(capabilitiesRegistrar instanceof CapabilitiesRegistrar)
                                    .toBeTruthy();
                        });

                        it("is has all members", function() {
                            expect(capabilitiesRegistrar.registerCapability).toBeDefined();
                            expect(typeof capabilitiesRegistrar.registerCapability === "function")
                                    .toBeTruthy();
                        });

                        it("is checks the provider's implementation", function() {
                            capabilitiesRegistrar.registerCapability(
                                    authToken,
                                    domain,
                                    provider,
                                    providerQos);
                            expect(provider.checkImplementation).toHaveBeenCalled();
                        });

                        it(
                                "is checks the provider's implementation, and throws if incomplete",
                                function() {
                                    provider.checkImplementation = function() {
                                        return [ "Operation:addFavoriteStation"
                                        ];
                                    };

                                    expect(
                                            function() {
                                                capabilitiesRegistrar.registerCapability(
                                                        authToken,
                                                        domain,
                                                        provider,
                                                        providerQos);
                                            }).toThrow(
                                            new Error("provider: "
                                                + domain
                                                + "/"
                                                + provider.interfaceName
                                                + " is missing: Operation:addFavoriteStation"));

                                });

                        it("fetches participantId from the participantIdStorage", function() {
                            capabilitiesRegistrar.registerCapability(
                                    authToken,
                                    domain,
                                    provider,
                                    providerQos);
                            expect(participantIdStorageSpy.getParticipantId).toHaveBeenCalled();
                            expect(participantIdStorageSpy.getParticipantId).toHaveBeenCalledWith(
                                    authToken,
                                    domain,
                                    provider);
                        });

                        it("registers next hop with routing table", function() {
                            capabilitiesRegistrar.registerCapability(
                                    authToken,
                                    domain,
                                    provider,
                                    providerQos);
                            expect(messageRouterSpy.addNextHop).toHaveBeenCalled();
                            expect(messageRouterSpy.addNextHop).toHaveBeenCalledWith(
                                    participantId,
                                    libjoynrMessagingAddress);
                        });

                        it("registers provider at RequestReplyManager", function() {
                            capabilitiesRegistrar.registerCapability(
                                    authToken,
                                    domain,
                                    provider,
                                    providerQos);
                            expect(requestReplyManagerSpy.addRequestCaller).toHaveBeenCalled();
                            expect(requestReplyManagerSpy.addRequestCaller).toHaveBeenCalledWith(
                                    participantId,
                                    provider);
                        });

                        it(
                                "registers a provider with PublicationManager if it has an attribute",
                                function() {
                                    capabilitiesRegistrar.registerCapability(
                                            authToken,
                                            domain,
                                            provider,
                                            providerQos);
                                    expect(publicationManagerSpy.addPublicationProvider)
                                            .toHaveBeenCalled();
                                    expect(publicationManagerSpy.addPublicationProvider)
                                            .toHaveBeenCalledWith(participantId, provider);
                                });

                        it("registers capability at capabilities stub", function() {
                            capabilitiesRegistrar.registerCapability(
                                    authToken,
                                    domain,
                                    provider,
                                    providerQos);
                            expect(discoveryStubSpy.add).toHaveBeenCalled();
                            expect(discoveryStubSpy.add).toHaveBeenCalledWith(new DiscoveryEntry({
                                domain : domain,
                                interfaceName : provider.interfaceName,
                                participantId : participantId,
                                qos : providerQos,
                                connections : [ CommunicationMiddleware.JOYNR
                                ]
                            }));
                        });

                        it("registers logging context with the ContextManager", function() {
                            var loggingContext = {
                                myContext : "myContext"
                            };
                            capabilitiesRegistrar.registerCapability(
                                    authToken,
                                    domain,
                                    provider,
                                    providerQos,
                                    loggingContext);
                            expect(loggingManagerSpy.setLoggingContext).toHaveBeenCalled();
                            expect(loggingManagerSpy.setLoggingContext).toHaveBeenCalledWith(
                                    participantId,
                                    loggingContext);
                        });

                        it("returns the promise success from capabilites stub", function() {
                            var spy = jasmine.createSpyObj("spy", [
                                "onFulfilled",
                                "onRejected"
                            ]);

                            runs(function() {
                                capabilitiesRegistrar.registerCapability(
                                        authToken,
                                        domain,
                                        provider,
                                        providerQos).then(spy.onFulfilled).catch(spy.onRejected);
                            });

                            waitsFor(function() {
                                return spy.onFulfilled.callCount > 0;
                            }, "onFulfilled spy to be invoked", 100);

                            runs(function() {
                                expect(spy.onFulfilled).toHaveBeenCalled();
                                expect(spy.onFulfilled).toHaveBeenCalledWith([
                                    undefined,
                                    undefined
                                ]);
                                expect(spy.onRejected).not.toHaveBeenCalled();
                            });
                        });

                        it(
                                "returns the promise onRejected from capabilites stub",
                                function() {
                                    var spy = jasmine.createSpyObj("spy", [
                                        "onFulfilled",
                                        "onRejected"
                                    ]);
                                    discoveryStubSpy.add.andReturn(Promise.reject(new Error("Some error.")));

                                    runs(function() {
                                        capabilitiesRegistrar.registerCapability(
                                                authToken,
                                                domain,
                                                provider,
                                                providerQos).then(spy.onFulfilled).catch(spy.onRejected);
                                    });

                                    waitsFor(function() {
                                        return spy.onRejected.callCount > 0;
                                    }, "onRejected spy to be called", 100);

                                    runs(function() {
                                        expect(spy.onFulfilled).not.toHaveBeenCalled();
                                        expect(spy.onRejected).toHaveBeenCalled();
                                        expect(
                                                Object.prototype.toString
                                                        .call(spy.onRejected.mostRecentCall.args[0]) === "[object Error]")
                                                .toBeTruthy();
                                    });
                                });

                    });

        }); // require
