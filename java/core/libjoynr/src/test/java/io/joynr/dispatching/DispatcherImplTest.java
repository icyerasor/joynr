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

import static io.joynr.runtime.JoynrInjectionConstants.JOYNR_SCHEDULER_CLEANUP;
import static org.mockito.Mockito.mock;
import io.joynr.dispatching.rpc.RpcUtils;
import io.joynr.dispatching.subscription.PublicationManager;
import io.joynr.dispatching.subscription.SubscriptionManager;
import io.joynr.messaging.MessageReceiver;
import io.joynr.messaging.ReceiverStatusListener;
import io.joynr.messaging.routing.MessageRouter;
import io.joynr.proxy.JoynrMessagingConnectorFactory;

import java.util.UUID;
import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.ThreadFactory;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

import joynr.Request;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.Spy;
import org.mockito.runners.MockitoJUnitRunner;

import com.google.common.util.concurrent.ThreadFactoryBuilder;
import com.google.inject.AbstractModule;
import com.google.inject.Guice;
import com.google.inject.Injector;
import com.google.inject.name.Names;

@RunWith(MockitoJUnitRunner.class)
public class DispatcherImplTest {

    @Mock
    private RequestReplyManager requestReplyManagerMock;
    @Mock
    private SubscriptionManager subscriptionManagerMock;
    @Mock
    private PublicationManager publicationManagerMock;
    @Mock
    private MessageRouter messageRouterMock;
    @Spy
    private MessageReceiverMock messageReceiverMock = new MessageReceiverMock();

    private Dispatcher fixture;
    private RequestCallerDirectory requestCallerDirectory;

    @Before
    public void setUp() throws NoSuchMethodException, SecurityException {
        Injector injector = Guice.createInjector(new AbstractModule() {

            @Override
            protected void configure() {
                bind(Dispatcher.class).to(DispatcherImpl.class);
                bind(RequestReplyManager.class).toInstance(requestReplyManagerMock);
                bind(SubscriptionManager.class).toInstance(subscriptionManagerMock);
                bind(PublicationManager.class).toInstance(publicationManagerMock);
                bind(MessageRouter.class).toInstance(messageRouterMock);
                bind(MessageReceiver.class).toInstance(messageReceiverMock);

                requestStaticInjection(RpcUtils.class, Request.class, JoynrMessagingConnectorFactory.class);

                ThreadFactory namedThreadFactory = new ThreadFactoryBuilder().setNameFormat("joynr.Cleanup-%d").build();
                ScheduledExecutorService cleanupExecutor = Executors.newSingleThreadScheduledExecutor(namedThreadFactory);
                bind(ScheduledExecutorService.class).annotatedWith(Names.named(JOYNR_SCHEDULER_CLEANUP))
                                                    .toInstance(cleanupExecutor);
            }

        });
        fixture = injector.getInstance(Dispatcher.class);
        messageReceiverMock.start(fixture, new ReceiverStatusListener() {
            @Override
            public void receiverStarted() {
            }

            @Override
            public void receiverException(Throwable e) {
            }
        });
        requestCallerDirectory = injector.getInstance(RequestCallerDirectory.class);
    }

    @Test
    public void addRequestCallerDoesNotBlock() throws InterruptedException, ExecutionException, TimeoutException {

        final Callable<Boolean> stuffToDo = new Callable<Boolean>() {

            @Override
            public Boolean call() throws Exception {

                try {
                    String requestReplyId = UUID.randomUUID().toString();
                    RequestCaller requestCaller = mock(RequestCaller.class);
                    /* setBlockInitialisation to true causes the messageReceiver to block
                     * during startup
                     * The MessageReceiver is invoked by the dispatcher once a request caller
                     * is registered
                     *
                     */
                    messageReceiverMock.setBlockOnInitialisation(true);
                    requestCallerDirectory.addCaller(requestReplyId, requestCaller);
                } finally {
                    messageReceiverMock.setBlockOnInitialisation(false);
                }
                return true;
            }
        };

        final ExecutorService executor = Executors.newSingleThreadExecutor();
        final Future<Boolean> future = executor.submit(stuffToDo);

        // should not throw a timeout exception
        future.get(1000, TimeUnit.MILLISECONDS);
        Mockito.verify(messageReceiverMock).start(Mockito.eq(fixture), Mockito.any(ReceiverStatusListener.class));

    }
}
