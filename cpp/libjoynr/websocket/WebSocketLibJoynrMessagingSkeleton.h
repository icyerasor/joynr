/*
 * #%L
 * %%
 * Copyright (C) 2011 - 2014 BMW Car IT GmbH
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
#ifndef WEBSOCKETLIBJOYNRMESSAGINGSKELETON_H
#define WEBSOCKETLIBJOYNRMESSAGINGSKELETON_H

#include "joynr/PrivateCopyAssign.h"
#include "joynr/Logger.h"

#include "joynr/IMessaging.h"
#include "joynr/MessageRouter.h"

#include "IWebSocketEventHandler.h"

#include <string>

namespace joynr
{

class WebSocketLibJoynrMessagingSkeleton : public IMessaging
{
public:
    explicit WebSocketLibJoynrMessagingSkeleton(MessageRouter& messageRouter);

    ~WebSocketLibJoynrMessagingSkeleton() override = default;

    void transmit(JoynrMessage& message) override;

    void onTextMessageReceived(const std::string& message);

private:
    DISALLOW_COPY_AND_ASSIGN(WebSocketLibJoynrMessagingSkeleton);
    ADD_LOGGER(WebSocketLibJoynrMessagingSkeleton);
    MessageRouter& messageRouter;
};

} // namespace joynr
#endif // WEBSOCKETLIBJOYNRMESSAGINGSKELETON_H
