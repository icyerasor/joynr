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
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "joynr/ReplyCaller.h"
#include "tests/utils/MockObjects.h"

using ::testing::Property;
using ::testing::Eq;
using ::testing::ByRef;
using ::testing::_;
using namespace ::testing;

MATCHER(timeoutException, "") {
    return (dynamic_cast<const joynr::exceptions::JoynrTimeOutException*>(&arg) != nullptr)
            && arg.getMessage() == "timeout waiting for the response";
}

MATCHER_P(providerRuntimeException, msg, "") {
    return arg.getTypeName() == joynr::exceptions::ProviderRuntimeException::TYPE_NAME
            && arg.getMessage() == msg;
}

using namespace joynr;

/*
 * This tests the ReplyCaller class, for all template specifications i.e. void and not void.
 */

class ReplyCallerTest : public ::testing::Test {
public:
    ReplyCallerTest()
        : intCallback(new MockCallbackWithJoynrException<int>()),
          intFixture(std::bind(&MockCallbackWithJoynrException<int>::onSuccess, intCallback, std::placeholders::_1),
                     std::bind(&MockCallbackWithJoynrException<int>::onError, intCallback, std::placeholders::_1)),
          voidCallback(new MockCallbackWithJoynrException<void>()),
          voidFixture(std::bind(&MockCallbackWithJoynrException<void>::onSuccess, voidCallback),
                      std::bind(&MockCallbackWithJoynrException<void>::onError, voidCallback, std::placeholders::_1)) {}

    std::shared_ptr<MockCallbackWithJoynrException<int>> intCallback;
    ReplyCaller<int> intFixture;
    std::shared_ptr<MockCallbackWithJoynrException<void>> voidCallback;
    ReplyCaller<void> voidFixture;
};

typedef ReplyCallerTest ReplyCallerDeathTest;

TEST_F(ReplyCallerTest, getType) {
    ASSERT_EQ(Util::getTypeId<int>(), intFixture.getTypeId());
}

TEST_F(ReplyCallerTest, getTypeInt64_t) {
    std::shared_ptr<MockCallbackWithJoynrException<std::int64_t>> callback(new MockCallbackWithJoynrException<std::int64_t>());
    ReplyCaller<std::int64_t> int64_tReplyCaller(
                [callback](const std::int64_t& value) {
                    callback->onSuccess(value);
                },
                [](const exceptions::JoynrException& error){
                });
    ASSERT_EQ(Util::getTypeId<std::int64_t>(), int64_tReplyCaller.getTypeId());
}

TEST_F(ReplyCallerTest, getTypeInt8_t) {
    std::shared_ptr<MockCallbackWithJoynrException<std::int8_t>> callback(new MockCallbackWithJoynrException<std::int8_t>());
    ReplyCaller<std::int8_t> int8_tReplyCaller(
                [callback](const std::int8_t& value) {
                    callback->onSuccess(value);
                },
                [](const exceptions::JoynrException& error){
                });
    ASSERT_EQ(Util::getTypeId<std::int8_t>(), int8_tReplyCaller.getTypeId());
}

TEST_F(ReplyCallerTest, getTypeForVoid) {
    int typeId = voidFixture.getTypeId();
    ASSERT_EQ(Util::getTypeId<void>(), typeId);
}


TEST_F(ReplyCallerTest, timeOut) {
    EXPECT_CALL(*intCallback, onSuccess(_)).Times(0);
    EXPECT_CALL(*intCallback, onError(timeoutException())).Times(1);
    intFixture.timeOut();
}

TEST_F(ReplyCallerTest, timeOutForVoid) {
    EXPECT_CALL(*voidCallback, onSuccess()).Times(0);
    EXPECT_CALL(*voidCallback, onError(timeoutException())).Times(1);
    voidFixture.timeOut();
}

TEST_F(ReplyCallerTest, errorReceived) {
    std::string errorMsg = "errorMsgFromProvider";
    EXPECT_CALL(*intCallback, onError(providerRuntimeException(errorMsg))).Times(1);
    EXPECT_CALL(*intCallback, onSuccess(_)).Times(0);
    intFixture.returnError(exceptions::ProviderRuntimeException(errorMsg));
}

TEST_F(ReplyCallerTest, errorReceivedForVoid) {
    std::string errorMsg = "errorMsgFromProvider";
    EXPECT_CALL(*voidCallback, onError(providerRuntimeException(errorMsg))).Times(1);
    EXPECT_CALL(*voidCallback, onSuccess()).Times(0);
    voidFixture.returnError(exceptions::ProviderRuntimeException(errorMsg));
}

TEST_F(ReplyCallerTest, resultReceived) {
    EXPECT_CALL(*intCallback, onSuccess(7));
    EXPECT_CALL(*intCallback, onError(_)).Times(0);
    intFixture.returnValue(7);
}

TEST_F(ReplyCallerTest, resultReceivedForVoid) {
    EXPECT_CALL(*voidCallback, onSuccess());
    EXPECT_CALL(*voidCallback, onError(_)).Times(0);
    voidFixture.returnValue();
}

