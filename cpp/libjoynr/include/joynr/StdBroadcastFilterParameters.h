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
#ifndef STDBROADCASTFILTERPARAMETERS_H
#define STDBROADCASTFILTERPARAMETERS_H

#include <map>
#include <string>

#include "joynr/JoynrCommonExport.h"

namespace joynr
{

/**
 * \class StdBroadcastFilterParameters
 * \brief The StdBroadcastFilterParameters class represents generic filter parameters
 * for selective broadcasts by using std types
 */
class JOYNRCOMMON_EXPORT StdBroadcastFilterParameters
{

public:
    StdBroadcastFilterParameters();
    StdBroadcastFilterParameters(const StdBroadcastFilterParameters& filterParameters);
    StdBroadcastFilterParameters& operator=(const StdBroadcastFilterParameters& filterParameters);
    ~StdBroadcastFilterParameters();
    bool operator==(const StdBroadcastFilterParameters& filterParameters) const;

    void setFilterParameter(const std::string& parameter, const std::string& value);
    void setFilterParameters(const std::map<std::string, std::string>& value);

    std::map<std::string, std::string> getFilterParameters() const;
    std::string getFilterParameter(const std::string& parameter) const;

private:
    std::map<std::string, std::string> filterParameters;
};

} // namespace joynr

#endif // STDBROADCASTFILTERPARAMETERS_H