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
#include "cluster-controller/http-communication-manager/HttpReceiver.h"

#include "joynr/Util.h"
#include "cluster-controller/httpnetworking/HttpNetworking.h"
#include "cluster-controller/http-communication-manager/LongPollingMessageReceiver.h"
#include "cluster-controller/httpnetworking/HttpResult.h"
#include "cluster-controller/messaging/MessagingPropertiesPersistence.h"
#include "joynr/Future.h"
#include "joynr/TypeUtil.h"

namespace joynr
{

INIT_LOGGER(HttpReceiver);

HttpReceiver::HttpReceiver(const MessagingSettings& settings,
                           std::shared_ptr<MessageRouter> messageRouter)
        : channelCreatedSemaphore(new Semaphore(0)),
          channelId(),
          receiverId(),
          settings(settings),
          messageReceiver(nullptr),
          channelUrlDirectory(),
          messageRouter(messageRouter)
{
    MessagingPropertiesPersistence persist(settings.getMessagingPropertiesPersistenceFilename());
    channelId = persist.getChannelId();
    receiverId = persist.getReceiverId();
    init();

    // Remove any existing curl handles
    HttpNetworking::getInstance()->getCurlHandlePool()->reset();
}

void HttpReceiver::init()
{
    JOYNR_LOG_DEBUG(logger, "Print settings... ");
    settings.printSettings();
    updateSettings();
    JOYNR_LOG_DEBUG(logger, "Init finished.");
}

void HttpReceiver::init(std::shared_ptr<ILocalChannelUrlDirectory> channelUrlDirectory)
{
    this->channelUrlDirectory = channelUrlDirectory;
}

void HttpReceiver::updateSettings()
{
    // Setup the proxy to use
    if (settings.getLocalProxyHost().empty()) {
        HttpNetworking::getInstance()->setGlobalProxy(std::string());
    } else {
        HttpNetworking::getInstance()->setGlobalProxy(settings.getLocalProxyHost() + ":" +
                                                      settings.getLocalProxyPort());
    }

    // Turn on HTTP debug
    if (settings.getHttpDebug()) {
        HttpNetworking::getInstance()->setHTTPDebugOn();
    }

    // Set the connect timeout
    HttpNetworking::getInstance()->setConnectTimeout(
            std::chrono::milliseconds(settings.getHttpConnectTimeout()));

    // HTTPS settings
    HttpNetworking::getInstance()->setCertificateAuthority(settings.getCertificateAuthority());
    HttpNetworking::getInstance()->setClientCertificate(settings.getClientCertificate());
    HttpNetworking::getInstance()->setClientCertificatePassword(
            settings.getClientCertificatePassword());
}

HttpReceiver::~HttpReceiver()
{
    JOYNR_LOG_TRACE(logger, "destructing HttpCommunicationManager");
}

void HttpReceiver::startReceiveQueue()
{

    if (!messageRouter || !channelUrlDirectory) {
        JOYNR_LOG_FATAL(
                logger, "FAIL::receiveQueue started with no messageRouter/channelUrlDirectory.");
    }

    // Get the settings specific to long polling
    LongPollingMessageReceiverSettings longPollSettings = {
            std::chrono::milliseconds(settings.getBounceProxyTimeout()),
            std::chrono::milliseconds(settings.getLongPollTimeout()),
            std::chrono::milliseconds(settings.getLongPollRetryInterval()),
            std::chrono::milliseconds(settings.getCreateChannelRetryInterval())};

    JOYNR_LOG_DEBUG(logger, "startReceiveQueue");
    messageReceiver = new LongPollingMessageReceiver(settings.getBounceProxyUrl(),
                                                     channelId,
                                                     receiverId,
                                                     longPollSettings,
                                                     channelCreatedSemaphore,
                                                     channelUrlDirectory,
                                                     messageRouter);
    messageReceiver->start();
}

void HttpReceiver::waitForReceiveQueueStarted()
{
    JOYNR_LOG_TRACE(logger, "waiting for ReceiveQueue to be started.");
    channelCreatedSemaphore->wait();
    channelCreatedSemaphore->notify();
}

void HttpReceiver::stopReceiveQueue()
{
    // currently channelCreatedSemaphore is not released here. This would be necessary if
    // stopReceivequeue is called, before channel is created.
    JOYNR_LOG_DEBUG(logger, "stopReceiveQueue");
    if (messageReceiver != nullptr) {
        messageReceiver->stop();

        delete messageReceiver;
        messageReceiver = nullptr;
    }
}

const std::string& HttpReceiver::getReceiveChannelId() const
{
    return channelId;
}

bool HttpReceiver::tryToDeleteChannel()
{
    // If more than one attempt is needed, create a deleteChannelRunnable and move this to
    // messageSender.
    // TODO channelUrl is known only to the LongPlooMessageReceiver!
    std::string deleteChannelUrl =
            settings.getBounceProxyUrl().getDeleteChannelUrl(getReceiveChannelId()).toString();
    std::shared_ptr<IHttpDeleteBuilder> deleteChannelRequestBuilder(
            HttpNetworking::getInstance()->createHttpDeleteBuilder(deleteChannelUrl));
    std::shared_ptr<HttpRequest> deleteChannelRequest(
            deleteChannelRequestBuilder->withTimeout(std::chrono::seconds(20))->build());
    JOYNR_LOG_DEBUG(logger, "sending delete channel request to {}", deleteChannelUrl);
    HttpResult deleteChannelResult = deleteChannelRequest->execute();
    std::int32_t statusCode = deleteChannelResult.getStatusCode();
    if (statusCode == 200) {
        channelCreatedSemaphore->waitFor(
                std::chrono::seconds(5)); // Reset the channel created Semaphore.
        JOYNR_LOG_INFO(logger, "channel deletion successfull");
        channelUrlDirectory->unregisterChannelUrlsAsync(channelId);
        JOYNR_LOG_INFO(logger, "Sendeing unregister request to ChannelUrlDirectory ...");

        return true;
    } else if (statusCode == 204) {
        JOYNR_LOG_INFO(logger, "channel did not exist: {}", statusCode);
        return true;
    } else {
        JOYNR_LOG_INFO(logger,
                       "channel deletion failed with status code: {}",
                       deleteChannelResult.getStatusCode());
        return false;
    }
}

} // namespace joynr
