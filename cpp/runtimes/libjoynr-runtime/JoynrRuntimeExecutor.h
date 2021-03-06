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

#ifndef JOYNRRUNTIMEEXECUTOR_H
#define JOYNRRUNTIMEEXECUTOR_H

#include "joynr/PrivateCopyAssign.h"
#include "joynr/Semaphore.h"

#include <memory>

namespace joynr
{

class LibJoynrRuntime;
class Settings;

class JoynrRuntimeExecutor
{

protected:
    Settings* settings;
    std::unique_ptr<LibJoynrRuntime> runtime;
    Semaphore runtimeSemaphore;

public:
    explicit JoynrRuntimeExecutor(Settings* settings);
    virtual ~JoynrRuntimeExecutor() = default;

    LibJoynrRuntime* getRuntime();

private:
    DISALLOW_COPY_AND_ASSIGN(JoynrRuntimeExecutor);
};

} // namespace joynr
#endif // JOYNRRUNTIMEEXECUTOR_H
