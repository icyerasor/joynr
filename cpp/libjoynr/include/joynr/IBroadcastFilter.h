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
#ifndef IBROADCASTFILTER_H
#define IBROADCASTFILTER_H

#include "joynr/JoynrExport.h"
#include "joynr/BroadcastFilterParameters.h"
#include <string>
#include <vector>

namespace joynr
{

class Variant;

/**
  * @class IBroadcastFilter
  * @brief The IBroadcastFilter class provides an interface for filter logic implementation.
  */
class JOYNR_EXPORT IBroadcastFilter
{
public:
    explicit IBroadcastFilter(std::string name) : name(name)
    {
    }
    virtual ~IBroadcastFilter() = default;

    virtual bool filter(const std::vector<Variant>& eventValues,
                        const BroadcastFilterParameters& filterParameters) = 0;

    const std::string& getName()
    {
        return name;
    }

private:
    std::string name;
};

} // namespace joynr

#endif // BROADCASTFILTER_H
