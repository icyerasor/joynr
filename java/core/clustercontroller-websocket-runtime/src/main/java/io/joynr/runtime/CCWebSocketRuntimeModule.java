package io.joynr.runtime;

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

import com.google.inject.Provides;
import com.google.inject.Singleton;
import com.google.inject.name.Names;
import io.joynr.messaging.ConfigurableMessagingSettings;
import io.joynr.messaging.IMessagingSkeleton;
import io.joynr.messaging.inprocess.InProcessAddress;
import io.joynr.messaging.routing.MessageRouter;
import io.joynr.messaging.routing.MessageRouterImpl;
import io.joynr.messaging.websocket.CCWebSocketMessagingSkeleton;
import io.joynr.messaging.websocket.WebSocketClientMessageSerializerFactory;
import io.joynr.messaging.websocket.WebSocketClientMessagingStubFactory;
import io.joynr.messaging.websocket.WebsocketModule;
import joynr.system.RoutingTypes.Address;
import joynr.system.RoutingTypes.WebSocketClientAddress;

import javax.inject.Named;

/**
 *
 */
public class CCWebSocketRuntimeModule extends ClusterControllerRuntimeModule {
    @Override
    protected void configure() {
        super.configure();
        install(new WebsocketModule());
        bind(ClusterControllerRuntime.class).in(Singleton.class);
        bind(JoynrRuntime.class).to(ClusterControllerRuntime.class);
        bind(IMessagingSkeleton.class).annotatedWith(Names.named(ConfigurableMessagingSettings.PROPERTY_CLUSTERCONTROLER_MESSAGING_SKELETON))
                                      .to(CCWebSocketMessagingSkeleton.class)
                                      .in(Singleton.class);
        bind(WebSocketClientMessagingStubFactory.class).in(Singleton.class);
        bind(MessageRouter.class).to(MessageRouterImpl.class).in(Singleton.class);

        messagingStubFactory.addBinding(WebSocketClientAddress.class).to(WebSocketClientMessagingStubFactory.class);
        messageSerializerFactory.addBinding(WebSocketClientAddress.class)
                                .to(WebSocketClientMessageSerializerFactory.class);

    }

    @Provides
    @Singleton
    @Named(SystemServicesSettings.PROPERTY_CC_MESSAGING_ADDRESS)
    Address getCCMessagingAddress() {
        return new InProcessAddress();
    }

}
