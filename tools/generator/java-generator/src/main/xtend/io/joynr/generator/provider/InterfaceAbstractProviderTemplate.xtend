package io.joynr.generator.provider
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
import io.joynr.generator.templates.InterfaceTemplate
import io.joynr.generator.templates.util.AttributeUtil
import io.joynr.generator.templates.util.InterfaceUtil
import io.joynr.generator.templates.util.NamingUtil
import io.joynr.generator.util.JavaTypeUtil
import io.joynr.generator.util.JoynrJavaGeneratorExtensions
import io.joynr.generator.util.TemplateBase
import org.franca.core.franca.FInterface

class InterfaceAbstractProviderTemplate implements InterfaceTemplate{
	@Inject extension JoynrJavaGeneratorExtensions
	@Inject extension JavaTypeUtil
	@Inject extension NamingUtil
	@Inject extension InterfaceUtil
	@Inject extension AttributeUtil
	@Inject extension TemplateBase

	override generate(FInterface serviceInterface) {
		val interfaceName =  serviceInterface.joynrName
		val className = interfaceName + "AbstractProvider"
		val providerInterfaceName = interfaceName + "Provider"
		val packagePath = getPackagePathWithJoynrPrefix(serviceInterface, ".")

		'''
«warning()»
package «packagePath»;

import io.joynr.provider.AbstractJoynrProvider;

«FOR datatype : getRequiredIncludesFor(serviceInterface, false, false, false, true, true)»
	import «datatype»;
«ENDFOR»

public abstract class «className» extends AbstractJoynrProvider implements «providerInterfaceName» {

	@Override
	public Class<?> getProvidedInterface() {
		return «providerInterfaceName».class;
	}

	@Override
	public String getInterfaceName() {
		return «providerInterfaceName».INTERFACE_NAME;
	}

	«FOR attribute : getAttributes(serviceInterface)»
		«val attributeName = attribute.joynrName»
		«val attributeType = attribute.typeName»
		«IF isNotifiable(attribute)»
			@Override
			public final void «attributeName»Changed(«attributeType» «attributeName») {
				onAttributeValueChanged("«attributeName»", «attributeName»);
			}
		«ENDIF»
	«ENDFOR»

	«FOR broadcast : serviceInterface.broadcasts»
		«var broadcastName = broadcast.joynrName»
		@Override
		public void fire«broadcastName.toFirstUpper»(«broadcast.commaSeperatedTypedOutputParameterList») {
			«IF broadcast.outArgs.length == 1 && (isArray(broadcast.outArgs.get(0)) || isByteBuffer(broadcast.outArgs.get(0).type))»
				// passing array to varargs will cause array elements to be understood as multiple parameters.
				// Cast to Object prevents this.
				fireBroadcast("«broadcastName»", broadcastFilters.get("«broadcastName»"), (Object) «broadcast.commaSeperatedUntypedOutputParameterList»);
			«ELSE»
				fireBroadcast("«broadcastName»", broadcastFilters.get("«broadcastName»"), «broadcast.commaSeperatedUntypedOutputParameterList»);
			«ENDIF»
		}

	«ENDFOR»
}
		'''
	}
}
