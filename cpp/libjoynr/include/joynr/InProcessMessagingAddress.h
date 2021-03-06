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
#ifndef INPROCESSMESSAGINGADDRESS_H
#define INPROCESSMESSAGINGADDRESS_H

#include "joynr/JoynrCommonExport.h"
#include "joynr/system/RoutingTypes/Address.h"

#include <memory>

namespace joynr
{

class InProcessMessagingSkeleton;

class JOYNRCOMMON_EXPORT InProcessMessagingAddress : public joynr::system::RoutingTypes::Address
{
public:
    explicit InProcessMessagingAddress(std::shared_ptr<InProcessMessagingSkeleton> skeleton);
    std::shared_ptr<InProcessMessagingSkeleton> getSkeleton() const;

private:
    std::shared_ptr<InProcessMessagingSkeleton> skeleton;
};

} // namespace joynr
#endif // INPROCESSMESSAGINGADDRESS_H
