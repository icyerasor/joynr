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
#ifndef JOYNREXCEPTIONSERIALIZER_H
#define JOYNREXCEPTIONSERIALIZER_H

#include <iosfwd>

#include "joynr/ClassDeserializer.h"
#include "joynr/ClassSerializer.h"
#include "joynr/exceptions/JoynrException.h"

namespace joynr
{

class IObject;

// Serializes a JoynrRuntimeException
template <>
void ClassSerializerImpl<exceptions::JoynrRuntimeException>::serialize(const exceptions::JoynrRuntimeException& exception, std::ostream& o);

// Deserializes a JoynrRuntimeException
template <>
void ClassDeserializerImpl<exceptions::JoynrRuntimeException>::deserialize(exceptions::JoynrRuntimeException& t, IObject& o);

// Serializes a JoynrTimeOutException
template <>
void ClassSerializerImpl<exceptions::JoynrTimeOutException>::serialize(const exceptions::JoynrTimeOutException& exception, std::ostream& o);

// Deserializes a JoynrTimeOutException
template <>
void ClassDeserializerImpl<exceptions::JoynrTimeOutException>::deserialize(exceptions::JoynrTimeOutException& t, IObject& o);

// Serializes a DiscoveryException
template <>
void ClassSerializerImpl<exceptions::DiscoveryException>::serialize(const exceptions::DiscoveryException& exception, std::ostream& o);

// Deserializes a DiscoveryException
template <>
void ClassDeserializerImpl<exceptions::DiscoveryException>::deserialize(exceptions::DiscoveryException& t, IObject& o);

// Serializes a PublicationMissedException
template <>
void ClassSerializerImpl<exceptions::PublicationMissedException>::serialize(const exceptions::PublicationMissedException& exception, std::ostream& o);

// Deserializes a PublicationMissedException
template <>
void ClassDeserializerImpl<exceptions::PublicationMissedException>::deserialize(exceptions::PublicationMissedException& t, IObject& o);

// Serializes a ApplicationException
template <>
void ClassSerializerImpl<exceptions::ApplicationException>::serialize(const exceptions::ApplicationException& exception, std::ostream& o);

// Deserializes a ApplicationException
template <>
void ClassDeserializerImpl<exceptions::ApplicationException>::deserialize(exceptions::ApplicationException& t, IObject& o);

// Serializes a ProviderRuntimeException
template <>
void ClassSerializerImpl<exceptions::ProviderRuntimeException>::serialize(const exceptions::ProviderRuntimeException& exception, std::ostream& o);
// Deserializes a ProviderRuntimeException
template <>
void ClassDeserializerImpl<exceptions::ProviderRuntimeException>::deserialize(exceptions::ProviderRuntimeException& t, IObject& o);

// Serializes a MethodInvocationException
template <>
void ClassSerializerImpl<exceptions::MethodInvocationException>::serialize(const exceptions::MethodInvocationException& exception, std::ostream& o);
// Deserializes a MethodInvocationException
template <>
void ClassDeserializerImpl<exceptions::MethodInvocationException>::deserialize(exceptions::MethodInvocationException& t, IObject& o);
} // namespace joynr
#endif // JOYNREXCEPTIONSERIALIZER_H
