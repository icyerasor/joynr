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

joynrTestRequire(
        "joynr/messaging/inprocess/TestInProcessMessagingStubFactory",
        [ "joynr/messaging/inprocess/InProcessMessagingStubFactory"
        ],
        function(InProcessMessagingStubFactory) {

            describe(
                    "libjoynr-js.joynr.messaging.inprocess.InProcessMessagingStubFactory",
                    function() {
                        var skeletonCallReturn, inProcessMessagingSkeleton, inProcessAddress;
                        var inProcessMessagingStubFactory, joynrMessage;

                        beforeEach(function() {
                            skeletonCallReturn = {
                                key : "skeletonCallReturn"
                            };
                            inProcessMessagingSkeleton =
                                    jasmine.createSpyObj(
                                            "inProcessMessagingSkeleton",
                                            [ "receiveMessage"
                                            ]);
                            inProcessMessagingSkeleton.receiveMessage.andReturn(skeletonCallReturn);
                            inProcessAddress =
                                    jasmine.createSpyObj("inProcessAddress", [ "getSkeleton"
                                    ]);
                            inProcessAddress.getSkeleton.andReturn(inProcessMessagingSkeleton);
                            inProcessMessagingStubFactory = new InProcessMessagingStubFactory({});
                            joynrMessage = {
                                key : "joynrMessage"
                            };
                        });

                        it(
                                "is instantiable and of correct type",
                                function() {
                                    expect(InProcessMessagingStubFactory).toBeDefined();
                                    expect(typeof InProcessMessagingStubFactory === "function")
                                            .toBeTruthy();
                                    expect(inProcessMessagingStubFactory).toBeDefined();
                                    expect(
                                            inProcessMessagingStubFactory instanceof InProcessMessagingStubFactory)
                                            .toBeTruthy();
                                    expect(inProcessMessagingStubFactory.build).toBeDefined();
                                    expect(
                                            typeof inProcessMessagingStubFactory.build === "function")
                                            .toBeTruthy();
                                });

                        it("creates a messaging stub and uses it correctly", function() {
                            var inProcessMessagingStub =
                                    inProcessMessagingStubFactory.build(inProcessAddress);
                            expect(inProcessAddress.getSkeleton).toHaveBeenCalledWith();

                            var result = inProcessMessagingStub.transmit(joynrMessage);
                            expect(inProcessMessagingSkeleton.receiveMessage).toHaveBeenCalledWith(
                                    joynrMessage);
                            expect(result).toEqual(skeletonCallReturn);
                        });

                    });

        });