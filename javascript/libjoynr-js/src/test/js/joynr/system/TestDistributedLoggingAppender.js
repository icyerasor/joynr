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

joynrTestRequire("joynr/system/TestDistributedLoggingAppender", [
    "joynr/system/LoggingManager",
    "joynr/system/DistributedLoggingAppender"
], function(LoggingManager, DistributedLoggingAppender) {
    var loggingProxy;

    loggingProxy = {
        log : function(value) {}
    };

    describe("libjoynr-js.joynr.system.DistributedLoggingAppender", function() {

        beforeEach(function() {
            jasmine.Clock.useMock();
            jasmine.Clock.reset();

        });

        it("is instantiable", function() {
            expect(new DistributedLoggingAppender()).toBeDefined();
        });

        it("uses the correct interval", function() {
            var config, context, appender, loggingEvent;

            context = {};
            config = {
                flushInterval : 1000,
                flushMaxLogEventsCount : 3
            };

            spyOn(loggingProxy, "log");

            appender = new DistributedLoggingAppender(config, context);
            appender.setProxy(loggingProxy);

            expect(loggingProxy.log).not.toHaveBeenCalled();

            loggingEvent = {
                logger : {
                    name : "loggerName"
                },
                timeStampInMilliseconds : 10000000,
                messages : [ "log message"
                ],
                level : {
                    name : "DEBUG"
                }
            };
            appender.append(loggingEvent);

            expect(loggingProxy.log).not.toHaveBeenCalled();

            jasmine.Clock.tick(1000);

            expect(loggingProxy.log).toHaveBeenCalled();

        });

        it("flushes when the max number of events has been reached", function() {
            var config, context, appender, loggingEvent;

            context = {};
            config = {
                flushInterval : 1000,
                flushMaxLogEventsCount : 3

            };

            spyOn(loggingProxy, "log");

            appender = new DistributedLoggingAppender(config, context);
            appender.setProxy(loggingProxy);

            expect(loggingProxy.log).not.toHaveBeenCalled();
            loggingEvent = {
                logger : {
                    name : "loggerName"
                },
                timeStampInMilliseconds : 10000000,
                messages : [ "log message"
                ],
                level : {
                    name : "DEBUG"
                }
            };
            appender.append(loggingEvent);
            expect(loggingProxy.log).not.toHaveBeenCalled();

            appender.append(loggingEvent);
            expect(loggingProxy.log).not.toHaveBeenCalled();

            appender.append(loggingEvent);
            expect(loggingProxy.log).toHaveBeenCalled();
        });

    });
});