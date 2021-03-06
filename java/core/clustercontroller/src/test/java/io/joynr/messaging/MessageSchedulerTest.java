package io.joynr.messaging;

/*
 * #%L
 * %%
 * Copyright (C) 2011 - 2016 BMW Car IT GmbH
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

import static org.junit.Assert.fail;
import static org.mockito.Mockito.*;
import java.util.UUID;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.ScheduledThreadPoolExecutor;
import java.util.concurrent.ThreadFactory;
import java.util.concurrent.TimeUnit;

import javax.inject.Named;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.runners.MockitoJUnitRunner;

import com.google.common.util.concurrent.ThreadFactoryBuilder;
import com.google.inject.AbstractModule;
import com.google.inject.Guice;
import com.google.inject.Injector;
import com.google.inject.Provides;
import com.google.inject.TypeLiteral;
import com.google.inject.multibindings.MapBinder;
import com.google.inject.name.Names;

import io.joynr.common.ExpiryDate;
import io.joynr.exceptions.JoynrMessageNotSentException;
import io.joynr.messaging.channel.ChannelMessageSerializerFactory;
import io.joynr.messaging.channel.ChannelMessagingStubFactory;
import io.joynr.messaging.http.HttpMessageSender;
import io.joynr.messaging.routing.MessagingStubFactory;
import io.joynr.messaging.serialize.AbstractMiddlewareMessageSerializerFactory;
import io.joynr.messaging.serialize.JsonSerializer;
import io.joynr.messaging.serialize.MessageSerializerFactory;
import joynr.JoynrMessage;
import joynr.system.RoutingTypes.Address;
import joynr.system.RoutingTypes.ChannelAddress;

@RunWith(MockitoJUnitRunner.class)
public class MessageSchedulerTest {

    private String channelId = "MessageSchedulerTest_" + UUID.randomUUID().toString();
    private final ChannelAddress channelAddress = new ChannelAddress(channelId);

    private MessageScheduler messageScheduler;

    @Mock
    private JsonSerializer jsonSerializer;

    @Mock
    private HttpMessageSender httpMessageSenderMock;

    @Before
    public void setUp() throws Exception {

        AbstractModule mockModule = new AbstractModule() {

            private Long msgRetryIntervalMs = 10L;
            private int maximumParallelSends = 1;

            @Override
            protected void configure() {
                bind(MessageScheduler.class).to(MessageSchedulerImpl.class);
                bind(JsonSerializer.class).toInstance(jsonSerializer);
                bind(String.class).annotatedWith(Names.named(MessagingPropertyKeys.CHANNELID)).toInstance(channelId);
                bind(Long.class).annotatedWith(Names.named(ConfigurableMessagingSettings.PROPERTY_SEND_MSG_RETRY_INTERVAL_MS))
                                .toInstance(msgRetryIntervalMs);
                bind(HttpMessageSender.class).toInstance(httpMessageSenderMock);

                MapBinder<Class<? extends Address>, AbstractMiddlewareMessagingStubFactory<? extends IMessaging, ? extends Address>> messagingStubFactory;
                messagingStubFactory = MapBinder.newMapBinder(binder(), new TypeLiteral<Class<? extends Address>>() {
                }, new TypeLiteral<AbstractMiddlewareMessagingStubFactory<? extends IMessaging, ? extends Address>>() {
                }, Names.named(MessagingStubFactory.MIDDLEWARE_MESSAGING_STUB_FACTORIES));
                messagingStubFactory.addBinding(ChannelAddress.class).to(ChannelMessagingStubFactory.class);

                MapBinder<Class<? extends Address>, AbstractMiddlewareMessageSerializerFactory<? extends Address>> messageSerializerFactory;
                messageSerializerFactory = MapBinder.newMapBinder(binder(),
                                                                  new TypeLiteral<Class<? extends Address>>() {
                                                                  },
                                                                  new TypeLiteral<AbstractMiddlewareMessageSerializerFactory<? extends Address>>() {
                                                                  },
                                                                  Names.named(MessageSerializerFactory.MIDDLEWARE_MESSAGE_SERIALIZER_FACTORIES));
                messageSerializerFactory.addBinding(ChannelAddress.class).to(ChannelMessageSerializerFactory.class);
            }

            @Provides
            @Named(MessageScheduler.SCHEDULEDTHREADPOOL)
            ScheduledExecutorService provideMessageSchedulerThreadPoolExecutor() {
                ThreadFactory schedulerNamedThreadFactory = new ThreadFactoryBuilder().setNameFormat("joynr.MessageScheduler-scheduler-%d")
                                                                                      .build();
                ScheduledThreadPoolExecutor scheduler = new ScheduledThreadPoolExecutor(maximumParallelSends,
                                                                                        schedulerNamedThreadFactory);
                scheduler.setKeepAliveTime(100, TimeUnit.SECONDS);
                scheduler.allowCoreThreadTimeOut(true);
                return scheduler;
            }
        };

        Injector injector = Guice.createInjector(mockModule);
        messageScheduler = injector.getInstance(MessageScheduler.class);
    }

    @Test
    public void testScheduleMessageOk() {
        JoynrMessage joynrMessage = new JoynrMessage();
        joynrMessage.setExpirationDate(ExpiryDate.fromRelativeTtl(10000));
        messageScheduler.scheduleMessage(channelAddress, joynrMessage);
        verify(jsonSerializer).serialize(eq(joynrMessage));
        verify(httpMessageSenderMock).sendMessage(eq(channelAddress), any(String.class), any(FailureAction.class));
    }

    @Test
    public void testScheduleExpiredMessageFails() throws InterruptedException {
        JoynrMessage joynrMessage = new JoynrMessage();
        joynrMessage.setExpirationDate(ExpiryDate.fromRelativeTtl(1));
        Thread.sleep(5);
        try {
            messageScheduler.scheduleMessage(channelAddress, joynrMessage);
        } catch (JoynrMessageNotSentException e) {
            verify(jsonSerializer, never()).serialize(any(JoynrMessage.class));
            verify(httpMessageSenderMock, never()).sendMessage(any(ChannelAddress.class),
                                                               any(String.class),
                                                               any(FailureAction.class));
            return;
        }
        fail("scheduling an expired message should throw");

    }
}
