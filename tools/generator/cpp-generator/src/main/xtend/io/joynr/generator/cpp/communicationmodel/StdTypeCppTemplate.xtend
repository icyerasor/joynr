package io.joynr.generator.cpp.communicationmodel
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

import com.google.inject.Inject
import io.joynr.generator.cpp.util.CppStdTypeUtil
import io.joynr.generator.cpp.util.JoynrCppGeneratorExtensions
import io.joynr.generator.cpp.util.TemplateBase
import io.joynr.generator.templates.CompoundTypeTemplate
import io.joynr.generator.templates.util.NamingUtil
import org.franca.core.franca.FCompoundType

class StdTypeCppTemplate implements CompoundTypeTemplate{

	@Inject
	private extension TemplateBase

	@Inject
	private extension JoynrCppGeneratorExtensions

	@Inject
	private extension CppStdTypeUtil

	@Inject
	private extension NamingUtil

	override generate(FCompoundType type) '''
«val typeName = type.joynrName»
«warning»

#include <sstream>
#include <string>
#include <typeinfo>

«IF type.hasExtendsDeclaration || getMembers(type).size > 0»
#include <boost/functional/hash.hpp>
«ENDIF»
#include "joynr/HashUtil.h"
#include «type.includeOf»

«getNamespaceStarter(type, true)»

static const bool is«typeName»Registered = Variant::registerType<«type.typeName»>("«type.typeName.replace("::", ".")»");

«typeName»::«typeName»()«IF !getMembersRecursive(type).empty»:«ENDIF»
	«IF hasExtendsDeclaration(type)»
		«getExtendedType(type).joynrName»()«IF !getMembers(type).empty»,«ENDIF»
	«ENDIF»
	«FOR member: getMembers(type) SEPARATOR ','»
		«member.joynrName»(«member.defaultValue»)
	«ENDFOR»
{
}

«IF !getMembersRecursive(type).empty»
«typeName»::«typeName»(
		«FOR member: getMembersRecursive(type) SEPARATOR ','»
			const «member.typeName» &«member.joynrName»
		«ENDFOR»
	):
		«IF hasExtendsDeclaration(type)»
			«val extendedType = getExtendedType(type)»
			«extendedType.joynrName»(
			«FOR member: getMembersRecursive(extendedType) SEPARATOR ','»
				«member.joynrName»
			«ENDFOR»
			),
		«ENDIF»
		«FOR member: getMembers(type) SEPARATOR ','»
			«member.joynrName»(«member.joynrName»)
		«ENDFOR»
{
}

«ENDIF»
«IF !getMembers(type).isEmpty»
// Copy Constructor
«typeName»::«typeName»(const «typeName»& other) :
		«IF hasExtendsDeclaration(type)»
			«getExtendedType(type).joynrName»(other),
		«ENDIF»
		«FOR member: getMembers(type) SEPARATOR ','»
			«member.joynrName»(other.«member.joynrName»)
		«ENDFOR»
{
}

«ENDIF»
bool «typeName»::operator!=(const «typeName»& other) const {
	return !(*this==other);
}

bool «typeName»::operator==(const «typeName»& other) const {
	if (typeid(*this) != typeid(other)) {
		return false;
	}

	return
		«FOR member: getMembers(type)»
			this->«member.joynrName» == other.«member.joynrName» &&
		«ENDFOR»
		«IF hasExtendsDeclaration(type)»
			«getExtendedType(type).typeName»::operator==(other);
		«ELSE»
			true;
		«ENDIF»
}

std::size_t «typeName»::hashCode() const {
	std::size_t seed = 0;

	«FOR member: getMembers(type)»
		«val joynrName = member.joynrName»
		boost::hash_combine(seed, get«joynrName.toFirstUpper»());
	«ENDFOR»

	«IF type.hasExtendsDeclaration»
		boost::hash_combine(seed, «type.extendedType.joynrName»::hashCode());
	«ENDIF»
	return seed;
}

«FOR member: getMembers(type)»
	«val joynrName = member.joynrName»
	«IF isEnum(member.type) && ! isArray(member)»
		std::string «typeName»::get«joynrName.toFirstUpper»Internal() const {
			return «member.typeName.substring(0, member.typeName.length-6)»::getLiteral(this->«joynrName»);
		}

	«ENDIF»
«ENDFOR»
std::string «typeName»::toString() const {
	std::ostringstream typeAsString;
	typeAsString << "«typeName»{";
	«IF hasExtendsDeclaration(type)»
		typeAsString << «getExtendedType(type).typeName»::toString();
		«IF !getMembers(type).empty»
		typeAsString << ", ";
		«ENDIF»
	«ENDIF»
	«FOR member: getMembers(type) SEPARATOR "\ntypeAsString << \", \";"»
		«val memberName = member.joynrName»
		«val memberType = if (member.type.isTypeDef) member.type.typeDefType.actualType else member.type»
		«IF isArray(member)»
			typeAsString << " unprinted List «memberName»  ";
		«ELSEIF isByteBuffer(memberType)»
			typeAsString << " unprinted ByteBuffer «memberName»  ";
		«ELSEIF isString(getPrimitive(memberType))»
			typeAsString << "«memberName»:" + get«memberName.toFirstUpper»();
		«ELSEIF isEnum(memberType)»
			typeAsString << "«memberName»:" + get«memberName.toFirstUpper»Internal();
		«ELSEIF isCompound(memberType)»
			typeAsString << "«memberName»:" + get«memberName.toFirstUpper»().toString();
		«ELSEIF isMap(memberType)»
			typeAsString << " unprinted Map «memberName»  ";
		«ELSE»
			typeAsString << "«memberName»:" + std::to_string(get«memberName.toFirstUpper»());
		«ENDIF»
	«ENDFOR»
	typeAsString << "}";
	return typeAsString.str();
}

// printing «typeName» with google-test and google-mock
void PrintTo(const «typeName»& «typeName.toFirstLower», ::std::ostream* os) {
	*os << "«typeName»::" << «typeName.toFirstLower».toString();
}

std::size_t hash_value(«typeName» const& «typeName.toFirstLower»Value)
{
	return «typeName.toFirstLower»Value.hashCode();
}

«getNamespaceEnder(type, true)»
'''
}
