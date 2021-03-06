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
import io.joynr.generator.templates.CompoundTypeTemplate
import io.joynr.generator.templates.util.NamingUtil
import javax.inject.Inject
import org.franca.core.franca.FBasicTypeId
import org.franca.core.franca.FCompoundType
import org.franca.core.franca.FTypeRef

class TypeSerializerCppTemplate implements CompoundTypeTemplate{

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
«val typeName = type.typeName»
«warning»
#include «type.includeOfSerializer»
#include "joynr/ArraySerializer.h"
#include "joynr/PrimitiveDeserializer.h"
#include "joynr/SerializerRegistry.h"
#include "joynr/Variant.h"
#include "joynr/JoynrTypeId.h"

#include <string>
#include <utility>
#include <algorithm>

namespace joynr
{

using namespace «getPackagePathWithJoynrPrefix(type, "::", true)»;

// Register the «joynrName» type id (_typeName value) and serializer/deserializer
static const bool is«joynrName»SerializerRegistered =
		SerializerRegistry::registerType<«joynrName»>("«typeName.replace("::", ".")»");

template <>
void ClassDeserializerImpl<«joynrName»>::deserialize(«joynrName» &«joynrName.toFirstLower»Var, IObject &object)
{
	«IF type.membersRecursive.isEmpty»
		std::ignore = «joynrName.toFirstLower»Var;
		std::ignore = object;
	«ELSE»
		while (object.hasNextField()) {
			IField& field = object.nextField();
			«FOR member: type.membersRecursive SEPARATOR " else "»
				if (field.name() == "«member.name»") {
				«IF member.array»
					«IF member.type.isPrimitive»
						«deserializePrimitiveArrayValue(member.type.getPrimitive,member.name, joynrName.toFirstLower+"Var", "field")»
					«ELSE»
						«val deserializerType = member.type.deserializer»
						IArray& array = field.value();
						auto&& converted«member.name.toFirstUpper» = convertArray<«member.type.typeName»>(array, «deserializerType»<«member.type.typeName»>::deserialize);
						«joynrName.toFirstLower»Var.set«member.name.toFirstUpper»(std::forward<std::vector<«member.type.typeName»>>(converted«member.name.toFirstUpper»));
					«ENDIF»
				«ELSE»
					«IF member.type.isPrimitive»
						«deserializePrimitiveValue(member.type.getPrimitive,member.name, joynrName.toFirstLower+"Var", "field")»
					«ELSE»
						«val deserializerType = member.type.deserializer»
						«member.type.typeName» «member.name»Container;
						«deserializerType»<«member.type.typeName»>::deserialize(«member.name»Container, field.value());
						«joynrName.toFirstLower»Var.set«member.name.toFirstUpper»(«member.name»Container);
					«ENDIF»
				«ENDIF»
				}
			«ENDFOR»
		}
	«ENDIF»
}

template <>
void ClassSerializerImpl<«joynrName»>::serialize(const «joynrName» &«joynrName.toFirstLower»Var, std::ostream& stream)
{
	«IF type.membersRecursive.isEmpty»
		std::ignore = «joynrName.toFirstLower»Var;
	«ENDIF»
	«val members = type.membersRecursive»
	stream << "{";
	stream << "\"_typeName\":\"" << JoynrTypeId<«joynrName»>::getTypeName() << "\"«IF !members.empty»,«ENDIF»";
	«FOR member: members SEPARATOR "\nstream << \",\";"»
		«IF member.array»
			«IF member.type.isPrimitive»
				«serializePrimitiveArrayValue(member.type.predefined, member.name, joynrName.toFirstLower + "Var")»
			«ELSE»
				stream << "\"«member.name»\": ";
				ArraySerializer::serialize<«member.type.typeName»>(«joynrName.toFirstLower»Var.get«member.name.toFirstUpper»(), stream);
			«ENDIF»
		«ELSE»
			«IF member.type.isPrimitive»
				«serializePrimitiveValue(member.type.predefined, member.name, joynrName.toFirstLower + "Var")»
			«ELSE»
				stream << "\"«member.name»\": ";
				ClassSerializerImpl<«member.typeName»> «member.name»Serializer;
				«member.name»Serializer.serialize(«joynrName.toFirstLower»Var.get«member.name.toFirstUpper»(), stream);
			«ENDIF»
		«ENDIF»
	«ENDFOR»
	stream << "}";
}

} // namespace joynr

'''

def getDeserializer(FTypeRef type) {
	val targetType = if (type.isTypeDef) type.typeDefType.actualType else type
	if (targetType.isEnum){
		"PrimitiveDeserializer"
	} else if (targetType.isCompound || targetType.isMap) {
		"ClassDeserializer"
	} else {
		throw new IllegalStateException("No deserializer known for type " + type.class.simpleName)
	}
}

def deserializePrimitiveArrayValue(FBasicTypeId basicType, String memberName, String varName, String fieldName) {
	var primitiveType = basicType.typeName
	var converterFunction = ""
	var converterFunctionTemplate = "<" + primitiveType + ">"
	switch basicType {
		case BOOLEAN: {
			converterFunction = "convertBool"
			converterFunctionTemplate = ""
		}
		case STRING : {
			converterFunction = "convertString"
			converterFunctionTemplate = ""
		}
		case INT8,
		case INT16,
		case INT32,
		case INT64 : converterFunction = "convertIntType"
		case UINT8,
		case UINT16,
		case UINT32,
		case UINT64: converterFunction = "convertUIntType"
		case FLOAT,
		case DOUBLE : converterFunction = "convertDoubleType"
		default: throw new IllegalStateException("Type for member " + memberName + " could not be resolved")
	}
	'''
	IArray& array = «fieldName».value();
	auto&& converted«memberName.toFirstUpper» = convertArray<«primitiveType»>(array, «converterFunction»«converterFunctionTemplate»);
	«varName».set«memberName.toFirstUpper»(std::forward<std::vector<«primitiveType»>>(converted«memberName.toFirstUpper»));
	'''
}

def deserializePrimitiveValue(FBasicTypeId basicType, String memberName, String varName, String fieldName) {
	var deserializedValue = varName + ".set" + memberName.toFirstUpper + "(" + fieldName + ".value()"
	switch basicType {
		case BYTE_BUFFER : return '''
		IArray& array = «fieldName».value();
		auto&& converted«memberName.toFirstUpper» = convertArray<std::uint8_t>(array, convertUIntType<std::uint8_t>);
		«varName».set«memberName.toFirstUpper»(std::forward<std::vector<std::uint8_t>>(converted«memberName.toFirstUpper»));
		'''
		case STRING : '''
		std::string stringValue;
		PrimitiveDeserializer<std::string>::deserialize(stringValue, «fieldName».value());
		«varName».set«memberName.toFirstUpper»(stringValue);
		'''
		case BOOLEAN : return deserializedValue + ".getBool());"
		case INT8 : return deserializedValue + ".getIntType<std::int8_t>());"
		case INT16 : return deserializedValue + ".getIntType<std::int16_t>());"
		case INT32 : return deserializedValue + ".getIntType<std::int32_t>());"
		case INT64 : return deserializedValue + ".getIntType<std::int64_t>());"
		case UINT8 : return deserializedValue + ".getIntType<std::uint8_t>());"
		case UINT16 : return deserializedValue + ".getIntType<std::uint16_t>());"
		case UINT32 : return deserializedValue + ".getIntType<std::uint32_t>());"
		case UINT64 : return deserializedValue + ".getIntType<std::uint64_t>());"
		case FLOAT : return deserializedValue + ".getDoubleType<float>());"
		case DOUBLE : return deserializedValue + ".getDoubleType<double>());"
		default: throw new IllegalStateException("Type for member " + memberName + " could not be resolved")
	}
}

def serializePrimitiveArrayValue(FBasicTypeId basicType, String memberName, String varName) {
	'''
	stream << "\"«memberName»\": ";
	ArraySerializer::serialize<«basicType.typeName»>(«varName».get«memberName.toFirstUpper»(), stream);
	'''
}
def serializePrimitiveValue(FBasicTypeId basicType, String memberName, String varName) {
	switch (basicType) {
		case INT8,
		case INT16,
		case INT32,
		case INT64,
		case UINT8,
		case UINT16,
		case UINT32,
		case UINT64: return '''
			stream << "\"«memberName»\": " << std::to_string(«varName».get«memberName.toFirstUpper»());
		'''
		case DOUBLE: return '''
			stream << "\"«memberName»\": ";
			ClassSerializerImpl<double>::serialize(«varName».get«memberName.toFirstUpper»(), stream);
		'''
		case FLOAT: return '''
			stream << "\"«memberName»\": ";
			ClassSerializerImpl<float>::serialize(«varName».get«memberName.toFirstUpper»(), stream);
		'''
		case STRING: return '''
			stream << "\"«memberName»\": ";
			ClassSerializerImpl<std::string>::serialize(«varName».get«memberName.toFirstUpper»(), stream);
		'''
		case BOOLEAN: 
		return '''
			stream << "\"«memberName»\": ";
			if («varName».get«memberName.toFirstUpper»()) {
				stream << "true";
			} else {
				stream << "false";
			}
		'''
		case BYTE_BUFFER:
		return '''
			stream << "\"«memberName»\": ";
			ArraySerializer::serialize<std::uint8_t>(«varName».get«memberName.toFirstUpper»(), stream);
		'''
		default: throw new IllegalStateException("Type for member " + memberName + " could not be resolved")
	}
}
}
