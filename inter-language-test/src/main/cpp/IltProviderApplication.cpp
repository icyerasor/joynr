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

#include "IltHelper.h"
#include "IltProvider.h"
#include "joynr/JoynrRuntime.h"
#include "IltStringBroadcastFilter.h"
#include "joynr/Semaphore.h"
#include <memory>
#include <string>
#include <iostream>
#include <signal.h>

using namespace joynr;

Semaphore semaphore;

int main(int argc, char* argv[])
{
    // setup alternate signal stack
    stack_t ss;
    ss.ss_sp = malloc(SIGSTKSZ);
    if (ss.ss_sp == nullptr) {
        std::cerr << "unable to allocate SIGSTKSZ bytes of stack\n" << std::endl;
        exit(1);
    }
    ss.ss_size = SIGSTKSZ;
    ss.ss_flags = 0;
    if (sigaltstack(&ss, nullptr) == -1) {
        std::cerr << "unable to call sigaltstack\n" << std::endl;
        exit(1);
    }

    // Get a logger
    Logger logger("IltProviderApplication");

    // Check the usage
    std::string programName(argv[0]);
    if (argc != 2) {
        JOYNR_LOG_ERROR(logger, "USAGE: {} <provider-domain>", programName);
        return 1;
    }

    // Get the provider domain
    std::string providerDomain(argv[1]);
    JOYNR_LOG_INFO(logger, "Registering provider on domain {}", providerDomain);

    // Get the current program directory
    std::string dir(IltHelper::getAbsolutePathToExecutable(programName));

    // Initialise the JOYn runtime
    std::string pathToMessagingSettings(dir + "/resources/test-app-provider.settings");
    // not used
    // std::string pathToLibJoynrSettings(dir.toStdString() +
    // "/resources/test-app-provider.libjoynr.settings");
    JoynrRuntime* runtime = JoynrRuntime::createRuntime(pathToMessagingSettings);

    // create provider instance
    std::shared_ptr<IltProvider> provider(new IltProvider());

    // add any broadcast filters here (later)
    std::shared_ptr<IltStringBroadcastFilter> myStringBroadcastFilter(
            new IltStringBroadcastFilter());
    provider->addBroadcastFilter(myStringBroadcastFilter);

    // Register the provider
    runtime->registerProvider<interlanguagetest::TestInterfaceProvider>(providerDomain, provider);

    JOYNR_LOG_INFO(logger, "********************************************************************");
    JOYNR_LOG_INFO(logger, "Provider is registered");
    JOYNR_LOG_INFO(logger, "********************************************************************");

    semaphore.wait();

    // Unregister the provider
    runtime->unregisterProvider<interlanguagetest::TestInterfaceProvider>(providerDomain, provider);

    delete runtime;
    return 0;
}
