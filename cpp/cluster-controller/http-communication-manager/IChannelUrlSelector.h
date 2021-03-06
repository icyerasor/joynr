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
#ifndef ICHANNELURLSELECTOR_H
#define ICHANNELURLSELECTOR_H

#include "joynr/ILocalChannelUrlDirectory.h"
#include "joynr/StatusCode.h"
#include <memory>
#include <string>
#include <chrono>

namespace joynr
{

class MessagingSettings;

/**
 * @brief
 *
 */
class IChannelUrlSelector
{

public:
    virtual ~IChannelUrlSelector() = default;

    /**
    * @brief Uses the ChannelUrlDirectoryProxy to query the remote ChannelUrlDirectory
    *
    * @param channelUrlDirectoryProxy
    */
    virtual void init(std::shared_ptr<ILocalChannelUrlDirectory> channelUrlDirectory,
                      const MessagingSettings& settings) = 0;

    /**
    * @brief Get the "best" URL for this channel. Feedback is used to figure out which
    * URL is currently best depending on recent availability and initial ordering (eg direct before
    * bounceproxy URL.
    *
    * @param channelId
    * @param status
    * @param timeout
    * @return std::string
    */
    virtual std::string obtainUrl(const std::string& channelId,
                                  StatusCodeEnum& status,
                                  std::chrono::milliseconds timeout) = 0;
    /**
    * @brief Provide feedback on performance of URL: was the connection successful or not?
    *
    * @param success
    * @param channelId
    * @param url
    */
    virtual void feedback(bool success, const std::string& channelId, std::string url) = 0;
};

} // namespace joynr
#endif // ICHANNELURLSELECTOR_H
