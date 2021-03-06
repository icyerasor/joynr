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
#ifndef INTERFACEADDRESS_H_
#define INTERFACEADDRESS_H_

#include "joynr/JoynrCommonExport.h"
#include <string>

namespace joynr
{

/**
  * Immutable data class for messaging interface addresses.
  */
class JOYNRCOMMON_EXPORT InterfaceAddress
{
public:
    InterfaceAddress();
    explicit InterfaceAddress(const std::string& domain, const std::string& interfaceName);
    const std::string& getDomain() const;
    const std::string& getInterface() const;

    bool operator==(const InterfaceAddress& interfaceAddress) const;
    bool operator<(const InterfaceAddress& interfaceAddress) const;

private:
    std::string domain;
    std::string interfaceName;
};

} // namespace joynr
#endif // INTERFACEADDRESS_H_
