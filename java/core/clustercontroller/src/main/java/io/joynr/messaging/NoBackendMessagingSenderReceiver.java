package io.joynr.messaging;

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

import java.util.concurrent.Future;

import com.google.common.util.concurrent.Futures;
import com.google.inject.Singleton;

import joynr.JoynrMessage;

/**
 * A Dummy implementation that does not perform any backend communication
 *
 */

@Singleton
public class NoBackendMessagingSenderReceiver implements MessageReceiver, MessageSender {

    @Override
    public String getChannelId() {
        return "null";
    }

    @Override
    public String getReplyToChannelId() {
        return "null";
    }

    @Override
    public void sendMessage(String mcid, JoynrMessage message) {
        throw new UnsupportedOperationException("The NoBackendMessagingSenderReceiver cannot send messages");
    }

    public void receiveMessage(JoynrMessage message) {
        throw new UnsupportedOperationException("The NoBackendMessagingSenderReceiver cannot receive messages");
    }

    @Override
    public void shutdown(boolean clear) {

    }

    @Override
    public boolean deleteChannel() {
        return true;
    }

    @Override
    public boolean isStarted() {
        return true;
    }

    @Override
    public void suspend() {
    }

    @Override
    public void resume() {
    }

    @Override
    public boolean isChannelCreated() {
        return true;
    }

    @Override
    public void shutdown() {
    }

    @Override
    public Future<Void> start(MessageArrivedListener messageArrivedListener, ReceiverStatusListener... statusListeners) {
        return Futures.immediateFuture(null);
    }
}