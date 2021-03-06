package io.joynr.messaging;

/*
 * #%L
 * %%
 * Copyright (C) 2011 - 2013 BMW Car IT GmbH
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

import io.joynr.dispatcher.rpc.annotation.JoynrRpcCallback;
import io.joynr.dispatcher.rpc.annotation.JoynrRpcParam;
import io.joynr.proxy.Callback;
import io.joynr.proxy.Future;

import java.util.Map;

import joynr.infrastructure.ChannelUrlDirectoryProxy;
import joynr.types.ChannelUrlInformation;

public class TestChannelUrlClientImpl implements ChannelUrlDirectoryProxy {

    private final Map<String, ChannelUrlInformation> entries;

    public TestChannelUrlClientImpl(Map<String, ChannelUrlInformation> entries) {
        this.entries = entries;
    }

    @Override
    public ChannelUrlInformation getUrlsForChannel(String channelId) {
        return entries.get(channelId);
    }

    @Override
    public Future<Void> registerChannelUrls(@JoynrRpcCallback(deserializationType = Void.class) Callback<Void> callback,
                                            @JoynrRpcParam("channelId") String channelId,
                                            @JoynrRpcParam("channelUrlInformation") ChannelUrlInformation channelUrlInformation) {
        entries.put(channelId, channelUrlInformation);
        Future<Void> future = new Future<Void>();
        future.onSuccess(null);
        return future;
    }

    @Override
    public Future<Void> unregisterChannelUrls(@JoynrRpcCallback(deserializationType = Void.class) Callback<Void> callback,
                                              @JoynrRpcParam("channelId") String channelId) {
        Future<Void> future = new Future<Void>();
        future.onSuccess(null);
        return future;
    }

    @Override
    public Future<ChannelUrlInformation> getUrlsForChannel(@JoynrRpcCallback(deserializationType = ChannelUrlInformation.class) Callback<ChannelUrlInformation> callback,
                                                           @JoynrRpcParam("channelId") String channelId) {
        // TODO Auto-generated method stub
        return null;
    }

    @Override
    public void unregisterChannelUrls(@JoynrRpcParam("channelId") String channelId) {
        entries.remove(channelId);

    }

    @Override
    public void registerChannelUrls(@JoynrRpcParam("channelId") String channelId,
                                    @JoynrRpcParam("channelUrlInformation") ChannelUrlInformation channelUrlInformation) {
        // TODO Auto-generated method stub

    }

}
