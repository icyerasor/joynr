package io.joynr.generator.cpp.communicationmodel.serializer
/*
 * !!!
 *
 * Copyright (C) 2011 - 2015 BMW Car IT GmbH
 *
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
 */

import io.joynr.generator.cpp.util.CppStdTypeUtil
import io.joynr.generator.cpp.util.JoynrCppGeneratorExtensions
import io.joynr.generator.cpp.util.TemplateBase
import io.joynr.generator.templates.CompoundTypeTemplate
import io.joynr.generator.templates.util.NamingUtil
import javax.inject.Inject
import org.franca.core.franca.FCompoundType

class TypeSerializerHTemplate implements CompoundTypeTemplate{

	@Inject
	private extension JoynrCppGeneratorExtensions

	@Inject
	private extension CppStdTypeUtil

	@Inject
	private extension NamingUtil

	@Inject
	private extension TemplateBase

	override generate(FCompoundType type)
'''
«val joynrName = type.joynrName»
«val headerGuard = ("GENERATED_TYPE_"+getPackagePathWithJoynrPrefix(type, "_", true)+"_"+joynrName+"_SERIALIZER_H").toUpperCase»
«warning()»
#ifndef «headerGuard»
#define «headerGuard»

#include "joynr/ClassDeserializer.h"
#include "joynr/ClassSerializer.h"
#include «getIncludeOf(type)»
#include <ostream>

namespace joynr
{

using «joynrName» = «type.typeName»;

/**
 * @brief Specialized serialize method for «joynrName».
 * @param «joynrName.toFirstLower» the object to serialization
 * @param stringstream the stream to write the serialized content to
 */
template <>
void ClassSerializerImpl<«joynrName»>::serialize(const «joynrName»& «joynrName.toFirstLower», std::ostream& stringstream);

/**
 * @brief Specialized deserialize method for «joynrName».
 * @param «joynrName.toFirstLower» the object to fill during deserialization
 * @param object object containing the parsed json tokens
 */
template <>
void ClassDeserializerImpl<«joynrName»>::deserialize(«joynrName»& «joynrName.toFirstLower», IObject& object);

} // namespace joynr

#endif // «headerGuard»
'''
}
