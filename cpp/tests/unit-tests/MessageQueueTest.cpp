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
#include "joynr/PrivateCopyAssign.h"
#include "gtest/gtest.h"
#include <QFile>
#include "gmock/gmock.h"
#include "joynr/MessageQueue.h"
#include "QThread"
#include <QThreadPool>


using namespace joynr;

class MessageQueueTest : public ::testing::Test {
public:
    MessageQueueTest():
        messageQueue(new MessageQueue()),
        threadPool(),
        cleanerRunnable(new MessageQueueCleanerRunnable(*messageQueue, 50))
    {
        threadPool.setMaxThreadCount(1);
        threadPool.start(cleanerRunnable);
    }

    ~MessageQueueTest(){
        cleanerRunnable->stop();
        threadPool.waitForDone();
        delete messageQueue;
    }

protected:
    MessageQueue* messageQueue;
    QThreadPool threadPool;
    MessageQueueCleanerRunnable* cleanerRunnable;

private:
    DISALLOW_COPY_AND_ASSIGN(MessageQueueTest);
};

TEST_F(MessageQueueTest, initialQueueIsEmpty) {
    EXPECT_EQ(messageQueue->getQueueLength(), 0);
}

TEST_F(MessageQueueTest, addMultipleMessages) {
    EXPECT_EQ(messageQueue->queueMessage(JoynrMessage(), MessagingQos()), 1);
    EXPECT_EQ(messageQueue->queueMessage(JoynrMessage(), MessagingQos()), 2);
    EXPECT_EQ(messageQueue->queueMessage(JoynrMessage(), MessagingQos()), 3);
    EXPECT_EQ(messageQueue->queueMessage(JoynrMessage(), MessagingQos()), 4);
}

TEST_F(MessageQueueTest, queueDequeueMessages) {
    // add messages to the queue
    JoynrMessage msg1;
    msg1.setHeaderTo("TEST1");
    messageQueue->queueMessage(msg1, MessagingQos());

    JoynrMessage msg2;
    msg2.setHeaderTo("TEST2");
    messageQueue->queueMessage(msg2, MessagingQos());
    EXPECT_EQ(messageQueue->getQueueLength(), 2);

    // get messages from queue
    MessageQueueItem* item = messageQueue->getNextMessageForParticipant("TEST1");
    EXPECT_EQ(item->getContent().first, msg1);
    EXPECT_EQ(messageQueue->getQueueLength(), 1);

    item = messageQueue->getNextMessageForParticipant("TEST2");
    EXPECT_EQ(item->getContent().first, msg2);
    EXPECT_EQ(messageQueue->getQueueLength(), 0);
}

TEST_F(MessageQueueTest, queueDequeueMultipleMessagesForOneParticipant) {
    // add messages to the queue
    JoynrMessage msg;
    msg.setHeaderTo("TEST");
    messageQueue->queueMessage(msg, MessagingQos());
    messageQueue->queueMessage(msg, MessagingQos());
    EXPECT_EQ(messageQueue->getQueueLength(), 2);

    // get messages from queue
    MessageQueueItem* item = messageQueue->getNextMessageForParticipant("TEST");
    EXPECT_EQ(item->getContent().first, msg);
    EXPECT_EQ(messageQueue->getQueueLength(), 1);

    item = messageQueue->getNextMessageForParticipant("TEST");
    EXPECT_EQ(item->getContent().first, msg);
    EXPECT_EQ(messageQueue->getQueueLength(), 0);
}

TEST_F(MessageQueueTest, dequeueInvalidParticipantId) {
    EXPECT_FALSE(messageQueue->getNextMessageForParticipant("TEST"));
}

TEST_F(MessageQueueTest, removeOutdatedMessage) {
    EXPECT_EQ(messageQueue->queueMessage(JoynrMessage(), MessagingQos(10)), 1);
    QThread::msleep(5);
    EXPECT_EQ(messageQueue->removeOutdatedMessages(), 0);
    QThread::msleep(6);
    EXPECT_EQ(messageQueue->removeOutdatedMessages(), 1);
}

TEST_F(MessageQueueTest, removeOutdatedMessagesWithRunnable) {
    EXPECT_EQ(messageQueue->queueMessage(JoynrMessage(), MessagingQos(25)), 1);
    EXPECT_EQ(messageQueue->queueMessage(JoynrMessage(), MessagingQos(250)), 2);
    EXPECT_EQ(messageQueue->queueMessage(JoynrMessage(), MessagingQos(250)), 3);

    // wait to remove the first message
    QThread::msleep(100);
    EXPECT_EQ(messageQueue->getQueueLength(), 2);

    // wait to remove all messages
    QThread::msleep(500);
    EXPECT_EQ(messageQueue->getQueueLength(), 0);
}