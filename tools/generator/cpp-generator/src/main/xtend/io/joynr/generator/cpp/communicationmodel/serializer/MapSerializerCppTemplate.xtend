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
 *	  http://www.apache.org/licenses/LICENSE-2.0
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
import io.joynr.generator.templates.MapTemplate
import io.joynr.generator.templates.util.NamingUtil
import javax.inject.Inject
import org.franca.core.franca.FMapType

class MapSerializerCppTemplate implements MapTemplate{

	@Inject private extension JoynrCppGeneratorExtensions

	@Inject private extension CppStdTypeUtil

	@Inject private extension NamingUtil

	@Inject private extension TemplateBase

	override generate(FMapType type)
'''
«val joynrName = type.joynrName»
«val typeName = type.typeName»
«warning»
#include «type.includeOfSerializer»
#include "joynr/ArraySerializer.h"
#include "joynr/PrimitiveDeserializer.h"
#include "joynr/MapSerializer.h"
#include "joynr/SerializerRegistry.h"
#include "joynr/Variant.h"
#include "joynr/JoynrTypeId.h"

#include <string>
#include <utility>
#include <algorithm>

namespace joynr
{

using namespace «getPackagePathWithJoynrPrefix(type, "::", true)»;

«var fqJoynrName = typeName.replace("::", ".")»
// Register the «joynrName» type id (_typeName value) and serializer/deserializer
static const bool is«joynrName»SerializerRegistered =
		SerializerRegistry::registerType<«joynrName»>("«fqJoynrName»");

template <>
void ClassSerializerImpl<«joynrName»>::serialize(const «joynrName» &«joynrName.toFirstLower»Var, std::ostream& stream)
{
	MapSerializer::serialize<«type.keyType.typeName», «type.valueType.typeName»>(
				"«fqJoynrName»",
				«joynrName.toFirstLower»Var,
				stream);

}

} // namespace joynr

'''

}
