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
#include "joynr/PrivateCopyAssign.h"
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "joynr/MessageRouter.h"
#include "tests/utils/MockObjects.h"
#include "joynr/system/RoutingTypes/ChannelAddress.h"
#include "joynr/MessagingStubFactory.h"
#include "joynr/MessageQueue.h"
#include "libjoynr/in-process/InProcessMessagingStubFactory.h"
#include <chrono>
#include <cstdint>

using namespace joynr;

class MessageRouterTest : public ::testing::Test {
public:
    MessageRouterTest() :
        settingsFileName("MessageRouterTest.settings"),
        settings(settingsFileName),
        messagingSettings(settings),
        messagingStubFactory(new MockMessagingStubFactory()),
        messageQueue(new MessageQueue()),
        messageRouter(new MessageRouter(new MessagingStubFactory(), nullptr, 6, messageQueue)),
        joynrMessage()
    {
        // provision global capabilities directory
        std::shared_ptr<joynr::system::RoutingTypes::Address> addressCapabilitiesDirectory(
            new system::RoutingTypes::ChannelAddress(
                        messagingSettings.getCapabilitiesDirectoryChannelId())
        );
        messageRouter->addProvisionedNextHop(messagingSettings.getCapabilitiesDirectoryParticipantId(), addressCapabilitiesDirectory);
        // provision channel url directory
        std::shared_ptr<joynr::system::RoutingTypes::Address> addressChannelUrlDirectory(
            new system::RoutingTypes::ChannelAddress(
                        messagingSettings.getChannelUrlDirectoryChannelId())
        );
        messageRouter->addProvisionedNextHop(messagingSettings.getChannelUrlDirectoryParticipantId(), addressChannelUrlDirectory);
        JoynrTimePoint now = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
        joynrMessage.setHeaderExpiryDate(now + std::chrono::milliseconds(100));
    }

    ~MessageRouterTest() {
        std::remove(settingsFileName.c_str());
    }

    void SetUp(){
        joynrMessage.setType(JoynrMessage::VALUE_MESSAGE_TYPE_ONE_WAY);
    }
    void TearDown(){
    }
protected:
    std::string settingsFileName;
    Settings settings;
    MessagingSettings messagingSettings;
    MockMessagingStubFactory* messagingStubFactory;
    MessageQueue* messageQueue;
    MessageRouter* messageRouter;
    JoynrMessage joynrMessage;
private:
    DISALLOW_COPY_AND_ASSIGN(MessageRouterTest);
};

TEST_F(MessageRouterTest, DISABLED_routeDelegatesToStubFactory){
    // cH: this thest doesn't make sense anymore, since the MessageRouter
    // will create the MessagingStubFactory internally and therefor couldn't
    // be mocked. However, this test was already disabled.
    EXPECT_CALL(*messagingStubFactory, create(_)).Times(1);

    messageRouter->route(joynrMessage);

}

TEST_F(MessageRouterTest, addMessageToQueue){
    messageRouter->route(joynrMessage);
    EXPECT_EQ(messageQueue->getQueueLength(), 1);

    messageRouter->route(joynrMessage);
    EXPECT_EQ(messageQueue->getQueueLength(), 2);
}

TEST_F(MessageRouterTest, doNotAddMessageToQueue){
    // this message should be added because no destination header set
    messageRouter->route(joynrMessage);
    EXPECT_EQ(messageQueue->getQueueLength(), 1);

    // the message now has a known destination and should be directly routed
    joynrMessage.setHeaderTo(messagingSettings.getCapabilitiesDirectoryParticipantId());
    messageRouter->route(joynrMessage);
    EXPECT_EQ(messageQueue->getQueueLength(), 1);
}

TEST_F(MessageRouterTest, resendMessageWhenDestinationAddressIsAdded){
    // this message should be added because no destination header set
    joynrMessage.setHeaderTo("TEST");
    messageRouter->route(joynrMessage);
    EXPECT_EQ(messageQueue->getQueueLength(), 1);

    // add destination address -> message should be routed
    std::shared_ptr<system::RoutingTypes::ChannelAddress> address(new system::RoutingTypes::ChannelAddress("TEST"));
    messageRouter->addNextHop("TEST", address);
    EXPECT_EQ(messageQueue->getQueueLength(), 0);
}

TEST_F(MessageRouterTest, outdatedMessagesAreRemoved){
    messageRouter->route(joynrMessage);
    EXPECT_EQ(messageQueue->getQueueLength(), 1);

    // we wait for the time out (500ms) and the thread sleep (1000ms)
    std::this_thread::sleep_for(std::chrono::milliseconds(1200));
    EXPECT_EQ(messageQueue->getQueueLength(), 0);
}
