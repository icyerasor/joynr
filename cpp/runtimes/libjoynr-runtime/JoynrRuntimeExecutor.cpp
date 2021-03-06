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
#include "runtimes/libjoynr-runtime/JoynrRuntimeExecutor.h"

#include "LibJoynrRuntime.h"
#include "runtimes/libjoynr-runtime/websocket/LibJoynrWebSocketRuntime.h"
#include "joynr/Settings.h"

namespace joynr
{

JoynrRuntimeExecutor::JoynrRuntimeExecutor(Settings* settings)
        : settings(settings), runtime(), runtimeSemaphore(0)
{
}

LibJoynrRuntime* JoynrRuntimeExecutor::getRuntime()
{
    runtimeSemaphore.wait();
    return runtime.release();
}

} // namespace joynr
