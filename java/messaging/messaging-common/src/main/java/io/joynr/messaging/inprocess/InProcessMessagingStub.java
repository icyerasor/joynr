package io.joynr.messaging.inprocess;

import io.joynr.messaging.FailureAction;

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

import io.joynr.messaging.IMessaging;

import javax.inject.Inject;

import joynr.JoynrMessage;

public class InProcessMessagingStub implements IMessaging {

    private final InProcessMessagingSkeleton skeleton;

    @Inject
    public InProcessMessagingStub(InProcessMessagingSkeleton skeleton) {
        this.skeleton = skeleton;
    }

    @Override
    public void transmit(JoynrMessage message, FailureAction failureAction) {
        skeleton.transmit(message, failureAction);
    }

    @Override
    public void transmit(String serializedMessage, FailureAction failureAction) {
        throw new IllegalStateException("InProcess messaging should not send serialized messages");
    }

}
