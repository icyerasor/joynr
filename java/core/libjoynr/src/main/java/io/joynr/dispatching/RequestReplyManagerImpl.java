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
import io.joynr.common.ExpiryDate;
import io.joynr.dispatching.rpc.ReplyCaller;
import io.joynr.dispatching.rpc.ReplyCallerDirectory;
import io.joynr.dispatching.rpc.RequestInterpreter;
import io.joynr.dispatching.rpc.SynchronizedReplyCaller;
import io.joynr.exceptions.JoynrCommunicationException;
import io.joynr.exceptions.JoynrMessageNotSentException;
import io.joynr.exceptions.JoynrRequestInterruptedException;
import io.joynr.exceptions.JoynrSendBufferFullException;
import io.joynr.exceptions.JoynrShutdownException;
import io.joynr.messaging.routing.MessageRouter;
import io.joynr.provider.ProviderCallback;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;

import joynr.JoynrMessage;
import joynr.OneWay;
import joynr.Reply;
import joynr.Request;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.fasterxml.jackson.core.JsonGenerationException;
import com.fasterxml.jackson.databind.JsonMappingException;
import com.google.common.collect.Maps;
import com.google.inject.Inject;
import com.google.inject.Singleton;
import com.google.inject.name.Named;

@Singleton
public class RequestReplyManagerImpl implements RequestReplyManager, CallerDirectoryListener<RequestCaller> {
    private static final Logger logger = LoggerFactory.getLogger(RequestReplyManagerImpl.class);
    private boolean running = true;

    private List<Thread> outstandingRequestThreads = Collections.synchronizedList(new ArrayList<Thread>());
    private Map<String, PayloadListener<?>> oneWayRecipients = Maps.newHashMap();
    private ConcurrentHashMap<String, ConcurrentLinkedQueue<ContentWithExpiryDate<Request>>> requestQueue = new ConcurrentHashMap<String, ConcurrentLinkedQueue<ContentWithExpiryDate<Request>>>();
    private ConcurrentHashMap<String, ConcurrentLinkedQueue<ContentWithExpiryDate<OneWay>>> oneWayRequestQueue = new ConcurrentHashMap<String, ConcurrentLinkedQueue<ContentWithExpiryDate<OneWay>>>();
    private ConcurrentHashMap<Request, ProviderCallback<Reply>> replyCallbacks = new ConcurrentHashMap<Request, ProviderCallback<Reply>>();

    private ReplyCallerDirectory replyCallerDirectory;
    private RequestCallerDirectory requestCallerDirectory;
    private RequestInterpreter requestInterpreter;
    private MessageRouter messageRouter;
    private JoynrMessageFactory joynrMessageFactory;

    private ScheduledExecutorService cleanupScheduler;

    @Inject
    public RequestReplyManagerImpl(JoynrMessageFactory joynrMessageFactory,
                                   ReplyCallerDirectory replyCallerDirectory,
                                   RequestCallerDirectory requestCallerDirectory,
                                   MessageRouter messageRouter,
                                   RequestInterpreter requestInterpreter,
                                   @Named(JOYNR_SCHEDULER_CLEANUP) ScheduledExecutorService cleanupScheduler) {
        this.joynrMessageFactory = joynrMessageFactory;
        this.replyCallerDirectory = replyCallerDirectory;
        this.requestCallerDirectory = requestCallerDirectory;
        this.messageRouter = messageRouter;
        this.requestInterpreter = requestInterpreter;
        this.cleanupScheduler = cleanupScheduler;
        requestCallerDirectory.addListener(this);
    }

    /*
     * (non-Javadoc)
     *
     * @see io.joynr.dispatcher.MessageSender#sendRequest(java. lang.String, java.lang.String,
     * java.lang.Object, io.joynr.dispatcher.ReplyCaller, long, long)
     */

    @Override
    public void sendRequest(final String fromParticipantId, final String toParticipantId, Request request, long ttl_ms)
                                                                                                                       throws JoynrSendBufferFullException,
                                                                                                                       JoynrMessageNotSentException,
                                                                                                                       JsonGenerationException,
                                                                                                                       JsonMappingException,
                                                                                                                       IOException {

        logger.trace("SEND USING RequestReplySenderImpl with Id: " + System.identityHashCode(this));

        ExpiryDate expiryDate = DispatcherUtils.convertTtlToExpirationDate(ttl_ms);

        JoynrMessage message = joynrMessageFactory.createRequest(fromParticipantId,
                                                                 toParticipantId,
                                                                 request,
                                                                 expiryDate);

        messageRouter.route(message);

    }

    /*
     * (non-Javadoc)
     *
     * @see io.joynr.dispatcher.MessageSender#sendSyncRequest(java .lang.String, java.lang.String,
     * java.lang.Object, long, long)
     */

    @Override
    public Object sendSyncRequest(final String fromParticipantId,
                                  final String toParticipantId,
                                  Request request,
                                  SynchronizedReplyCaller synchronizedReplyCaller,
                                  long ttl_ms) throws JoynrCommunicationException, JoynrSendBufferFullException,
                                              JoynrMessageNotSentException, JsonGenerationException,
                                              JsonMappingException, IOException {

        if (!running) {
            throw new IllegalStateException("Request: " + request.getRequestReplyId() + " failed. SenderImpl ID: "
                    + System.identityHashCode(this) + ": joynr is shutting down");
        }

        final ArrayList<Object> responsePayloadContainer = new ArrayList<Object>(1);
        // the synchronizedReplyCaller will call notify on the responsePayloadContainer when a message arrives
        synchronizedReplyCaller.setResponseContainer(responsePayloadContainer);

        sendRequest(fromParticipantId, toParticipantId, request, ttl_ms);

        long entryTime = System.currentTimeMillis();

        // saving all calling threads so that they can be interrupted at shutdown
        outstandingRequestThreads.add(Thread.currentThread());
        synchronized (responsePayloadContainer) {
            while (running && responsePayloadContainer.isEmpty() && entryTime + ttl_ms > System.currentTimeMillis()) {
                try {
                    responsePayloadContainer.wait(ttl_ms);
                } catch (InterruptedException e) {
                    if (running) {
                        throw new JoynrRequestInterruptedException("Request: " + request.getRequestReplyId()
                                + " interrupted.");
                    }
                    throw new JoynrShutdownException("Request: " + request.getRequestReplyId()
                            + " interrupted by shutdown");

                }
            }
        }
        outstandingRequestThreads.remove(Thread.currentThread());

        if (responsePayloadContainer.isEmpty()) {
            throw new JoynrCommunicationException("Request: " + request.getRequestReplyId()
                    + " failed. The response didn't arrive in time");
        }

        Object response = responsePayloadContainer.get(0);
        if (response instanceof Throwable) {
            Throwable error = (Throwable) response;
            throw new JoynrMessageNotSentException("Request: " + request.getRequestReplyId() + " failed: "
                    + error.getMessage(), error);
        }

        return response;
    }

    /*
     * (non-Javadoc)
     *
     * @see io.joynr.dispatcher.MessageSender#sendOneWay(java.lang .String, java.lang.String,
     * java.lang.Object, long)
     */

    @Override
    public void sendOneWay(final String fromParticipantId, final String toParticipantId, Object payload, long ttl_ms)
                                                                                                                     throws JoynrSendBufferFullException,
                                                                                                                     JoynrMessageNotSentException,
                                                                                                                     JsonGenerationException,
                                                                                                                     JsonMappingException,
                                                                                                                     IOException {
        JoynrMessage message = joynrMessageFactory.createOneWay(fromParticipantId,
                                                                toParticipantId,
                                                                payload,
                                                                DispatcherUtils.convertTtlToExpirationDate(ttl_ms));

        messageRouter.route(message);

    }

    @Override
    public void sendReply(final String fromParticipantId,
                          final String toParticipantId,
                          Reply payload,
                          ExpiryDate expiryDate) throws JoynrSendBufferFullException, JoynrMessageNotSentException,
                                                JsonGenerationException, JsonMappingException, IOException {
        JoynrMessage message = joynrMessageFactory.createReply(fromParticipantId, toParticipantId, payload, expiryDate);

        messageRouter.route(message);
    }

    @Override
    public void addOneWayRecipient(final String participantId, PayloadListener<?> listener) {
        synchronized (oneWayRecipients) {
            oneWayRecipients.put(participantId, listener);
        }

        ConcurrentLinkedQueue<ContentWithExpiryDate<OneWay>> oneWayRequestList = oneWayRequestQueue.remove(participantId);
        if (oneWayRequestList != null) {
            for (ContentWithExpiryDate<OneWay> oneWayRequestItem : oneWayRequestList) {
                if (!oneWayRequestItem.isExpired()) {
                    handleOneWayRequest(listener, oneWayRequestItem.getContent());
                }
            }
        }
    }

    @Override
    public void callerAdded(String participantId, RequestCaller requestCaller) {
        ConcurrentLinkedQueue<ContentWithExpiryDate<Request>> requestList = requestQueue.remove(participantId);
        if (requestList != null) {
            for (ContentWithExpiryDate<Request> requestItem : requestList) {
                if (!requestItem.isExpired()) {
                    Request request = requestItem.getContent();
                    handleRequest(replyCallbacks.remove(request), requestCaller, request);
                }
            }
        }
    }

    @Override
    public void callerRemoved(String participantId) {
        //TODO cleanup requestQueue?
    }

    @Override
    public void removeListener(final String participantId) {
        synchronized (oneWayRecipients) {
            oneWayRecipients.remove(participantId);
        }
    }

    @Override
    public void handleOneWayRequest(String providerParticipantId, OneWay requestPayload, long expiryDate) {
        synchronized (oneWayRecipients) {
            final PayloadListener<?> listener = oneWayRecipients.get(providerParticipantId);
            if (listener != null) {
                handleOneWayRequest(listener, requestPayload);
            } else {
                queueOneWayRequest(providerParticipantId, requestPayload, ExpiryDate.fromAbsolute(expiryDate));
            }
        }
    }

    @SuppressWarnings({ "unchecked", "rawtypes" })
    private void handleOneWayRequest(final PayloadListener listener, final OneWay requestPayload) {
        listener.receive(requestPayload.getPayload());
    }

    @Override
    public void handleRequest(ProviderCallback<Reply> replyCallback,
                              String providerParticipant,
                              Request request,
                              long expiryDate) {
        if (requestCallerDirectory.containsCaller(providerParticipant)) {
            handleRequest(replyCallback, requestCallerDirectory.getCaller(providerParticipant), request);
        } else {
            queueRequest(replyCallback, providerParticipant, request, ExpiryDate.fromAbsolute(expiryDate));
            logger.info("No requestCaller found for participantId: {} queuing request message.", providerParticipant);
        }
    }

    private void handleRequest(ProviderCallback<Reply> replyCallback, RequestCaller requestCaller, Request request) {
        // TODO shall be moved to request manager and not handled by dispatcher
        logger.debug("executing request {}", request.getRequestReplyId());
        requestInterpreter.execute(replyCallback, requestCaller, request);
    }

    @Override
    public void handleReply(final Reply reply) {
        final ReplyCaller callBack = replyCallerDirectory.removeCaller(reply.getRequestReplyId());
        if (callBack == null) {
            logger.warn("No reply caller found for id: " + reply.getRequestReplyId());
            return;
        }

        callBack.messageCallBack(reply);
    }

    @Override
    public void handleError(Request request, Throwable error) {
        String requestReplyId = request.getRequestReplyId();
        if (requestReplyId != null) {
            ReplyCaller replyCaller = replyCallerDirectory.removeCaller(requestReplyId);
            if (replyCaller != null) {
                replyCaller.error(error);
            }
        }
    }

    private void queueRequest(final ProviderCallback<Reply> replyCallback,
                              final String providerParticipantId,
                              Request request,
                              ExpiryDate incomingTtlExpirationDate_ms) {

        if (!requestQueue.containsKey(providerParticipantId)) {
            ConcurrentLinkedQueue<ContentWithExpiryDate<Request>> newRequestList = new ConcurrentLinkedQueue<ContentWithExpiryDate<Request>>();
            requestQueue.putIfAbsent(providerParticipantId, newRequestList);
        }
        final ContentWithExpiryDate<Request> requestItem = new ContentWithExpiryDate<Request>(request,
                                                                                              incomingTtlExpirationDate_ms);
        requestQueue.get(providerParticipantId).add(requestItem);
        replyCallbacks.put(request, replyCallback);
        cleanupScheduler.schedule(new Runnable() {

            @Override
            public void run() {
                requestQueue.get(providerParticipantId).remove(requestItem);
                replyCallbacks.remove(requestItem.getContent());
                Request request = requestItem.getContent();
                logger.warn("TTL DISCARD. providerParticipantId: {} request method: {} because it has expired. ",
                            new String[]{ providerParticipantId, request.getMethodName() });

            }
        }, incomingTtlExpirationDate_ms.getRelativeTtl(), TimeUnit.MILLISECONDS);
    }

    private void queueOneWayRequest(final String providerParticipantId,
                                    OneWay request,
                                    ExpiryDate incomingTtlExpirationDate_ms) {

        if (!oneWayRequestQueue.containsKey(providerParticipantId)) {
            ConcurrentLinkedQueue<ContentWithExpiryDate<OneWay>> newOneWayRequestList = new ConcurrentLinkedQueue<ContentWithExpiryDate<OneWay>>();
            oneWayRequestQueue.putIfAbsent(providerParticipantId, newOneWayRequestList);
        }
        final ContentWithExpiryDate<OneWay> oneWayRequestItem = new ContentWithExpiryDate<OneWay>(request,
                                                                                                  incomingTtlExpirationDate_ms);
        oneWayRequestQueue.get(providerParticipantId).add(oneWayRequestItem);
        cleanupScheduler.schedule(new Runnable() {

            @Override
            public void run() {
                oneWayRequestQueue.get(providerParticipantId).remove(oneWayRequestItem);
                logger.warn("TTL DISCARD. providerParticipantId: {} one way request with payload: {} because it has expired. ",
                            new String[]{ providerParticipantId, oneWayRequestItem.getContent().toString() });

            }
        },
                                  incomingTtlExpirationDate_ms.getRelativeTtl(),
                                  TimeUnit.MILLISECONDS);
    }

    @Override
    public void shutdown() {
        running = false;
        synchronized (outstandingRequestThreads) {
            for (Thread thread : outstandingRequestThreads) {
                logger.debug("shutting down. Interrupting thread: " + thread);
                thread.interrupt();
            }
        }
        messageRouter.shutdown();
        requestCallerDirectory.removeListener(this);
        replyCallerDirectory.shutdown();
    }
}
