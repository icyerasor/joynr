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
#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "joynr/PrivateCopyAssign.h"

#include "joynr/MessageQueue.h"
#include "joynr/Timer.h"

#include <chrono>
#include <cstdint>

using namespace joynr;

class MessageQueueTest : public ::testing::Test {
public:
    MessageQueueTest()
        : messageQueue(),
          cleanerTimer(),
          expiryDate(std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()) + std::chrono::milliseconds(100))
    {
    }

    ~MessageQueueTest() = default;

    void SetUp()
    {
        cleanerTimer.addTimer(
            [this](joynr::Timer::TimerId) {
                this->messageQueue.removeOutdatedMessages();
            },
            [](Timer::TimerId) { },
            50,
            true
        );
    }

    void TearDown()
    {
        cleanerTimer.shutdown();
    }


protected:
    MessageQueue messageQueue;
    Timer cleanerTimer;
    JoynrTimePoint expiryDate;

private:
    DISALLOW_COPY_AND_ASSIGN(MessageQueueTest);
};

TEST_F(MessageQueueTest, initialQueueIsEmpty) {
    EXPECT_EQ(messageQueue.getQueueLength(), 0);
}

TEST_F(MessageQueueTest, addMultipleMessages) {
    JoynrMessage msg1;
    msg1.setHeaderExpiryDate(expiryDate);
    EXPECT_EQ(messageQueue.queueMessage(msg1), 1);
    JoynrMessage msg2;
    msg2.setHeaderExpiryDate(expiryDate);
    EXPECT_EQ(messageQueue.queueMessage(msg2), 2);
    JoynrMessage msg3;
    msg3.setHeaderExpiryDate(expiryDate);
    EXPECT_EQ(messageQueue.queueMessage(msg3), 3);
    JoynrMessage msg4;
    msg4.setHeaderExpiryDate(expiryDate);
    EXPECT_EQ(messageQueue.queueMessage(msg4), 4);
}

TEST_F(MessageQueueTest, queueDequeueMessages) {
    // add messages to the queue
    JoynrMessage msg1;
    msg1.setHeaderTo("TEST1");
    msg1.setHeaderExpiryDate(expiryDate);
    messageQueue.queueMessage(msg1);

    JoynrMessage msg2;
    msg2.setHeaderTo("TEST2");
    msg2.setHeaderExpiryDate(expiryDate);
    messageQueue.queueMessage(msg2);
    EXPECT_EQ(messageQueue.getQueueLength(), 2);

    // get messages from queue
    MessageQueueItem* item = messageQueue.getNextMessageForParticipant("TEST1");
    EXPECT_EQ(item->getContent(), msg1);
    EXPECT_EQ(messageQueue.getQueueLength(), 1);

    item = messageQueue.getNextMessageForParticipant("TEST2");
    EXPECT_EQ(item->getContent(), msg2);
    EXPECT_EQ(messageQueue.getQueueLength(), 0);
}

TEST_F(MessageQueueTest, queueDequeueMultipleMessagesForOneParticipant) {
    // add messages to the queue
    JoynrMessage msg;
    msg.setHeaderTo("TEST");
    msg.setHeaderExpiryDate(expiryDate);
    messageQueue.queueMessage(msg);
    messageQueue.queueMessage(msg);
    EXPECT_EQ(messageQueue.getQueueLength(), 2);

    // get messages from queue
    MessageQueueItem* item = messageQueue.getNextMessageForParticipant("TEST");
    EXPECT_EQ(item->getContent(), msg);
    EXPECT_EQ(messageQueue.getQueueLength(), 1);

    item = messageQueue.getNextMessageForParticipant("TEST");
    EXPECT_EQ(item->getContent(), msg);
    EXPECT_EQ(messageQueue.getQueueLength(), 0);
}

TEST_F(MessageQueueTest, dequeueInvalidParticipantId) {
    EXPECT_FALSE(messageQueue.getNextMessageForParticipant("TEST"));
}

TEST_F(MessageQueueTest, removeOutdatedMessage) {
    JoynrMessage msg10;
    JoynrTimePoint now = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
    msg10.setHeaderExpiryDate(now + std::chrono::milliseconds(10));
    EXPECT_EQ(messageQueue.queueMessage(msg10), 1);
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    EXPECT_EQ(messageQueue.removeOutdatedMessages(), 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(6));
    EXPECT_EQ(messageQueue.removeOutdatedMessages(), 1);
}

TEST_F(MessageQueueTest, removeOutdatedMessagesWithRunnable) {
    JoynrMessage msg25;
    JoynrTimePoint now = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
    msg25.setHeaderExpiryDate(now + std::chrono::milliseconds(25));
    JoynrMessage msg250;
    msg250.setHeaderExpiryDate(now + std::chrono::milliseconds(250));
    JoynrMessage msg300;
    msg300.setHeaderExpiryDate(now + std::chrono::milliseconds(250));
    EXPECT_EQ(messageQueue.queueMessage(msg25), 1);
    EXPECT_EQ(messageQueue.queueMessage(msg250), 2);
    EXPECT_EQ(messageQueue.queueMessage(msg300), 3);

    // wait to remove the first message
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_EQ(messageQueue.getQueueLength(), 2);

    // wait to remove all messages
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    EXPECT_EQ(messageQueue.getQueueLength(), 0);
}
