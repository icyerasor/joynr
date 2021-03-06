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
        "joynr/messaging/routing/TestMessageQueue",
        [
            "joynr/util/UtilInternal",
            "joynr/messaging/routing/MessageQueue",
            "joynr/messaging/JoynrMessage",
            "Date"
        ],
        function(Util, MessageQueue, JoynrMessage, Date) {

            var fakeTime;

            function increaseFakeTime(time_ms) {
                fakeTime = fakeTime + time_ms;
                jasmine.Clock.tick(time_ms);
            }

            describe(
                    "libjoynr-js.joynr.messaging.routing.MessageQueue",
                    function() {
                        var messageQueue, joynrMessage, joynrMessage2, receiverParticipantId;
                        receiverParticipantId = "TestMessageQueue_participantId_" + Date.now();
                        joynrMessage = new JoynrMessage(JoynrMessage.JOYNRMESSAGE_TYPE_REQUEST);
                        joynrMessage.to = receiverParticipantId;
                        joynrMessage.from = "senderParticipantId";
                        joynrMessage.payload = "hello";
                        joynrMessage2 = new JoynrMessage(JoynrMessage.JOYNRMESSAGE_TYPE_REQUEST);
                        joynrMessage2.to = receiverParticipantId;
                        joynrMessage2.from = "senderParticipantId2";
                        joynrMessage2.payload = "hello2";

                        messageQueue = new MessageQueue({
                            maxQueueSizeInKBytes : 0.5
                        // set the qsize to 500 bytes for testing purposes
                        });

                        beforeEach(function() {
                            fakeTime = Date.now();
                            jasmine.Clock.useMock();
                            jasmine.Clock.reset();
                            spyOn(Date, "now").andCallFake(function() {
                                return fakeTime;
                            });
                            messageQueue.reset();
                        });

                        it(
                                "test message queue limit",
                                function() {
                                    var newJoynrMessage, i = 0, payload = "hello", oldQueueSize, maxIterations =
                                            Math
                                                    .floor((messageQueue.maxQueueSizeInKBytes * 1024 / Util
                                                            .getLengthInBytes(payload)));
                                    newJoynrMessage =
                                            new JoynrMessage(JoynrMessage.JOYNRMESSAGE_TYPE_REQUEST);
                                    newJoynrMessage.expiryDate = Date.now() + 1000;
                                    newJoynrMessage.payload = payload;

                                    while (i < maxIterations) {
                                        newJoynrMessage.to = receiverParticipantId + "i";
                                        newJoynrMessage.from = "senderParticipantId" + "i";
                                        oldQueueSize = messageQueue.currentQueueSize;
                                        messageQueue.putMessage(joynrMessage);
                                        i++;
                                        //until now, all messages shall be in the queue
                                        expect(messageQueue.currentQueueSize).toEqual(
                                                oldQueueSize + Util.getLengthInBytes(payload));
                                    }
                                    //now, the next message shall lead to a queue overflow
                                    newJoynrMessage.to =
                                            receiverParticipantId + "ExceedsQueueBuffer";
                                    newJoynrMessage.from =
                                            "senderParticipantId" + "ExceedsQueueBuffer";
                                    oldQueueSize = messageQueue.currentQueueSize;
                                    i = 0;
                                    while (i < 10) {
                                        messageQueue.putMessage(joynrMessage);
                                        expect(messageQueue.currentQueueSize).toEqual(oldQueueSize);
                                        i++;
                                    }
                                });

                        it(
                                "put Message adds new queued message, dropped after getAndRemoveMessage call",
                                function() {
                                    var queuedMessages;
                                    joynrMessage.expiryDate = Date.now() + 1000;
                                    messageQueue.putMessage(joynrMessage);

                                    queuedMessages =
                                            messageQueue
                                                    .getAndRemoveMessages(receiverParticipantId);

                                    expect(queuedMessages.length).toEqual(1);
                                    expect(queuedMessages[0]).toEqual(joynrMessage);

                                    expect(
                                            messageQueue
                                                    .getAndRemoveMessages(receiverParticipantId).length)
                                            .toEqual(0);
                                });

                        it(
                                "put Message adds multiple queued messages, dropped after getAndRemoveMessage call",
                                function() {
                                    var queuedMessages;
                                    joynrMessage.expiryDate = Date.now() + 1000;
                                    joynrMessage2.expiryDate = Date.now() + 1000;
                                    messageQueue.putMessage(joynrMessage);
                                    messageQueue.putMessage(joynrMessage2);

                                    queuedMessages =
                                            messageQueue
                                                    .getAndRemoveMessages(receiverParticipantId);

                                    expect(queuedMessages.length).toEqual(2);
                                    expect(queuedMessages[0]).toEqual(joynrMessage);
                                    expect(queuedMessages[1]).toEqual(joynrMessage2);

                                    expect(
                                            messageQueue
                                                    .getAndRemoveMessages(receiverParticipantId).length)
                                            .toEqual(0);
                                });

                        it(
                                "put Message adds new queued message, dropped after timeout",
                                function() {
                                    var queuedMessages;
                                    joynrMessage.expiryDate = Date.now() + 1000;
                                    messageQueue.putMessage(joynrMessage);

                                    increaseFakeTime(1000 + 1);

                                    queuedMessages =
                                            messageQueue
                                                    .getAndRemoveMessages(receiverParticipantId);

                                    expect(queuedMessages.length).toEqual(0);
                                });

                        it(
                                "put Message adds multiple queued messages, dropped first one after timeout",
                                function() {
                                    var queuedMessages;
                                    joynrMessage.expiryDate = Date.now() + 1000;
                                    joynrMessage2.expiryDate = Date.now() + 2000;
                                    messageQueue.putMessage(joynrMessage);
                                    messageQueue.putMessage(joynrMessage2);

                                    increaseFakeTime(1000 + 1);

                                    queuedMessages =
                                            messageQueue
                                                    .getAndRemoveMessages(receiverParticipantId);

                                    expect(queuedMessages.length).toEqual(1);
                                    expect(queuedMessages[0]).toEqual(joynrMessage2);
                                    expect(
                                            messageQueue
                                                    .getAndRemoveMessages(receiverParticipantId).length)
                                            .toEqual(0);
                                });

                    });
        });