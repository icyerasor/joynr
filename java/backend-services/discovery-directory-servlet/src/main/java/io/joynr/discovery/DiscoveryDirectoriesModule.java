package io.joynr.discovery;

/*
 * #%L
 * %%
 * Copyright (C) 2011 - 2014 BMW Car IT GmbH
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

import io.joynr.capabilities.directory.CapabilitiesDirectoryModule;
import io.joynr.channel.ChannelUrlDirectoryModule;
import io.joynr.messaging.ChannelUrlStore;
import io.joynr.messaging.ConfigurableMessagingSettings;
import io.joynr.messaging.MessagingPropertyKeys;
import io.joynr.runtime.AbstractJoynrApplication;

import java.util.HashMap;

import joynr.types.ChannelUrlInformation;

import com.google.common.collect.Maps;
import com.google.inject.AbstractModule;
import com.google.inject.Module;
import com.google.inject.Provides;
import com.google.inject.name.Named;
import com.google.inject.persist.jpa.JpaPersistModule;
import com.google.inject.util.Modules;

public class DiscoveryDirectoriesModule extends AbstractModule {

    private static final HashMap<String, ChannelUrlInformation> emptyChannelUrlMap = Maps.newHashMap();

    @Override
    protected void configure() {
        install(new JpaPersistModule("Discovery"));
        AbstractModule discoveryDirectoryModule = new AbstractModule() {

            @Override
            protected void configure() {
            }

            @Provides
            @Named(AbstractJoynrApplication.PROPERTY_JOYNR_DOMAIN_LOCAL)
            String provideCapabilitiesDirectoryDomain(@Named(ConfigurableMessagingSettings.PROPERTY_DISCOVERY_DIRECTORIES_DOMAIN) String discoveryDirectoryDomain) {
                return discoveryDirectoryDomain;
            }

            @Provides
            @Named(MessagingPropertyKeys.CHANNELID)
            String provideCapabilitiesDirectoryChannelId(@Named(ConfigurableMessagingSettings.PROPERTY_CAPABILITIES_DIRECTORY_CHANNEL_ID) String discoveryDirectoriesChannelId) {
                return discoveryDirectoriesChannelId;
            }

            // An empty store.
            // The ChannelUrlDirectory's joynlib should not use a cache, but rather ask the real ChannelUrlDirectory Provider when trying to resolve channelUrls
            @Provides
            ChannelUrlStore provideChannelUrlStore() {
                return new ChannelUrlStore() {

                    @Override
                    public void registerChannelUrls(String channelId, ChannelUrlInformation channelUrlInformation) {
                    }

                    @Override
                    public void removeChannelUrls(String channelId) {
                    }

                    @Override
                    public ChannelUrlInformation findChannelEntry(String channelId) {
                        return new ChannelUrlInformation();
                    }

                    @Override
                    public HashMap<String, ChannelUrlInformation> getAllChannelUrls() {
                        return emptyChannelUrlMap;
                    }

                    @Override
                    public void registerChannelUrl(String channelId, String channelUrl) {
                    }
                };
            }
        };
        Module combinedDirectoriesModule = Modules.override(new ChannelUrlDirectoryModule())
                                                  .with(new CapabilitiesDirectoryModule());
        install(Modules.override(combinedDirectoriesModule).with(discoveryDirectoryModule));
    }

}
