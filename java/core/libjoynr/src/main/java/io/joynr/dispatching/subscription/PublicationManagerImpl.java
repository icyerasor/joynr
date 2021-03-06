package io.joynr.dispatching.subscription;

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
import io.joynr.dispatcher.rpc.ReflectionUtils;
import io.joynr.dispatching.CallerDirectoryListener;
import io.joynr.dispatching.Dispatcher;
import io.joynr.dispatching.RequestCaller;
import io.joynr.dispatching.RequestCallerDirectory;
import io.joynr.exceptions.JoynrException;
import io.joynr.exceptions.JoynrMessageNotSentException;
import io.joynr.exceptions.JoynrRuntimeException;
import io.joynr.exceptions.JoynrSendBufferFullException;
import io.joynr.messaging.MessagingQos;
import io.joynr.provider.Promise;
import io.joynr.provider.PromiseListener;
import io.joynr.pubsub.HeartbeatSubscriptionInformation;
import io.joynr.pubsub.SubscriptionQos;
import io.joynr.pubsub.publication.AttributeListener;
import io.joynr.pubsub.publication.BroadcastFilter;
import io.joynr.pubsub.publication.BroadcastListener;

import java.io.IOException;
import java.lang.reflect.Method;
import java.util.Arrays;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;
import java.util.concurrent.ConcurrentMap;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.ScheduledFuture;
import java.util.concurrent.TimeUnit;

import joynr.BroadcastFilterParameters;
import joynr.BroadcastSubscriptionRequest;
import joynr.OnChangeSubscriptionQos;
import joynr.SubscriptionPublication;
import joynr.SubscriptionRequest;
import joynr.exceptions.ProviderRuntimeException;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.fasterxml.jackson.core.JsonGenerationException;
import com.fasterxml.jackson.databind.JsonMappingException;
import com.google.common.collect.HashMultimap;
import com.google.common.collect.Maps;
import com.google.common.collect.Multimap;
import com.google.inject.Inject;
import com.google.inject.Singleton;
import com.google.inject.name.Named;

@Singleton
public class PublicationManagerImpl implements PublicationManager, CallerDirectoryListener<RequestCaller> {
    private static final Logger logger = LoggerFactory.getLogger(PublicationManagerImpl.class);
    // Map ProviderId -> SubscriptionRequest
    private final Multimap<String, PublicationInformation> queuedSubscriptionRequests;
    // Map SubscriptionId -> SubscriptionRequest
    private final ConcurrentMap<String, PublicationInformation> subscriptionId2PublicationInformation;
    // Map SubscriptionId -> PublicationTimer
    private final ConcurrentMap<String, PublicationTimer> publicationTimers;
    // Map SubscriptionId -> ScheduledFuture
    private final ConcurrentMap<String, ScheduledFuture<?>> subscriptionEndFutures;
    // Map SubscriptionId -> UnregisterAttributeListener
    private final ConcurrentMap<String, UnregisterAttributeListener> unregisterAttributeListeners;
    // Map SubscriptionId -> UnregisterBroadcastListener
    private final ConcurrentMap<String, UnregisterBroadcastListener> unregisterBroadcastListeners;

    private AttributePollInterpreter attributePollInterpreter;
    private ScheduledExecutorService cleanupScheduler;
    private Dispatcher dispatcher;
    private RequestCallerDirectory requestCallerDirectory;

    static class PublicationInformation {
        private String providerParticipantId;
        private String proxyParticipantId;
        private SubscriptionRequest subscriptionRequest;
        private PubSubState pubState;

        PublicationInformation(String providerParticipantId,
                               String proxyParticipantId,
                               SubscriptionRequest subscriptionRequest) {
            pubState = new PubSubState();
            this.setProviderParticipantId(providerParticipantId);
            this.subscriptionRequest = subscriptionRequest;
            this.setProxyParticipantId(proxyParticipantId);
        }

        public String getProviderParticipantId() {
            return providerParticipantId;
        }

        public void setProviderParticipantId(String providerParticipantId) {
            this.providerParticipantId = providerParticipantId;
        }

        public String getProxyParticipantId() {
            return proxyParticipantId;
        }

        public void setProxyParticipantId(String proxyParticipantId) {
            this.proxyParticipantId = proxyParticipantId;
        }

        public String getSubscriptionId() {
            return subscriptionRequest.getSubscriptionId();
        }

        public PubSubState getState() {
            return pubState;
        }

        public SubscriptionQos getQos() {
            return subscriptionRequest.getQos();
        }

        @Override
        public boolean equals(Object arg0) {
            if (!(arg0 instanceof PublicationInformation)) {
                return false;
            }
            PublicationInformation pi = (PublicationInformation) arg0;
            return proxyParticipantId.equals(pi.proxyParticipantId)
                    && providerParticipantId.equals(pi.providerParticipantId)
                    && subscriptionRequest.equals(pi.subscriptionRequest);
        }

        @Override
        public int hashCode() {
            final int prime = 31;
            int result = 1;
            result = prime * result + ((proxyParticipantId == null) ? 0 : proxyParticipantId.hashCode());
            result = prime * result + ((providerParticipantId == null) ? 0 : providerParticipantId.hashCode());
            result = prime * result + ((subscriptionRequest == null) ? 0 : subscriptionRequest.hashCode());
            return result;
        }

        public String getSubscribedToName() {
            return subscriptionRequest.getSubscribedToName();
        }
    }

    @Inject
    public PublicationManagerImpl(AttributePollInterpreter attributePollInterpreter,
                                  Dispatcher dispatcher,
                                  RequestCallerDirectory requestCallerDirectory,
                                  @Named(JOYNR_SCHEDULER_CLEANUP) ScheduledExecutorService cleanupScheduler) {
        super();
        this.dispatcher = dispatcher;
        this.requestCallerDirectory = requestCallerDirectory;
        this.cleanupScheduler = cleanupScheduler;
        this.queuedSubscriptionRequests = HashMultimap.create();
        this.subscriptionId2PublicationInformation = Maps.newConcurrentMap();
        this.publicationTimers = Maps.newConcurrentMap();
        this.subscriptionEndFutures = Maps.newConcurrentMap();
        this.unregisterAttributeListeners = Maps.newConcurrentMap();
        this.unregisterBroadcastListeners = Maps.newConcurrentMap();
        this.attributePollInterpreter = attributePollInterpreter;
        requestCallerDirectory.addListener(this);

    }

    private void handleSubscriptionRequest(PublicationInformation publicationInformation,
                                           SubscriptionRequest subscriptionRequest,
                                           RequestCaller requestCaller) {

        final String subscriptionId = subscriptionRequest.getSubscriptionId();

        SubscriptionQos subscriptionQos = subscriptionRequest.getQos();

        try {
            Method method = findGetterForAttributeName(requestCaller.getClass(),
                                                       subscriptionRequest.getSubscribedToName());

            // Send initial publication
            triggerPublication(publicationInformation, requestCaller, method);

            boolean hasSubscriptionHeartBeat = subscriptionQos instanceof HeartbeatSubscriptionInformation;
            boolean isOnChangeSubscription = subscriptionQos instanceof OnChangeSubscriptionQos;

            if (hasSubscriptionHeartBeat || isOnChangeSubscription) {
                final PublicationTimer timer = new PublicationTimer(publicationInformation,
                                                                    method,
                                                                    requestCaller,
                                                                    this,
                                                                    attributePollInterpreter);

                timer.startTimer();
                publicationTimers.put(subscriptionId, timer);
            }

            // Handle onChange subscriptions
            if (subscriptionQos instanceof OnChangeSubscriptionQos) {
                AttributeListener attributeListener = new AttributeListenerImpl(subscriptionId, this);
                String attributeName = subscriptionRequest.getSubscribedToName();
                requestCaller.registerAttributeListener(attributeName, attributeListener);
                unregisterAttributeListeners.put(subscriptionId, new UnregisterAttributeListener(requestCaller,
                                                                                                 attributeName,
                                                                                                 attributeListener));
            }
        } catch (NoSuchMethodException e) {
            cancelPublicationCreation(subscriptionId);
            logger.error("Error subscribing: {}. The provider does not have the requested attribute",
                         subscriptionRequest);
            throw new JoynrRuntimeException("Error subscribing: The provider does not have the requested attribute: "
                    + e);
        } catch (IllegalArgumentException e) {
            cancelPublicationCreation(subscriptionId);
            logger.error("Error subscribing: " + subscriptionRequest, e);
            throw new JoynrRuntimeException("Error subscribing: " + e);

        }

    }

    private void handleBroadcastSubscriptionRequest(String proxyParticipantId,
                                                    String providerParticipantId,
                                                    BroadcastSubscriptionRequest subscriptionRequest,
                                                    RequestCaller requestCaller) {
        logger.debug("adding broadcast publication: " + subscriptionRequest.toString());

        BroadcastListener broadcastListener = new BroadcastListenerImpl(subscriptionRequest.getSubscriptionId(), this);
        String broadcastName = subscriptionRequest.getSubscribedToName();
        requestCaller.registerBroadcastListener(broadcastName, broadcastListener);
        unregisterBroadcastListeners.put(subscriptionRequest.getSubscriptionId(),
                                         new UnregisterBroadcastListener(requestCaller,
                                                                         broadcastName,
                                                                         broadcastListener));
    }

    private void addSubscriptionRequest(String proxyParticipantId,
                                        String providerParticipantId,
                                        SubscriptionRequest subscriptionRequest,
                                        RequestCaller requestCaller) {

        PublicationInformation publicationInformation = new PublicationInformation(providerParticipantId,
                                                                                   proxyParticipantId,
                                                                                   subscriptionRequest);

        try {
            // Check that this is a valid subscription
            SubscriptionQos subscriptionQos = subscriptionRequest.getQos();
            long subscriptionEndDelay = subscriptionQos.getExpiryDate() == SubscriptionQos.NO_EXPIRY_DATE ? SubscriptionQos.NO_EXPIRY_DATE
                    : subscriptionQos.getExpiryDate() - System.currentTimeMillis();

            if (subscriptionEndDelay < 0) {
                logger.error("Not adding subscription which ends in {} ms", subscriptionEndDelay);
                return;
            }

            // See if the publications for this subscription are already handled
            final String subscriptionId = subscriptionRequest.getSubscriptionId();
            if (publicationExists(subscriptionId)) {
                logger.debug("updating publication: " + subscriptionRequest.toString());
                removePublication(subscriptionId);
            } else {
                logger.debug("adding publication: " + subscriptionRequest.toString());
            }

            subscriptionId2PublicationInformation.put(subscriptionId, publicationInformation);

            if (subscriptionRequest instanceof BroadcastSubscriptionRequest) {
                handleBroadcastSubscriptionRequest(proxyParticipantId,
                                                   providerParticipantId,
                                                   (BroadcastSubscriptionRequest) subscriptionRequest,
                                                   requestCaller);
            } else {
                handleSubscriptionRequest(publicationInformation, subscriptionRequest, requestCaller);
            }

            if (subscriptionQos.getExpiryDate() != SubscriptionQos.NO_EXPIRY_DATE) {
                // Create a runnable to remove the publication when the subscription expires
                ScheduledFuture<?> subscriptionEndFuture = cleanupScheduler.schedule(new Runnable() {

                    @Override
                    public void run() {
                        logger.debug("Publication with Id " + subscriptionId + " expired...");
                        removePublication(subscriptionId);
                    }

                }, subscriptionEndDelay, TimeUnit.MILLISECONDS);
                subscriptionEndFutures.put(subscriptionId, subscriptionEndFuture);
            }
            logger.debug("publication added: " + subscriptionRequest.toString());
        } catch (Exception e) {
            JoynrRuntimeException error = new JoynrRuntimeException("Error processing subscription request ("
                    + subscriptionRequest + "): " + e);
            sendPublicationError(error, publicationInformation);
        }
    }

    private void cancelPublicationCreation(String subscriptionId) {
        subscriptionId2PublicationInformation.remove(subscriptionId);
        logger.error("Subscription request rejected. Removing publication.");
    }

    private boolean publicationExists(String subscriptionId) {
        return subscriptionId2PublicationInformation.containsKey(subscriptionId);
    }

    @Override
    public void addSubscriptionRequest(String proxyParticipantId,
                                       String providerParticipantId,
                                       SubscriptionRequest subscriptionRequest) {
        if (requestCallerDirectory.containsCaller(providerParticipantId)) {
            addSubscriptionRequest(proxyParticipantId,
                                   providerParticipantId,
                                   subscriptionRequest,
                                   requestCallerDirectory.getCaller(providerParticipantId));
        } else {
            logger.debug("Adding subscription request for non existing provider to queue.");
            PublicationInformation publicationInformation = new PublicationInformation(providerParticipantId,
                                                                                       proxyParticipantId,
                                                                                       subscriptionRequest);
            queuedSubscriptionRequests.put(providerParticipantId, publicationInformation);
            subscriptionId2PublicationInformation.put(subscriptionRequest.getSubscriptionId(), publicationInformation);
        }
    }

    protected void removePublication(String subscriptionId) {
        PublicationInformation publicationInformation = subscriptionId2PublicationInformation.remove(subscriptionId);
        if (publicationInformation == null) {
            return;
        }

        // Remove (eventually) queued subcriptionRequest
        Collection<PublicationInformation> queuedSubscriptionRequestsForParticipant = queuedSubscriptionRequests.get(publicationInformation.getProviderParticipantId());

        if (queuedSubscriptionRequestsForParticipant != null) {
            queuedSubscriptionRequestsForParticipant.remove(publicationInformation);
        }

        PublicationTimer publicationTimer = publicationTimers.remove(subscriptionId);
        if (publicationTimer != null) {
            publicationTimer.cancel();
        }

        ScheduledFuture<?> future = subscriptionEndFutures.remove(subscriptionId);
        if (future != null) {
            future.cancel(true);
        }

        UnregisterAttributeListener unregisterAttributeListener = unregisterAttributeListeners.remove(subscriptionId);
        if (unregisterAttributeListener != null) {
            unregisterAttributeListener.unregister();
        }
        UnregisterBroadcastListener unregisterBroadcastListener = unregisterBroadcastListeners.remove(subscriptionId);
        if (unregisterBroadcastListener != null) {
            unregisterBroadcastListener.unregister();
        }
    }

    // Class that holds information needed to unregister attribute listener
    static class UnregisterAttributeListener {
        final RequestCaller requestCaller;
        final String attributeName;
        final AttributeListener attributeListener;

        public UnregisterAttributeListener(RequestCaller requestCaller,
                                           String attributeName,
                                           AttributeListener attributeListener) {
            this.requestCaller = requestCaller;
            this.attributeName = attributeName;
            this.attributeListener = attributeListener;
        }

        public void unregister() {
            requestCaller.unregisterAttributeListener(attributeName, attributeListener);
        }
    }

    // Class that holds information needed to unregister broadcast listener
    static class UnregisterBroadcastListener {
        final RequestCaller requestCaller;
        final String broadcastName;
        final BroadcastListener broadcastListener;

        public UnregisterBroadcastListener(RequestCaller requestCaller,
                                           String broadcastName,
                                           BroadcastListener broadcastListener) {
            this.requestCaller = requestCaller;
            this.broadcastName = broadcastName;
            this.broadcastListener = broadcastListener;
        }

        public void unregister() {
            requestCaller.unregisterBroadcastListener(broadcastName, broadcastListener);
        }
    }

    @Override
    public void stopPublication(String subscriptionId) {
        PublicationInformation publicationInformation = subscriptionId2PublicationInformation.get(subscriptionId);
        if (publicationInformation == null) {
            return;
        }
        try {
            removePublication(subscriptionId);
        } catch (Exception e) {
            JoynrRuntimeException error = new JoynrRuntimeException("Error stopping subscription " + subscriptionId
                    + ": " + e);
            sendPublicationError(error, publicationInformation);
        }
    }

    /**
     * Stops all publications for a provider
     * 
     * @param providerId provider for which all publication should be stopped
     */
    private void stopPublicationByProviderId(String providerParticipantId) {
        for (PublicationInformation publicationInformation : subscriptionId2PublicationInformation.values()) {
            if (publicationInformation.getProviderParticipantId().equals(providerParticipantId)) {
                removePublication(publicationInformation.getSubscriptionId());
            }
        }

        if (providerParticipantId != null && queuedSubscriptionRequests.containsKey(providerParticipantId)) {
            queuedSubscriptionRequests.removeAll(providerParticipantId);
        }
    }

    private boolean isExpired(PublicationInformation publicationInformation) {
        long expiryDate = publicationInformation.subscriptionRequest.getQos().getExpiryDate();
        logger.debug("ExpiryDate - System.currentTimeMillis: " + (expiryDate - System.currentTimeMillis()));
        return (expiryDate != SubscriptionQos.NO_EXPIRY_DATE && expiryDate <= System.currentTimeMillis());
    }

    /**
     * Called every time a provider is registered to check whether there are already
     * subscriptionRequests waiting.
     * 
     * @param providerId provider id
     * @param requestCaller request caller
     */
    private void restoreQueuedSubscription(String providerId, RequestCaller requestCaller) {
        Collection<PublicationInformation> queuedRequests = queuedSubscriptionRequests.get(providerId);
        Iterator<PublicationInformation> queuedRequestsIterator = queuedRequests.iterator();
        while (queuedRequestsIterator.hasNext()) {
            PublicationInformation publicInformation = queuedRequestsIterator.next();
            queuedRequestsIterator.remove();
            if (!isExpired(publicInformation)) {
                addSubscriptionRequest(publicInformation.getProxyParticipantId(),
                                       publicInformation.getProviderParticipantId(),
                                       publicInformation.subscriptionRequest,
                                       requestCaller);
            }
        }
    }

    @Override
    public void attributeValueChanged(String subscriptionId, Object value) {

        if (subscriptionId2PublicationInformation.containsKey(subscriptionId)) {
            PublicationInformation publicationInformation = subscriptionId2PublicationInformation.get(subscriptionId);

            if (isExpired(publicationInformation)) {
                removePublication(subscriptionId);
            } else {
                PublicationTimer publicationTimer = publicationTimers.get(subscriptionId);
                SubscriptionPublication publication = prepareAttributePublication(value, subscriptionId);
                if (publicationTimer != null) {
                    // used by OnChangedWithKeepAlive
                    publicationTimer.sendPublicationNow(publication);
                } else {
                    sendPublication(publication, publicationInformation);
                }

                logger.debug("attribute changed for subscription id: {} sending publication if delay > minInterval.",
                             subscriptionId);
            }

        } else {
            logger.error("subscription {} has expired but attributeValueChanged has been called", subscriptionId);
            return;
        }

    }

    @Override
    public void broadcastOccurred(String subscriptionId, List<BroadcastFilter> filters, Object... values) {
        if (subscriptionId2PublicationInformation.containsKey(subscriptionId)) {
            PublicationInformation publicationInformation = subscriptionId2PublicationInformation.get(subscriptionId);

            if (processFilterChain(publicationInformation, filters, values)) {
                long minInterval = ((OnChangeSubscriptionQos) publicationInformation.getQos()).getMinInterval();
                if (minInterval <= System.currentTimeMillis()
                        - publicationInformation.getState().getTimeOfLastPublication()) {
                    sendPublication(prepareBroadcastPublication(Arrays.asList(values), subscriptionId),
                                    publicationInformation);
                    logger.debug("event occured changed for subscription id: {} sending publication: ", subscriptionId);
                } else {
                    logger.debug("Two subsequent broadcasts of event " + publicationInformation.getSubscribedToName()
                            + " occured within minInterval of subscription with id "
                            + publicationInformation.getSubscriptionId()
                            + ". Event will not be sent to the subscribing client.");
                }
            }

        } else {
            logger.error("subscription {} has expired but eventOccurred has been called", subscriptionId);
            return;
        }

    }

    private boolean processFilterChain(PublicationInformation publicationInformation,
                                       List<BroadcastFilter> filters,
                                       Object[] values) {

        if (filters != null && filters.size() > 0) {
            BroadcastSubscriptionRequest subscriptionRequest = (BroadcastSubscriptionRequest) publicationInformation.subscriptionRequest;
            BroadcastFilterParameters filterParameters = subscriptionRequest.getFilterParameters();

            for (BroadcastFilter filter : filters) {
                Method filterMethod = null;
                try {
                    Method[] methodsOfFilterClass = filter.getClass().getMethods();
                    for (Method method : methodsOfFilterClass) {
                        if (method.getName().equals("filter")) {
                            filterMethod = method;
                            break;
                        }
                    }
                    if (filterMethod == null) {
                        // no filtering
                        return true;
                    }

                    if (!filterMethod.isAccessible()) {
                        filterMethod.setAccessible(true);
                    }

                    Class<?> filterParametersType = filterMethod.getParameterTypes()[values.length];
                    BroadcastFilterParameters filterParametersDerived = (BroadcastFilterParameters) filterParametersType.newInstance();
                    filterParametersDerived.setFilterParameters(filterParameters.getFilterParameters());
                    Object[] args = Arrays.copyOf(values, values.length + 1);
                    args[args.length - 1] = filterParametersDerived;

                    if ((Boolean) filterMethod.invoke(filter, args) == false) {
                        return false;
                    }
                } catch (Exception e) {
                    logger.error("processFilterChain error: {}", e.getMessage());
                    throw new IllegalStateException("processFilterChain: Error in reflection calling filters.", e);
                }
            }
        }
        return true;
    }

    private SubscriptionPublication prepareAttributePublication(Object value, String subscriptionId) {
        return new SubscriptionPublication(Arrays.asList(value), subscriptionId);
    }

    private SubscriptionPublication prepareBroadcastPublication(List<Object> values, String subscriptionId) {
        return new SubscriptionPublication(values, subscriptionId);
    }

    private void sendPublication(SubscriptionPublication publication, PublicationInformation publicationInformation) {
        try {
            sendSubscriptionPublication(publication, publicationInformation);
            // TODO handle exceptions during publication. See JOYNR-2113
        } catch (JoynrRuntimeException e) {
            logger.error("sendPublication error: {}", e.getMessage());
        } catch (JsonGenerationException e) {
            logger.error("sendPublication error: {}", e.getMessage());
        } catch (JsonMappingException e) {
            logger.error("sendPublication error: {}", e.getMessage());
        } catch (IOException e) {
            logger.error("sendPublication error: {}", e.getMessage());
        }
    }

    private void sendPublicationError(JoynrRuntimeException error, PublicationInformation publicationInformation) {
        SubscriptionPublication publication = new SubscriptionPublication(error,
                                                                          publicationInformation.getSubscriptionId());
        sendPublication(publication, publicationInformation);
    }

    private void triggerPublication(final PublicationInformation publicationInformation,
                                    RequestCaller requestCaller,
                                    Method method) {
        try {
            Promise<?> attributeGetterPromise = attributePollInterpreter.execute(requestCaller, method);
            attributeGetterPromise.then(new PromiseListener() {

                @Override
                public void onRejection(JoynrException error) {
                    if (error instanceof JoynrRuntimeException) {
                        sendPublicationError((JoynrRuntimeException) error, publicationInformation);
                    } else {
                        sendPublicationError(new ProviderRuntimeException("Unexpected exception while calling getter for attribute "
                                                     + publicationInformation.getSubscribedToName()),
                                             publicationInformation);
                    }

                }

                @Override
                public void onFulfillment(Object... values) {
                    // attribute getters only return a single value
                    sendPublication(prepareAttributePublication(values[0], publicationInformation.getSubscriptionId()),
                                    publicationInformation);
                }
            });
        } catch (JoynrRuntimeException error) {
            sendPublicationError(error, publicationInformation);
        }
    }

    private Method findGetterForAttributeName(Class<?> clazz, String attributeName) throws NoSuchMethodException {
        String attributeGetterName = "get" + attributeName.toUpperCase().charAt(0)
                + attributeName.subSequence(1, attributeName.length());
        return ReflectionUtils.findMethodByParamTypes(clazz, attributeGetterName, new Class[]{});

    }

    @Override
    public void sendSubscriptionPublication(SubscriptionPublication publication,
                                            PublicationInformation publicationInformation)
                                                                                          throws JoynrSendBufferFullException,
                                                                                          JoynrMessageNotSentException,
                                                                                          JsonGenerationException,
                                                                                          JsonMappingException,
                                                                                          IOException {
        MessagingQos messagingQos = new MessagingQos();
        messagingQos.setTtl_ms(publicationInformation.subscriptionRequest.getQos().getPublicationTtl());
        dispatcher.sendSubscriptionPublication(publicationInformation.providerParticipantId,
                                               publicationInformation.proxyParticipantId,
                                               publication,
                                               messagingQos);
        publicationInformation.getState().updateTimeOfLastPublication();
    }

    @Override
    public void callerAdded(String providerParticipantId, RequestCaller requestCaller) {
        restoreQueuedSubscription(providerParticipantId, requestCaller);
    }

    @Override
    public void callerRemoved(String providerParticipantId) {
        stopPublicationByProviderId(providerParticipantId);
    }

    @Override
    public void shutdown() {
        requestCallerDirectory.removeListener(this);
    }
}
