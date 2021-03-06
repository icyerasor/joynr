package io.joynr.integration;

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

import io.joynr.capabilities.DummyCapabilitiesDirectory;
import io.joynr.capabilities.DummyDiscoveryModule;
import io.joynr.capabilities.DummyLocalChannelUrlDirectoryClient;
import io.joynr.capabilities.LocalCapabilitiesDirectory;
import io.joynr.common.ExpiryDate;
import io.joynr.common.JoynrPropertiesModule;
import io.joynr.dispatching.JoynrMessageFactory;
import io.joynr.exceptions.JoynrMessageNotSentException;
import io.joynr.exceptions.JoynrSendBufferFullException;
import io.joynr.integration.util.TestMessageListener;
import io.joynr.messaging.MessageHandlerImpl;
import io.joynr.messaging.LocalChannelUrlDirectoryClient;
import io.joynr.messaging.MessageReceiver;
import io.joynr.messaging.MessagingPropertyKeys;
import io.joynr.messaging.routing.RoutingTable;
import io.joynr.messaging.routing.RoutingTableImpl;
import io.joynr.runtime.AbstractJoynrApplication;
import io.joynr.runtime.PropertyLoader;
import io.joynr.util.PreconfiguredEndpointDirectoryModule;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.Properties;
import java.util.UUID;

import joynr.JoynrMessage;
import joynr.system.RoutingTypes.Address;
import joynr.system.RoutingTypes.ChannelAddress;
import joynr.types.ChannelUrlInformation;

import org.junit.After;
import org.junit.Before;
import org.junit.Ignore;
import org.junit.Test;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.fasterxml.jackson.core.JsonGenerationException;
import com.google.inject.Guice;
import com.google.inject.Injector;
import com.google.inject.Key;
import com.google.inject.Module;
import com.google.inject.name.Names;

public abstract class AbstractMessagingIntegrationTest {

    // The timeout should be achievable in all test environments
    private static final int DEFAULT_TIMEOUT = 8000;

    private MessageHandlerImpl joynrMessageSender1;
    private MessageHandlerImpl joynrMessageSender2;

    private String payload1;
    private String payload2;
    private String payload3;
    private String payload4;

    private String fromParticipantId = "mySenderParticipantId";
    private String toParticipantId = "myReceiverParticipantId";

    private MessageReceiver messageReceiver1;
    private MessageReceiver messageReceiver2;
    RoutingTable routingTable1;
    RoutingTable routingTable2;
    private JoynrMessageFactory joynrMessagingFactory;

    private static final Logger logger = LoggerFactory.getLogger(AbstractMessagingIntegrationTest.class);

    private static final String STATIC_PERSISTENCE_FILE = "target/temp/persistence.properties";
    private LocalChannelUrlDirectoryClient localChannelUrlDirectoryClient;
    private DummyCapabilitiesDirectory localCapDir;

    private String bounceProxyUrl = Guice.createInjector(new JoynrPropertiesModule(new Properties()))
                                         .getInstance(Key.get(String.class,
                                                              Names.named(MessagingPropertyKeys.BOUNCE_PROXY_URL)));

    private long relativeTtl_ms = 10000L;

    private ChannelAddress address2;
    private ChannelAddress address1;

    // To be provided by subclasses
    public abstract Injector createInjector(Properties joynrConfig, Module... modules);

    @Before
    public void setUp() throws SecurityException {
        System.getProperties().put(AbstractJoynrApplication.PROPERTY_JOYNR_DOMAIN_LOCAL,
                                   "TEST-libjoynr-domain-" + System.currentTimeMillis());

        // use the same capabilities and channelUrl directory in all injectors
        localCapDir = new DummyCapabilitiesDirectory();
        localChannelUrlDirectoryClient = new DummyLocalChannelUrlDirectoryClient();

        String channelId1 = "1_" + UUID.randomUUID().toString().substring(0, 2);
        address1 = new ChannelAddress(channelId1);
        Injector injector1 = setupMessageEndpoint(channelId1, localChannelUrlDirectoryClient, localCapDir);
        joynrMessageSender1 = injector1.getInstance(MessageHandlerImpl.class);
        messageReceiver1 = injector1.getInstance(MessageReceiver.class);
        //   IMessageReceivers messageReceivers = injector1.getInstance(IMessageReceivers.class);
        // messageReceivers.registerMessageReceiver(messageReceiver1, channelId1);

        String channelId2 = "2_" + UUID.randomUUID().toString();
        address2 = new ChannelAddress(channelId2);
        Injector injector2 = setupMessageEndpoint(channelId2, localChannelUrlDirectoryClient, localCapDir);
        joynrMessageSender2 = injector2.getInstance(MessageHandlerImpl.class);
        messageReceiver2 = injector2.getInstance(MessageReceiver.class);
        //   IMessageReceivers messageReceivers2 = injector2.getInstance(IMessageReceivers.class);
        //  messageReceivers2.registerMessageReceiver(messageReceiver2, channelId2);

        joynrMessagingFactory = injector1.getInstance(JoynrMessageFactory.class);

        payload1 = "test payload 1€";
        payload2 = "test payload 2€";
        payload3 = "this message should be received";
        payload4 = "this message should be dropped";

    }

    public Injector setupMessageEndpoint(String channelId,
                                         LocalChannelUrlDirectoryClient localChannelUrlDirectoryClient,
                                         LocalCapabilitiesDirectory localCapDir) {
        RoutingTable routingTable = new RoutingTableImpl("channelurldirectory_participantid",
                                                         new ChannelAddress("discoverydirectory_channelid"),
                                                         "capabilitiesdirectory_participantid",
                                                         new ChannelAddress("discoverydirectory_channelid"),
                                                         "domainaccesscontroller_participantid",
                                                         new ChannelAddress("domainaccesscontroller_channelid"),
                                                         "discovery_participantid",
                                                         new ChannelAddress("discovery_channelid"));

        ChannelUrlInformation channelUrlInformation = new ChannelUrlInformation();
        channelUrlInformation.setUrls(new String[]{ getChannelUrl(channelId) });
        localChannelUrlDirectoryClient.registerChannelUrls(channelId, channelUrlInformation);

        Properties joynrConfig = PropertyLoader.loadProperties("testMessaging.properties");
        joynrConfig.setProperty(MessagingPropertyKeys.PERSISTENCE_FILE, STATIC_PERSISTENCE_FILE);
        joynrConfig.put(MessagingPropertyKeys.CHANNELID, channelId);
        joynrConfig.put(MessagingPropertyKeys.RECEIVERID, UUID.randomUUID().toString());
        Injector injector = createInjector(joynrConfig,
                                           new DummyDiscoveryModule(localChannelUrlDirectoryClient, localCapDir),
                                           new PreconfiguredEndpointDirectoryModule(routingTable));

        return injector;

    }

    @After
    public void tearDown() {
        logger.info("TEAR DOWN");
        messageReceiver1.shutdown(true);
        messageReceiver2.shutdown(true);
        joynrMessageSender1.shutdown();
        joynrMessageSender2.shutdown();

    }

    @Test
    public void receiveOneWayMessagesBothDirections() throws Exception {
        ExpiryDate expiryDate;

        TestMessageListener listener2 = new TestMessageListener(2);
        messageReceiver2.start(listener2);

        TestMessageListener listener1 = new TestMessageListener(1);
        messageReceiver1.start(listener1);
        Thread.sleep(50);

        // send 2 messages one way
        expiryDate = ExpiryDate.fromRelativeTtl(relativeTtl_ms);
        JoynrMessage messageA = joynrMessagingFactory.createOneWay(fromParticipantId,
                                                                   toParticipantId,
                                                                   payload1,
                                                                   expiryDate);
        JoynrMessage messageB = joynrMessagingFactory.createOneWay(fromParticipantId,
                                                                   toParticipantId,
                                                                   payload2,
                                                                   expiryDate);
        joynrMessageSender1.sendMessage(address2, messageA);
        joynrMessageSender1.sendMessage(address2, messageB);

        listener2.assertAllPayloadsReceived(DEFAULT_TIMEOUT);
        listener2.assertReceivedPayloadsContains(messageA, messageB);

        // test the other direction
        String channelId1 = messageReceiver1.getChannelId();
        Address address1 = new ChannelAddress(channelId1);

        Thread.sleep(50);

        expiryDate = ExpiryDate.fromRelativeTtl(relativeTtl_ms);
        JoynrMessage messageC = joynrMessagingFactory.createOneWay(toParticipantId,
                                                                   fromParticipantId,
                                                                   payload2,
                                                                   expiryDate);
        joynrMessageSender2.sendMessage(address1, messageC);

        listener1.assertAllPayloadsReceived(DEFAULT_TIMEOUT);
        listener1.assertReceivedPayloadsContains(messageC);
    }

    @Test
    public void testUmlautInMessagesPayload() throws Exception {
        ExpiryDate ttl_absolute_ms;
        // send message one way
        TestMessageListener listener2 = new TestMessageListener(1);
        messageReceiver2.start(listener2);
        TestMessageListener listener1 = new TestMessageListener(0, 0);
        messageReceiver1.start(listener1);

        Thread.sleep(50);

        ttl_absolute_ms = ExpiryDate.fromRelativeTtl(relativeTtl_ms);
        JoynrMessage messageA = joynrMessagingFactory.createOneWay(fromParticipantId,
                                                                   toParticipantId,
                                                                   "Test äöü",
                                                                   ttl_absolute_ms);
        joynrMessageSender1.sendMessage(address2, messageA);

        listener2.assertAllPayloadsReceived(DEFAULT_TIMEOUT);
        listener2.assertReceivedPayloadsContains(messageA);

    }

    @Test
    public void ttlCausesMessageToBeDropped() throws Exception {
        // send 2 messages one way, one should be dropped
        TestMessageListener listener1 = new TestMessageListener(0, 1);
        messageReceiver1.start(listener1);

        TestMessageListener listener2 = new TestMessageListener(1);
        messageReceiver2.start(listener2);

        // stops long poll
        messageReceiver2.suspend();

        JoynrMessage message1 = joynrMessagingFactory.createOneWay(fromParticipantId,
                                                                   toParticipantId,
                                                                   payload3,
                                                                   ExpiryDate.fromRelativeTtl(50000));
        JoynrMessage message2 = joynrMessagingFactory.createOneWay(fromParticipantId,
                                                                   toParticipantId,
                                                                   payload4,
                                                                   ExpiryDate.fromRelativeTtl(200));
        joynrMessageSender1.sendMessage(address2, message1);
        joynrMessageSender1.sendMessage(address2, message2);

        // wait ttl to cause a message to be discarded
        Thread.sleep(500);
        messageReceiver2.resume();
        listener2.assertAllPayloadsReceived(5000);
        listener2.assertReceivedPayloadsContains(message1);
        listener2.assertReceivedPayloadsContainsNot(message2);

    }

    @Test
    public void receiveMultipleMessagesInOneResponseAndDistributeToListener() throws Exception {

        TestMessageListener listener1 = new TestMessageListener(0, 0);
        messageReceiver1.start(listener1);

        TestMessageListener listener2 = new TestMessageListener(2);
        messageReceiver2.start(listener2);

        messageReceiver2.suspend();

        ExpiryDate expiryDate = ExpiryDate.fromRelativeTtl(5000); // ttl 5 sec
        JoynrMessage message1 = joynrMessagingFactory.createOneWay(fromParticipantId,
                                                                   toParticipantId,
                                                                   payload1,
                                                                   expiryDate);
        JoynrMessage message2 = joynrMessagingFactory.createOneWay(fromParticipantId,
                                                                   toParticipantId,
                                                                   payload2,
                                                                   expiryDate);

        joynrMessageSender1.sendMessage(address2, message1);
        joynrMessageSender1.sendMessage(address2, message2);

        Thread.sleep(500);
        messageReceiver2.resume();

        listener2.assertAllPayloadsReceived(DEFAULT_TIMEOUT);
        listener2.assertReceivedPayloadsContains(message1, message2);
    }

    @Test
    public void receiveOneWayMessagesBothDirections2() throws Exception {
        ExpiryDate ttl_absolute_ms;

        TestMessageListener listener2 = new TestMessageListener(2);
        messageReceiver2.start(listener2);

        TestMessageListener listener1 = new TestMessageListener(1);
        messageReceiver1.start(listener1);
        Thread.sleep(50);

        // send 2 messages one way
        ttl_absolute_ms = ExpiryDate.fromRelativeTtl(relativeTtl_ms);
        JoynrMessage messageA = joynrMessagingFactory.createOneWay(fromParticipantId,
                                                                   toParticipantId,
                                                                   payload1,
                                                                   ttl_absolute_ms);
        JoynrMessage messageB = joynrMessagingFactory.createOneWay(fromParticipantId,
                                                                   toParticipantId,
                                                                   payload2,
                                                                   ttl_absolute_ms);
        joynrMessageSender1.sendMessage(address2, messageA);
        joynrMessageSender1.sendMessage(address2, messageB);

        listener2.assertAllPayloadsReceived(DEFAULT_TIMEOUT);
        listener2.assertReceivedPayloadsContains(messageA, messageB);

        Thread.sleep(50);

        // test the other direction
        ttl_absolute_ms = ExpiryDate.fromRelativeTtl(relativeTtl_ms);
        JoynrMessage messageC = joynrMessagingFactory.createOneWay(toParticipantId,
                                                                   fromParticipantId,
                                                                   payload2,
                                                                   ttl_absolute_ms);
        joynrMessageSender2.sendMessage(address1, messageC);

        listener1.assertAllPayloadsReceived(DEFAULT_TIMEOUT);
        listener1.assertReceivedPayloadsContains(messageC);
    }

    /* this test has been disabled, as it sporadically stops with receiving messages. It does not look like a timing issue, but
     * a resource issue with sudden message loss
     */
    @Test
    @Ignore
    public void fastSendAndReplyManyMessages() throws InterruptedException, JsonGenerationException,
                                              JoynrSendBufferFullException, JoynrMessageNotSentException, IOException {
        ExpiryDate ttl_absolute_ms;
        int nMessages = 500; // TestMessageListener listener1 = new TestMessageListener(nMessages);

        TestMessageListener listener2 = new TestMessageListener(nMessages);
        messageReceiver2.start(listener2);
        TestMessageListener listener1 = new TestMessageListener(nMessages);
        messageReceiver1.start(listener1);
        int message_delay_ms = 50;
        // wait 5 secs plus message_delay_ms per message extra
        int ttlForManyMessages = DEFAULT_TIMEOUT + nMessages * message_delay_ms;

        Thread.sleep(message_delay_ms);

        ttl_absolute_ms = ExpiryDate.fromRelativeTtl(ttlForManyMessages);

        List<JoynrMessage> messages = new ArrayList<JoynrMessage>();
        for (int i = 0; i < nMessages; i++) {
            JoynrMessage message = joynrMessagingFactory.createOneWay(fromParticipantId, toParticipantId, payload1
                    + "message:" + i, ttl_absolute_ms);
            messages.add(message);
            joynrMessageSender1.sendMessage(address2, message);
        }

        listener2.assertAllPayloadsReceived(ttlForManyMessages);

        listener2.assertReceivedPayloadsContains(messages.toArray());

    }

    // this test is a stub. There is currently on way to check, whether a channel is really deleted. One can use
    // this test to check manually
    @Test
    @Ignore
    public void deleteChannels() throws Exception {
        logger.info("channel id of dispatcher1: " + messageReceiver1.getChannelId());
        // Thread.sleep(5 * 1000);
        messageReceiver1.shutdown(true);
        // Thread.sleep(5 * 1000);
        logger.info("Channel url should be deleted now");
    }

    private String getChannelUrl(String channelId) {
        return bounceProxyUrl + "channels/" + channelId + "/";
    }

}
