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

define(
        "joynr/messaging/webmessaging/WebMessagingStub",
        [
            "global/Promise",
            "joynr/util/UtilInternal",
            "joynr/system/LoggerFactory"
        ],
        function(Promise, Util, LoggerFactory) {

            /**
             * @name WebMessagingStub
             * @constructor

             * @param {Object} settings the settings object for this constructor call
             * @param {Object} settings.window the default target window, the messages should be sent to
             * @param {String} settings.origin the default origin, the messages should be sent to
             */
            function WebMessagingStub(settings) {
                var log = LoggerFactory.getLogger("joynr/messaging/webmessaging/WebMessagingStub");

                Util.checkProperty(settings, "Object", "settings");

                if (settings.window === undefined) {
                    throw new Error("WebMessagingStub constructor parameter windows is undefined");
                }

                Util.checkProperty(settings.origin, "String", "settings.origin");

                if (settings.window.postMessage === undefined) {
                    throw new Error(
                            "WebMessagingStub constructor parameter window does not provide the expected functions \"postMessage\"");
                }

                /**
                 * @name WebMessagingStub#transmit
                 * @function
                 *
                 * @param {Object|JoynrMessage} message the message to transmit
                 */
                this.transmit =
                        function transmit(message) {
                            Util.checkProperty(message.message, [ "JoynrMessage"
                            ], "message.joynrMessage");
                            Util.checkPropertyIfDefined(
                                    message.windowId,
                                    "String",
                                    "message.windowId");

                            //TODO: check why sending a JoynrMessage provoces the following error
                            // maybe enumerability or visibility of members while using Object.defineProperties
                            /*
                             DataCloneError: An object could not be cloned.
                             code: 25
                             message: "An object could not be cloned."
                             name: "DataCloneError"
                             stack: "Error: An object could not be cloned.
                             __proto__: DOMException
                             */
                            log.debug("transmit message: \"" + JSON.stringify(message) + "\"");
                            settings.window.postMessage(
                                    JSON.parse(JSON.stringify(message)),
                                    settings.origin);

                            return Promise.resolve();
                        };

            }

            return WebMessagingStub;

        });