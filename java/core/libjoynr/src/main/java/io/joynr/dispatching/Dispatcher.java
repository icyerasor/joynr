package io.joynr.dispatching;

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

import io.joynr.exceptions.JoynrMessageNotSentException;
import io.joynr.exceptions.JoynrSendBufferFullException;
import io.joynr.messaging.MessageArrivedListener;
import io.joynr.messaging.MessagingQos;

import java.io.IOException;

import joynr.SubscriptionPublication;
import joynr.SubscriptionRequest;
import joynr.SubscriptionStop;

import com.fasterxml.jackson.core.JsonGenerationException;
import com.fasterxml.jackson.databind.JsonMappingException;

public interface Dispatcher extends MessageArrivedListener {
    public void sendSubscriptionRequest(String fromParticipantId,
                                        String toParticipantId,
                                        SubscriptionRequest subscriptionRequest,
                                        MessagingQos qosSettings,
                                        boolean broadcast) throws JoynrSendBufferFullException,
                                                          JoynrMessageNotSentException, JsonGenerationException,
                                                          JsonMappingException, IOException;

    public void sendSubscriptionStop(String fromParticipantId,
                                     String toParticipantId,
                                     SubscriptionStop subscriptionStop,
                                     MessagingQos qosSettings) throws JoynrSendBufferFullException,
                                                              JoynrMessageNotSentException, JsonGenerationException,
                                                              JsonMappingException, IOException;

    public void sendSubscriptionPublication(String fromParticipantId,
                                            String toParticipantId,
                                            SubscriptionPublication publication,
                                            MessagingQos qosSettings) throws JoynrSendBufferFullException,
                                                                     JoynrMessageNotSentException,
                                                                     JsonGenerationException, JsonMappingException,
                                                                     IOException;

    /**
     * 
     * @param clear
     *            indicates whether the channel should be closed
     */
    public void shutdown(boolean clear);
}
