package io.joynr.integration;

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

import com.google.inject.Module;
import io.joynr.accesscontrol.StaticDomainAccessControlProvisioningModule;
import io.joynr.arbitration.ArbitrationStrategy;
import io.joynr.arbitration.DiscoveryQos;
import io.joynr.exceptions.DiscoveryException;
import io.joynr.exceptions.JoynrIllegalStateException;
import io.joynr.messaging.MessagingPropertyKeys;
import io.joynr.messaging.MessagingQos;
import io.joynr.proxy.Future;
import io.joynr.proxy.ProxyBuilder;
import io.joynr.runtime.AbstractJoynrApplication;
import io.joynr.runtime.JoynrRuntime;
import io.joynr.runtime.PropertyLoader;
import joynr.OnChangeSubscriptionQos;
import joynr.exceptions.ApplicationException;
import joynr.tests.DefaulttestProvider;
import joynr.tests.testBroadcastInterface;
import joynr.tests.testBroadcastInterface.LocationUpdateSelectiveBroadcastFilterParameters;
import joynr.tests.testLocationUpdateSelectiveBroadcastFilter;
import joynr.tests.testProxy;
import joynr.tests.testTypes.TestEnum;
import joynr.types.ProviderQos;
import joynr.types.Localisation.GpsFixEnum;
import joynr.types.Localisation.GpsLocation;

import org.junit.After;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TestName;
import org.mockito.MockitoAnnotations;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.Properties;
import java.util.UUID;
import java.util.concurrent.Semaphore;
import java.util.concurrent.TimeUnit;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertArrayEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.fail;

public abstract class AbstractBroadcastEnd2EndTest extends JoynrEnd2EndTest {
    private static final Logger logger = LoggerFactory.getLogger(AbstractBroadcastEnd2EndTest.class);

    protected static class TestProvider extends DefaulttestProvider {
        public TestProvider(ProviderQos providerQos) {
            this.providerQos = providerQos;
        }
    }

    // This timeout must be shared by all integration test environments and
    // cannot be too short.
    private static final int CONST_DEFAULT_TEST_TIMEOUT = 8000;

    @Rule
    public TestName name = new TestName();

    private static TestProvider provider;
    private static testProxy proxy;
    private String domain;

    private static GpsLocation expectedLocation = new GpsLocation(1.0,
                                                                  2.0,
                                                                  3.0,
                                                                  GpsFixEnum.MODE2D,
                                                                  4.0,
                                                                  5.0,
                                                                  6.0,
                                                                  7.0,
                                                                  8l,
                                                                  9l,
                                                                  23);
    private static Float expectedSpeed = 100.0f;

    private JoynrRuntime providerRuntime;
    private JoynrRuntime consumerRuntime;

    protected ProviderQos providerQos = new ProviderQos();
    protected MessagingQos messagingQos = new MessagingQos(10000);
    protected DiscoveryQos discoveryQos = new DiscoveryQos(10000, ArbitrationStrategy.HighestPriority, Long.MAX_VALUE);

    // Overridden by test environment implementations
    protected abstract JoynrRuntime getRuntime(Properties joynrConfig, Module... modules);

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        String methodName = name.getMethodName();
        domain = "ProviderDomain-BroadcastEnd2End-" + methodName + "-" + System.currentTimeMillis();
        provisionPermissiveAccessControlEntry(domain, DefaulttestProvider.INTERFACE_NAME);
        setupProviderRuntime(methodName);
        setupConsumerRuntime(methodName);
        logger.info("Starting {} ...", methodName);
    }

    @After
    public void tearDown() throws Exception {
        providerRuntime.shutdown(true);
        consumerRuntime.shutdown(true);
    }

    private void setupProviderRuntime(String methodName) throws InterruptedException, ApplicationException {
        Properties factoryPropertiesProvider;

        String channelIdProvider = "JavaTest-" + UUID.randomUUID().getLeastSignificantBits()
                + "-Provider-BroadcastEnd2EndTest-" + methodName;

        factoryPropertiesProvider = PropertyLoader.loadProperties("testMessaging.properties");
        factoryPropertiesProvider.put(MessagingPropertyKeys.CHANNELID, channelIdProvider);
        factoryPropertiesProvider.put(MessagingPropertyKeys.RECEIVERID, UUID.randomUUID().toString());
        factoryPropertiesProvider.put(AbstractJoynrApplication.PROPERTY_JOYNR_DOMAIN_LOCAL, domain);
        providerRuntime = getRuntime(factoryPropertiesProvider, new StaticDomainAccessControlProvisioningModule());

        provider = new TestProvider(providerQos);
        Future<Void> voidFuture = providerRuntime.registerProvider(domain, provider);//.waitForFullRegistration(CONST_DEFAULT_TEST_TIMEOUT);
        voidFuture.get(CONST_DEFAULT_TEST_TIMEOUT);
    }

    private void setupConsumerRuntime(String methodName) throws DiscoveryException, JoynrIllegalStateException,
                                                        InterruptedException {
        String channelIdConsumer = "JavaTest-" + UUID.randomUUID().getLeastSignificantBits()
                + "-Consumer-BroadcastEnd2EndTest-" + methodName;

        Properties factoryPropertiesB = PropertyLoader.loadProperties("testMessaging.properties");
        factoryPropertiesB.put(MessagingPropertyKeys.CHANNELID, channelIdConsumer);
        factoryPropertiesB.put(MessagingPropertyKeys.RECEIVERID, UUID.randomUUID().toString());
        factoryPropertiesB.put(AbstractJoynrApplication.PROPERTY_JOYNR_DOMAIN_LOCAL, "ClientDomain-" + methodName + "-"
                + UUID.randomUUID().toString());

        consumerRuntime = getRuntime(factoryPropertiesB);

        ProxyBuilder<testProxy> proxyBuilder = consumerRuntime.getProxyBuilder(domain, testProxy.class);

        proxy = proxyBuilder.setMessagingQos(messagingQos).setDiscoveryQos(discoveryQos).build();
        // Wait until all the registrations and lookups are finished, to make
        // sure the timings are correct once the tests start by sending a sync
        // request to the test-proxy
        proxy.getFirstPrime();
        logger.trace("Sync call to proxy finished");

    }

    @Test(timeout = CONST_DEFAULT_TEST_TIMEOUT)
    public void subscribeToBroadcastOneOutput() throws InterruptedException {

        final Semaphore broadcastReceived = new Semaphore(0);

        long minInterval = 0;
        long ttl = CONST_DEFAULT_TEST_TIMEOUT;
        long expiryDate_ms = System.currentTimeMillis() + CONST_DEFAULT_TEST_TIMEOUT;
        OnChangeSubscriptionQos subscriptionQos = new OnChangeSubscriptionQos(minInterval, expiryDate_ms, ttl);
        proxy.subscribeToLocationUpdateBroadcast(new testBroadcastInterface.LocationUpdateBroadcastAdapter() {

            @Override
            public void onReceive(GpsLocation location) {
                assertEquals(expectedLocation, location);
                broadcastReceived.release();
            }
        }, subscriptionQos);

        Thread.sleep(300);

        provider.fireLocationUpdate(expectedLocation);
        broadcastReceived.acquire();
    }

    @Test(timeout = CONST_DEFAULT_TEST_TIMEOUT)
    public void subscribeToBroadcastMultipleOutputs() throws InterruptedException {
        final Semaphore broadcastReceived = new Semaphore(0);

        long minInterval = 0;
        long ttl = CONST_DEFAULT_TEST_TIMEOUT;
        long expiryDate_ms = System.currentTimeMillis() + CONST_DEFAULT_TEST_TIMEOUT;
        OnChangeSubscriptionQos subscriptionQos = new OnChangeSubscriptionQos(minInterval, expiryDate_ms, ttl);
        proxy.subscribeToLocationUpdateWithSpeedBroadcast(new testBroadcastInterface.LocationUpdateWithSpeedBroadcastAdapter() {

                                                              @Override
                                                              public void onReceive(GpsLocation location, Float speed) {
                                                                  assertEquals(expectedLocation, location);
                                                                  assertEquals(expectedSpeed, speed);
                                                                  broadcastReceived.release();
                                                              }
                                                          },
                                                          subscriptionQos);

        Thread.sleep(300);

        provider.fireLocationUpdateWithSpeed(expectedLocation, expectedSpeed);
        broadcastReceived.acquire();
    }

    @Test(timeout = CONST_DEFAULT_TEST_TIMEOUT)
    public void subscribeToBroadcastWithEnumOutput() throws InterruptedException {
        final Semaphore broadcastReceived = new Semaphore(0);
        final TestEnum expectedTestEnum = TestEnum.TWO;

        long minInterval = 0;
        long ttl = CONST_DEFAULT_TEST_TIMEOUT;
        long expiryDate_ms = System.currentTimeMillis() + CONST_DEFAULT_TEST_TIMEOUT;
        OnChangeSubscriptionQos subscriptionQos = new OnChangeSubscriptionQos(minInterval, expiryDate_ms, ttl);
        proxy.subscribeToBroadcastWithEnumOutputBroadcast(new testBroadcastInterface.BroadcastWithEnumOutputBroadcastListener() {

                                                              @Override
                                                              public void onReceive(TestEnum testEnum) {
                                                                  assertEquals(expectedTestEnum, testEnum);
                                                                  broadcastReceived.release();
                                                              }

                                                              @Override
                                                              public void onError() {
                                                                  fail("Error while receiving broadcast");
                                                              }
                                                          },
                                                          subscriptionQos);
        Thread.sleep(300);

        provider.fireBroadcastWithEnumOutput(expectedTestEnum);
        broadcastReceived.acquire();
    }

    @Test(timeout = CONST_DEFAULT_TEST_TIMEOUT)
    public void subscribeToBroadcastWithByteBufferOutput() throws InterruptedException {
        final Semaphore broadcastReceived = new Semaphore(0);
        final Byte[] expectedByteBuffer = { 1, 2, 3 };

        long minInterval = 0;
        long ttl = CONST_DEFAULT_TEST_TIMEOUT;
        long expiryDate_ms = System.currentTimeMillis() + CONST_DEFAULT_TEST_TIMEOUT;
        OnChangeSubscriptionQos subscriptionQos = new OnChangeSubscriptionQos(minInterval, expiryDate_ms, ttl);
        proxy.subscribeToBroadcastWithByteBufferParameterBroadcast(new testBroadcastInterface.BroadcastWithByteBufferParameterBroadcastListener() {

                                                                       @Override
                                                                       public void onError() {
                                                                           fail("Error while receiving broadcast");
                                                                       }

                                                                       @Override
                                                                       public void onReceive(Byte[] byteBufferParameter) {
                                                                           assertArrayEquals(expectedByteBuffer,
                                                                                             byteBufferParameter);
                                                                           broadcastReceived.release();
                                                                       }
                                                                   },
                                                                   subscriptionQos);
        Thread.sleep(300);

        provider.fireBroadcastWithByteBufferParameter(expectedByteBuffer);
        broadcastReceived.acquire();
    }

    @Test(timeout = CONST_DEFAULT_TEST_TIMEOUT)
    public void subscribeAndUnsubscribeFromBroadcast() throws InterruptedException {

        final Semaphore broadcastReceived = new Semaphore(0);

        long minInterval = 0;
        long ttl = CONST_DEFAULT_TEST_TIMEOUT;
        long expiryDate_ms = System.currentTimeMillis() + CONST_DEFAULT_TEST_TIMEOUT;
        OnChangeSubscriptionQos subscriptionQos = new OnChangeSubscriptionQos(minInterval, expiryDate_ms, ttl);
        String subscriptionId = proxy.subscribeToLocationUpdateWithSpeedBroadcast(new testBroadcastInterface.LocationUpdateWithSpeedBroadcastAdapter() {

                                                                                      @Override
                                                                                      public void onReceive(GpsLocation location,
                                                                                                            Float speed) {
                                                                                          assertEquals(expectedLocation,
                                                                                                       location);
                                                                                          assertEquals(expectedSpeed,
                                                                                                       speed);
                                                                                          broadcastReceived.release();
                                                                                      }
                                                                                  },
                                                                                  subscriptionQos);

        Thread.sleep(300);

        provider.fireLocationUpdateWithSpeed(expectedLocation, expectedSpeed);
        broadcastReceived.acquire();

        //unsubscribe correct subscription -> now, no more broadcast shall be received
        proxy.unsubscribeFromLocationUpdateWithSpeedBroadcast(UUID.randomUUID().toString());
        provider.fireLocationUpdateWithSpeed(expectedLocation, expectedSpeed);
        broadcastReceived.acquire();

        //unsubscribe correct subscription -> now, no more broadcast shall be received
        proxy.unsubscribeFromLocationUpdateWithSpeedBroadcast(subscriptionId);
        Thread.sleep(300);
        provider.fireLocationUpdateWithSpeed(expectedLocation, expectedSpeed);
        assertFalse(broadcastReceived.tryAcquire(300, TimeUnit.MILLISECONDS));
    }

    @Test(timeout = CONST_DEFAULT_TEST_TIMEOUT)
    public void subscribeToSelectiveBroadcast_FilterTrue() throws InterruptedException {

        final Semaphore broadcastReceived = new Semaphore(0);

        final LocationUpdateSelectiveBroadcastFilterParameters testFilterParameters = new LocationUpdateSelectiveBroadcastFilterParameters();
        testFilterParameters.setCountry("Germany");
        testFilterParameters.setStartTime("4.00 pm");

        testLocationUpdateSelectiveBroadcastFilter filter1 = new testLocationUpdateSelectiveBroadcastFilter() {

            @Override
            public boolean filter(GpsLocation location,
                                  LocationUpdateSelectiveBroadcastFilterParameters filterParameters) {

                assertEquals(testFilterParameters, filterParameters);
                return true;
            }
        };
        testLocationUpdateSelectiveBroadcastFilter filter2 = new testLocationUpdateSelectiveBroadcastFilter() {

            @Override
            public boolean filter(GpsLocation location,
                                  LocationUpdateSelectiveBroadcastFilterParameters filterParameters) {
                assertEquals(testFilterParameters, filterParameters);
                return true;
            }
        };

        provider.addBroadcastFilter(filter1);
        provider.addBroadcastFilter(filter2);

        long minInterval = 0;
        long ttl = CONST_DEFAULT_TEST_TIMEOUT;
        long expiryDate_ms = System.currentTimeMillis() + CONST_DEFAULT_TEST_TIMEOUT;
        OnChangeSubscriptionQos subscriptionQos = new OnChangeSubscriptionQos(minInterval, expiryDate_ms, ttl);
        proxy.subscribeToLocationUpdateSelectiveBroadcast(new testBroadcastInterface.LocationUpdateSelectiveBroadcastAdapter() {

                                                              @Override
                                                              public void onReceive(GpsLocation location) {
                                                                  assertEquals(expectedLocation, location);
                                                                  broadcastReceived.release();
                                                              }
                                                          },
                                                          subscriptionQos,
                                                          testFilterParameters);

        Thread.sleep(300);

        provider.fireLocationUpdateSelective(expectedLocation);
        broadcastReceived.acquire();
    }

    @Test(timeout = CONST_DEFAULT_TEST_TIMEOUT)
    public void subscribeToSelectiveBroadcast_FilterFalse() throws InterruptedException {

        final Semaphore broadcastReceived = new Semaphore(0);

        final LocationUpdateSelectiveBroadcastFilterParameters testFilterParameters = new LocationUpdateSelectiveBroadcastFilterParameters();
        testFilterParameters.setCountry("Germany");
        testFilterParameters.setStartTime("4.00 pm");

        testLocationUpdateSelectiveBroadcastFilter filter1 = new testLocationUpdateSelectiveBroadcastFilter() {

            @Override
            public boolean filter(GpsLocation location,
                                  LocationUpdateSelectiveBroadcastFilterParameters filterParameters) {
                assertEquals(testFilterParameters, filterParameters);
                return true;
            }
        };
        testLocationUpdateSelectiveBroadcastFilter filter2 = new testLocationUpdateSelectiveBroadcastFilter() {

            @Override
            public boolean filter(GpsLocation location,
                                  LocationUpdateSelectiveBroadcastFilterParameters filterParameters) {
                assertEquals(testFilterParameters, filterParameters);
                return false;
            }
        };

        provider.addBroadcastFilter(filter1);
        provider.addBroadcastFilter(filter2);

        long minInterval = 0;
        long ttl = CONST_DEFAULT_TEST_TIMEOUT;
        long expiryDate_ms = System.currentTimeMillis() + CONST_DEFAULT_TEST_TIMEOUT;
        OnChangeSubscriptionQos subscriptionQos = new OnChangeSubscriptionQos(minInterval, expiryDate_ms, ttl);
        proxy.subscribeToLocationUpdateSelectiveBroadcast(new testBroadcastInterface.LocationUpdateSelectiveBroadcastAdapter() {

                                                              @Override
                                                              public void onReceive(GpsLocation location) {
                                                                  assertEquals(expectedLocation, location);
                                                                  broadcastReceived.release();
                                                              }
                                                          },
                                                          subscriptionQos,
                                                          testFilterParameters);

        Thread.sleep(300);

        provider.fireLocationUpdateSelective(expectedLocation);
        assertFalse(broadcastReceived.tryAcquire(500, TimeUnit.MILLISECONDS));
    }
}
