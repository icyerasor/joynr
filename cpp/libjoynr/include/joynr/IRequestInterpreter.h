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
#ifndef IREQUESTINTERPRETER_H
#define IREQUESTINTERPRETER_H

#include <functional>
#include <memory>
#include <string>
#include <vector>
#include "joynr/exceptions/JoynrException.h"
#include "joynr/Variant.h"

namespace joynr
{

class RequestCaller;

/**
  * Common interface for all @class <Intf>RequestInterpreter.
  */
class IRequestInterpreter
{
public:
    virtual ~IRequestInterpreter() = default;

    /**
      * Executes method @param methodName with parameters @param methodParams
      * on the @param requestCaller object.
      */
    virtual void execute(
            std::shared_ptr<RequestCaller> requestCaller,
            const std::string& methodName,
            const std::vector<Variant>& paramValues,
            const std::vector<std::string>& paramTypes,
            std::function<void(std::vector<Variant>&& outParams)> onSuccess,
            std::function<void(const exceptions::JoynrException& exception)> onError) = 0;
};

} // namespace joynr
#endif // IREQUESTINTERPRETER_H
