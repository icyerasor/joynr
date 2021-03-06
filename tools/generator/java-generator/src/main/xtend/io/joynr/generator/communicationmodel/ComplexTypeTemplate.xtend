package io.joynr.generator.communicationmodel
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
import io.joynr.generator.templates.CompoundTypeTemplate
import io.joynr.generator.templates.util.NamingUtil
import io.joynr.generator.util.JavaTypeUtil
import io.joynr.generator.util.JoynrJavaGeneratorExtensions
import io.joynr.generator.util.TemplateBase
import org.franca.core.franca.FCompoundType

class ComplexTypeTemplate implements CompoundTypeTemplate{

	@Inject	extension JoynrJavaGeneratorExtensions
	@Inject extension JavaTypeUtil
	@Inject extension TemplateBase
	@Inject extension NamingUtil

	override generate(FCompoundType complexType) {
		val typeName = complexType.joynrName
		val complexTypePackageName = complexType.buildPackagePath(".", true)
		'''
		«warning()»

package «complexTypePackageName»;
import java.io.Serializable;

import io.joynr.subtypes.JoynrType;

«FOR member : getRequiredIncludesFor(complexType)»
import «member»;
«ENDFOR»
«IF hasArrayMembers(complexType)»
import edu.umd.cs.findbugs.annotations.SuppressFBWarnings;
«ENDIF»
import com.fasterxml.jackson.annotation.JsonProperty;
import com.fasterxml.jackson.annotation.JsonIgnore;

// NOTE: serialVersionUID is not defined since we don't support Franca versions right now.
//       The compiler will generate a serialVersionUID based on the class and its members
//       (cf. http://docs.oracle.com/javase/6/docs/platform/serialization/spec/class.html#4100),
//       which is probably more restrictive than what we want.

/**
«appendJavadocSummaryAndWriteSeeAndDescription(complexType, " *")»
 */
@SuppressWarnings("serial")
public class «typeName»«IF hasExtendsDeclaration(complexType)» extends «complexType.extendedType.typeName»«ENDIF» implements Serializable, JoynrType {
	«FOR member : getMembers(complexType)»
	«val memberType = member.typeName.replace("::","__")»
	@JsonProperty("«member.joynrName»")
	«IF isArray(member)»
	private «memberType» «member.joynrName» = {};
	«ELSE»
	private «memberType» «member.joynrName»;
	«ENDIF»
	«ENDFOR»

	/**
	 * Default Constructor
	 */
	public «typeName»() {
		«FOR member : getMembers(complexType)»
			«IF !(isArray(member))»
				this.«member.joynrName» = «member.defaultValue»;
			«ENDIF»
		«ENDFOR»
	}

	«val copyObjName = typeName.toFirstLower + "Obj"»
	/**
	 * Copy constructor
	 *
	 * @param «copyObjName» reference to the object to be copied
	 */
	public «typeName»(«typeName» «copyObjName») {
		«IF complexType.hasExtendsDeclaration»
		super(«copyObjName»);
		«ENDIF»
		«FOR member : getMembers(complexType)»
		«IF isArray(member)»
			this.«member.joynrName» = «copyObjName».«member.joynrName»;
		«ELSE»
			«IF isCompound(member.type) || isMap(member.type)»
			«val memberType = member.type.typeName»
			this.«member.joynrName» = new «memberType»(«copyObjName».«member.joynrName»);
			«ELSE»
			this.«member.joynrName» = «copyObjName».«member.joynrName»;
			«ENDIF»
		«ENDIF»
		«ENDFOR»
	}

	«IF !getMembersRecursive(complexType).empty»
	/**
	 * Parameterized constructor
	 *
	 «FOR member : getMembersRecursive(complexType)»
	 «appendJavadocParameter(member, "*")»
	 «ENDFOR»
	 */
	«IF hasArrayMembers(complexType)»@SuppressFBWarnings(value = "EI_EXPOSE_REP2", justification = "joynr object not used for storing internal state")«ENDIF»
	public «typeName»(
		«FOR member : getMembersRecursive(complexType) SEPARATOR ','»
		«member.typeName.replace("::","__")» «member.joynrName»
		«ENDFOR»
		) {
		«IF hasExtendsDeclaration(complexType)»
			super(
					«FOR member: getMembersRecursive(complexType.extendedType) SEPARATOR ','»
						«member.joynrName»
					«ENDFOR»
			);
		«ENDIF»
		«FOR member : getMembers(complexType)»
		this.«member.joynrName» = «member.joynrName»;
		«ENDFOR»
	}
	«ENDIF»

	«FOR member : getMembers(complexType)»
	«val memberType = member.typeName.replace("::","__")»
	«val memberName = member.joynrName»
	/**
	 * Gets «memberName.toFirstUpper»
	 *
	 * @return «appendJavadocComment(member, "* ")»«IF member.type.isTypeDef» (type resolved from modeled Franca typedef «member.type.joynrName» as «member.type.typeDefType.actualType.typeName»)«ENDIF»
	 */
	«IF isArray(member)»@SuppressFBWarnings(value = "EI_EXPOSE_REP", justification = "joynr object not used for storing internal state")«ENDIF»
	@JsonIgnore
	public «memberType» get«memberName.toFirstUpper»() {
		return this.«member.joynrName»;
	}

	/**
	 * Sets «memberName.toFirstUpper»
	 *
	 «appendJavadocParameter(member, "*")»
	 */
	«IF isArray(member)»@SuppressFBWarnings(value = "EI_EXPOSE_REP2", justification = "joynr object not used for storing internal state")«ENDIF»
	@JsonIgnore
	public void set«memberName.toFirstUpper»(«memberType» «member.joynrName») {
		this.«member.joynrName» = «member.joynrName»;
	}

	«ENDFOR»

	/**
	 * Stringifies the class
	 *
	 * @return stringified class content
	 */
	@Override
	public String toString() {
		return "«typeName» ["
		«IF hasExtendsDeclaration(complexType)»
				+ super.toString() + ", "
		«ENDIF»
		«FOR member : getMembers(complexType) SEPARATOR " + \", \""»
			«IF isArray(member)»
				+ "«member.joynrName»=" + java.util.Arrays.toString(this.«member.joynrName»)
			«ELSE»
				+ "«member.joynrName»=" + this.«member.joynrName»
			«ENDIF»
		«ENDFOR»
		+ "]";
	}

	/**
	 * Check for equality
	 *
	 * @param obj Reference to the object to compare to
	 * @return true, if objects are equal, false otherwise
	 */
	@Override
	public boolean equals(Object obj) {
		if (this == obj)
			return true;
		if (obj == null)
			return false;
		if (getClass() != obj.getClass())
			return false;
		«IF hasExtendsDeclaration(complexType)»
			if (!super.equals(obj))
				return false;
		«ENDIF»
		«IF !getMembers(complexType).empty»
		«typeName» other = («typeName») obj;
		«ENDIF»
		«FOR member : getMembers(complexType)»
			if (this.«member.joynrName» == null) {
				if (other.«member.joynrName» != null) {
					return false;
				}
			«IF isByteBuffer(member.type)»
			} else if (!java.util.Arrays.equals(this.«member.joynrName», other.«member.joynrName»)){
				return false;
			}
			«ELSEIF isArray(member)»
			} else if (!java.util.Arrays.deepEquals(this.«member.joynrName», other.«member.joynrName»)){
				return false;
			}
			«ELSE»
			} else if (!this.«member.joynrName».equals(other.«member.joynrName»)){
				return false;
			}
			«ENDIF»
		«ENDFOR»
		return true;
	}

	/**
	 * Calculate code for hashing based on member contents
	 *
	 * @return The calculated hash code
	 */
	@Override
	public int hashCode() {
		«IF hasExtendsDeclaration(complexType)»
			int result = super.hashCode();
		«ELSE»
			int result = 1;
		«ENDIF»
		«IF !getMembers(complexType).empty»
		final int prime = 31;
		«ENDIF»
		«FOR member : getMembers(complexType)»
			«IF isByteBuffer(member.type) || isArray(member)»
				result = prime * result + ((this.«member.joynrName» == null) ? 0 : java.util.Arrays.hashCode(this.«member.joynrName»));
			«ELSE»
				result = prime * result + ((this.«member.joynrName» == null) ? 0 : this.«member.joynrName».hashCode());
			«ENDIF»
		«ENDFOR»
		return result;
	}
}

	
		'''	
	}
}
