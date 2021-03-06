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
#ifndef JOYNROPTIONAL_H
#define JOYNROPTIONAL_H

#include "joynr/JoynrCommonExport.h"
#include <cassert>

namespace joynr
{
/**
 * This is a wrapper for another type that allows to possibly not return a value from a function,
 * or pass semi-null value to a function.
 */
template <typename T>
class JOYNRCOMMON_EXPORT Optional
{
private:
    T value;
    bool invalid;
    Optional();

public:
    Optional(const T& value);
    Optional(T&& value);

    Optional(const Optional&) = default;
    Optional(Optional&&) = default;

    Optional& operator=(Optional&&) = default;
    Optional& operator=(const Optional&) = default;

    static Optional createNull();

    explicit operator bool() const;

    /**
     * @brief getValue is a handle to wrapped value.
     * Prior calling this function check if optional is null.
     * @return Wrapped value. If optional is null, this function returns object in unknown state.
     */
    T getValue() const;
};

template <typename T>
Optional<T>::Optional()
        : value(T()), invalid(true)
{
}

template <typename T>
Optional<T>::Optional(const T& value)
        : value(value), invalid(false)
{
}

template <typename T>
Optional<T>::Optional(T&& value)
        : value(std::move(value)), invalid(false)
{
}

template <typename T>
Optional<T> Optional<T>::createNull()
{
    return Optional();
}

template <typename T>
Optional<T>::operator bool() const
{
    return !invalid;
}

template <typename T>
T Optional<T>::getValue() const
{
    assert(!this->invalid && "Optional is not initialized properly, it holds uninitialized value!");
    return value;
}
} // namespace joynr
#endif // JOYNROPTIONAL_H
