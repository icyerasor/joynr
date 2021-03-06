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

//TODO: some of this relies on the dummy implementation, change accordingly when implementating
joynrTestRequire(
        "joynr/capabilities/TestParticipantIdStorage",
        [ "joynr/capabilities/ParticipantIdStorage"
        ],
        function(ParticipantIdStorage) {

            describe("libjoynr-js.joynr.capabilities.ParticipantIdStorage", function() {
                var participantIdStorage, localStorageSpy, uuidSpy, authToken;
                var domain, provider, interfaceName, key, uuid, storedParticipantId;
                var generatedParticipantId;

                beforeEach(function() {
                    uuid = "uuid";
                    interfaceName = "interfaceName";
                    provider = {
                        interfaceName : interfaceName
                    };
                    localStorageSpy = jasmine.createSpyObj("localStorageSpy", [
                        "getItem",
                        "setItem"
                    ]);
                    uuidSpy = jasmine.createSpy("uuid");
                    uuidSpy.andReturn(uuid);
                    participantIdStorage = new ParticipantIdStorage(localStorageSpy, uuidSpy);
                    key =
                            "joynr.participant."
                                + domain
                                + "."
                                + provider.interfaceName
                                + "."
                                + authToken;
                    generatedParticipantId = domain + "." + interfaceName + "." + uuid;
                    storedParticipantId = "storedParticipantId";
                });

                it("is instantiable", function() {
                    expect(participantIdStorage).toBeDefined();
                    expect(participantIdStorage instanceof ParticipantIdStorage).toBeTruthy();
                });

                it("is has all members", function() {
                    expect(participantIdStorage.getParticipantId).toBeDefined();
                    expect(typeof participantIdStorage.getParticipantId === "function")
                            .toBeTruthy();
                });

                it(
                        "uses the stored participantId if available",
                        function() {
                            localStorageSpy.getItem.andReturn(storedParticipantId);
                            var result =
                                    participantIdStorage.getParticipantId(
                                            authToken,
                                            domain,
                                            provider);
                            expect(localStorageSpy.getItem).toHaveBeenCalled();
                            expect(localStorageSpy.getItem).toHaveBeenCalledWith(key);
                            expect(uuidSpy).not.toHaveBeenCalled();
                            expect(localStorageSpy.setItem).not.toHaveBeenCalled();
                            expect(result).toEqual(storedParticipantId);
                        });

                it(
                        "generates a new uuid if no participantId is stored",
                        function() {
                            var result =
                                    participantIdStorage.getParticipantId(
                                            authToken,
                                            domain,
                                            provider);
                            expect(localStorageSpy.getItem).toHaveBeenCalled();
                            expect(localStorageSpy.getItem).toHaveBeenCalledWith(key);
                            expect(uuidSpy).toHaveBeenCalled();
                            expect(uuidSpy).toHaveBeenCalledWith();
                            expect(localStorageSpy.setItem).toHaveBeenCalled();
                            expect(localStorageSpy.setItem).toHaveBeenCalledWith(
                                    key,
                                    generatedParticipantId);
                            expect(result).toEqual(generatedParticipantId);
                        });

            });

        }); // require
