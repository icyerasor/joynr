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
        "joynr/messaging/channel/TestChannelMessagingSkeletion",
        [ "joynr/messaging/channel/ChannelMessagingSkeleton"
        ],
        function(ChannelMessagingSkeleton) {

            describe("libjoynr-js.joynr.messaging.channel.ChannelMessagingSkeleton", function() {

                var channelMessagingSkeleton, joynrMessage1, joynrMessage2, messageRouterSpy;

                beforeEach(function() {
                    messageRouterSpy = jasmine.createSpyObj("messageRouterSpy", [
                        "addNextHop",
                        "route"
                    ]);
                    channelMessagingSkeleton = new ChannelMessagingSkeleton({
                        messageRouter : messageRouterSpy
                    });

                    joynrMessage1 = {
                        key : "joynrMessage2"
                    };
                    joynrMessage2 = {
                        key : "joynrMessage1",
                        replyChannelId : "channelId"
                    };
                });

                it("is of correct type and has all members", function() {
                    expect(ChannelMessagingSkeleton).toBeDefined();
                    expect(typeof ChannelMessagingSkeleton === "function").toBeTruthy();
                    expect(channelMessagingSkeleton).toBeDefined();
                    expect(channelMessagingSkeleton instanceof ChannelMessagingSkeleton)
                            .toBeTruthy();
                });

                it("throws if arguments are missing or of wrong type", function() {
                    expect(function() {
                        channelMessagingSkeleton = new ChannelMessagingSkeleton();
                    }).toThrow(); // correct call
                    expect(function() {
                        channelMessagingSkeleton = new ChannelMessagingSkeleton({
                            messageRouter : messageRouterSpy
                        });
                    }).not.toThrow(); // correct call
                });

                it("event calls through to messageRouter", function() {
                    expect(messageRouterSpy.route).not.toHaveBeenCalled();
                    channelMessagingSkeleton.receiveMessage(joynrMessage1);
                    expect(messageRouterSpy.route).toHaveBeenCalledWith(joynrMessage1);
                    expect(messageRouterSpy.addNextHop).not.toHaveBeenCalled();
                    expect(messageRouterSpy.route.calls.length).toBe(1);
                    channelMessagingSkeleton.receiveMessage(joynrMessage2);
                    expect(messageRouterSpy.route).toHaveBeenCalledWith(joynrMessage2);
                    expect(messageRouterSpy.addNextHop).toHaveBeenCalled();
                    expect(messageRouterSpy.addNextHop.mostRecentCall.args[0]).toBe(
                            joynrMessage2.from);
                    expect(messageRouterSpy.addNextHop.mostRecentCall.args[1].channelId).toBe(
                            joynrMessage2.replyChannelId);
                    expect(messageRouterSpy.route.calls.length).toBe(2);
                });

            });

        });