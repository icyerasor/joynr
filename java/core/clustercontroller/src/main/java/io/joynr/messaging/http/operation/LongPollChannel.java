package io.joynr.messaging.http.operation;

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

import io.joynr.exceptions.JoynrChannelMissingException;
import io.joynr.exceptions.JoynrCommunicationException;
import io.joynr.exceptions.JoynrShutdownException;
import io.joynr.messaging.MessageArrivedListener;
import io.joynr.messaging.MessagingSettings;
import io.joynr.messaging.datatypes.JoynrMessagingError;
import io.joynr.messaging.datatypes.JoynrMessagingErrorCode;
import io.joynr.messaging.util.Utilities;

import java.io.IOException;
import java.net.SocketException;
import java.net.URI;
import java.text.SimpleDateFormat;
import java.util.List;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.ThreadFactory;
import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

import joynr.JoynrMessage;

import org.apache.http.HttpEntity;
import org.apache.http.HttpResponse;
import org.apache.http.HttpStatus;
import org.apache.http.client.ClientProtocolException;
import org.apache.http.client.ResponseHandler;
import org.apache.http.client.config.RequestConfig;
import org.apache.http.client.config.RequestConfig.Builder;
import org.apache.http.impl.client.CloseableHttpClient;
import org.apache.http.util.EntityUtils;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.fasterxml.jackson.core.JsonParseException;
import com.fasterxml.jackson.databind.JsonMappingException;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.google.common.util.concurrent.ThreadFactoryBuilder;

/**
 * Callable to keep a long polling channel alive and to process incoming messages.
 */

public class LongPollChannel {
    private static final Logger logger = LoggerFactory.getLogger(LongPollChannel.class);
    final SimpleDateFormat format = new SimpleDateFormat("EEE, d MMM yyyy HH:mm:ss.SSS");

    public static final long LONGPOLLING_RETRY_INTERVAL_SECS = 5L;

    private CloseableHttpClient httpclient;
    private boolean shutdown = false;
    private MessageArrivedListener messageArrivedListener;
    private final ObjectMapper objectMapper;

    private String id = "";

    private Lock statusLock = new ReentrantLock();
    private Condition statusChanged = statusLock.newCondition();

    private Boolean longPollingDisabled;
    private MessagingSettings settings;
    private HttpConstants httpConstants;
    private String receiverId;
    private HttpGet httpget;
    protected int statusCode;
    private String statusText;
    private RequestConfig defaultRequestConfig;
    private HttpRequestFactory httpRequestFactory;
    private ExecutorService messageReceiverExecutor;

    // CHECKSTYLE:OFF
    public LongPollChannel(CloseableHttpClient httpclient,
                           RequestConfig defaultRequestConfig,
                           Boolean longPollingDisabled,
                           MessageArrivedListener messageArrivedListener,
                           ObjectMapper objectMapper,
                           MessagingSettings settings,
                           HttpConstants httpConstants,
                           String channelId,
                           String receiverId,
                           HttpRequestFactory httpRequestFactory) {
        // CHECKSTYLE:ON
        this.httpclient = httpclient;
        this.defaultRequestConfig = defaultRequestConfig;
        this.longPollingDisabled = longPollingDisabled;
        this.messageArrivedListener = messageArrivedListener;
        this.objectMapper = objectMapper;
        this.settings = settings;
        this.httpConstants = httpConstants;
        this.receiverId = receiverId;
        this.httpRequestFactory = httpRequestFactory;
        ThreadFactory namedThreadFactory = new ThreadFactoryBuilder().setNameFormat("joynr.LongPollChannel-%d").build();
        messageReceiverExecutor = Executors.newCachedThreadPool(namedThreadFactory);
    }

    /**
     * Start long polling loop throws ExecutionExeption in order to terminate channel properly returns Void (null) in
     * order to reconnect channel
     */
    public Void longPollLoop() throws JoynrShutdownException {
        logger.debug("LongPollingChannel OPENING CHANNEL: {} ", id);
        try {
            while (true) {
                // start long poll if it has not been disabled.
                // Checking the disable switch and request creation has
                // to be an atomic process.
                // When the request creation is done it can be canceled
                // by a call to suspend.
                if (shutdown) {
                    throw new JoynrShutdownException("shutting down");
                }

                if (longPollingDisabled == true) {
                    try {
                        statusLock.lockInterruptibly();
                    } catch (InterruptedException e) {
                        throw new JoynrShutdownException("INTERRUPTED. Shutting down");
                    }
                    logger.trace("Waiting for long polling to be resumed.");
                    statusChanged.awaitUninterruptibly();

                } else {
                    longPoll();
                    try {

                        // prevent error when long polling connection was reestablished too soon after repsone being
                        // returned
                        Thread.sleep(10);
                    } catch (InterruptedException e) {
                        throw new JoynrShutdownException("INTERRUPTED. Shutting down");
                    }
                }

            }
        } finally {
            // shutdown();
            logger.info("LongPollingChannel CHANNEL: " + id + " long poll loop exited");
        }
    }

    private void longPoll() {
        String responseBody = null;
        if (shutdown) {
            return;
        }

        try {

            responseBody = httpclient.execute(httpget, new ResponseHandler<String>() {

                @Override
                public String handleResponse(HttpResponse response) throws ClientProtocolException, IOException {
                    HttpEntity entity = response.getEntity();
                    String body = entity == null ? null : EntityUtils.toString(entity, "UTF-8");
                    statusCode = response.getStatusLine().getStatusCode();
                    statusText = response.getStatusLine().getReasonPhrase();
                    logger.debug("Long poll returned: {} reason: url {}", statusCode, httpget.getURI().toASCIIString());
                    return body;
                }
            });
        } catch (IllegalStateException e) {
            logger.error("IllegalStateException in long poll: {} message: {}",
                         httpget.getURI().toASCIIString(),
                         e.getMessage());
            throw new JoynrShutdownException(e.getMessage());
        } catch (ClientProtocolException e) {
            logger.error("ClientProtocolException in long poll: {} message: {}",
                         httpget.getURI().toASCIIString(),
                         e.getMessage());
            delay();
            return;
        } catch (SocketException e) {
            logger.warn("SocketException in long poll: {} message: {} message: {}",
                        httpget.getURI().toASCIIString(),
                        e.getMessage());
            delay();
            return;

        } catch (IOException e) {
            logger.warn("IOException in long poll: {} message: {}", httpget.getURI().toASCIIString(), e.getMessage());
            delay();
            return;
        } catch (Exception e) {
            logger.warn("Exception in long poll: {} message: {}", httpget.getURI().toASCIIString(), e.getMessage());
            delay();
            return;
        }

        switch (statusCode) {
        case HttpStatus.SC_OK:
            notifyDispatcher(responseBody);
            break;
        case HttpStatus.SC_NOT_FOUND:
            logger.error(responseBody);
            delay();
            throw new JoynrChannelMissingException("Not found");

        case HttpStatus.SC_BAD_REQUEST:
            if (responseBody != null) {
                try {
                    JoynrMessagingError error = objectMapper.readValue(responseBody, JoynrMessagingError.class);
                    JoynrMessagingErrorCode joynrMessagingErrorCode = JoynrMessagingErrorCode.getJoynrMessagingErrorCode(error.getCode());
                    logger.error(error.toString());
                    switch (joynrMessagingErrorCode) {
                    case JOYNRMESSAGINGERROR_CHANNELNOTFOUND:
                        throw new JoynrChannelMissingException(error.getReason());
                    default:
                        throw new JoynrCommunicationException(error.getReason());
                    }
                } catch (IOException e) {
                    throw new JoynrCommunicationException(statusText);
                }
            }

        default:
            delay();
            break;
        }

    }

    private void notifyDispatcher(String responseBody) {
        if (responseBody == null || responseBody.length() <= 0) {
            return;
        }

        // the response body could contain multiple json objects
        List<String> listOfJsonStrings = Utilities.splitJson(responseBody);
        logger.info("LongPollingChannel CHANNEL: {} messages received: {}", listOfJsonStrings.size());

        // Tries to parse each message as a JoynrMessage or MessageWrapper
        for (String json : listOfJsonStrings) {
            try {
                final JoynrMessage message = objectMapper.readValue(json, JoynrMessage.class); // jsonConverter.fromJson(json,
                // MessageWrapper.class);
                if (message != null) {
                    messageReceiverExecutor.execute(new Runnable() {

                        @Override
                        public void run() {
                            logger.info("ARRIVED {} messageId: {} type: {} from: {} to: {} header: {}", new String[]{
                                    httpget.getURI().toString(), message.getId(), message.getType(),
                                    message.getHeaderValue(JoynrMessage.HEADER_NAME_FROM_PARTICIPANT_ID),
                                    message.getHeaderValue(JoynrMessage.HEADER_NAME_TO_PARTICIPANT_ID),
                                    message.getHeader().toString() });
                            logger.debug("\r\n<<<<<<<<<<<<<<<<<\r\n:{}", message.toLogMessage());
                            messageArrivedListener.messageArrived(message);
                        }
                    });

                } else {
                    logger.warn("LongPollingChannel CHANNEL: {} message was null", id);
                    messageArrivedListener.error(null,
                                                 new JoynrCommunicationException("LongPollingChannel CHANNEL: {} message was null"));
                }
                continue;
            } catch (JsonParseException e) {
            } catch (JsonMappingException e) {
            } catch (IOException e) {
            }

            logger.error("CHANNEL: {} Error converting the JSON into a Message object, message could not be passed to dispatcher. \n"
                                 + "Response body was: " + json,
                         id);
        }
    }

    public void shutdown() {
        logger.info("SHUTTING down long poll for {}", httpget.getURI().toASCIIString());
        shutdown = true;

        if (httpclient != null) {
            try {
                httpclient.close();
            } catch (IOException e) {
            }
        }
        logger.debug("LongPollingChannel CHANNEL: {} SHUT DOWN", id);
    }

    public void resume() {
        try {
            statusLock.lockInterruptibly();
            httpget.reset();
            setLongPollingDisabled(false);
            statusChanged.signal();
        } catch (InterruptedException e) {
            e.printStackTrace();
            return;
        } finally {
            statusLock.unlock();
        }
    }

    public void suspend() {
        try {

            statusLock.lockInterruptibly();
            setLongPollingDisabled(true);
            httpget.abort();
        } catch (InterruptedException e) {
            return;
        } finally {
            statusLock.unlock();
        }

    }

    public boolean isLongPollingDisabled() {
        return longPollingDisabled;
    }

    private void setLongPollingDisabled(boolean longPollingDisabled) {

        this.longPollingDisabled = longPollingDisabled;

    }

    public void setChannelUrl(String channelUrl) {
        this.httpget = httpRequestFactory.createHttpGet(URI.create(channelUrl));
        Builder requestConfigBuilder = RequestConfig.copy(defaultRequestConfig);
        httpget.setConfig(requestConfigBuilder.build());
        httpget.setHeader(httpConstants.getHEADER_X_ATMOSPHERE_TRACKING_ID(), receiverId);
        if (channelUrl.length() > 15) {
            this.id = "..." + channelUrl.substring(channelUrl.length() - 15);
        } else {
            this.id = channelUrl;
        }

    }

    private void delay() {
        try {
            Thread.sleep(settings.getLongPollRetryIntervalMs());
        } catch (InterruptedException e) {
            // exit
        }
    }
}
