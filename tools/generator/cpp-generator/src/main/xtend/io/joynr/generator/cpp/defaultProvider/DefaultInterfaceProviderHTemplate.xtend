package io.joynr.generator.cpp.defaultProvider
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
import io.joynr.generator.templates.InterfaceTemplate
import io.joynr.generator.templates.util.AttributeUtil
import io.joynr.generator.templates.util.InterfaceUtil
import io.joynr.generator.templates.util.MethodUtil
import io.joynr.generator.templates.util.NamingUtil
import org.franca.core.franca.FInterface

class DefaultInterfaceProviderHTemplate implements InterfaceTemplate{

	@Inject
	private extension TemplateBase

	@Inject
	private extension CppStdTypeUtil

	@Inject
	private extension NamingUtil

	@Inject
	private extension AttributeUtil

	@Inject
	private extension MethodUtil

	@Inject
	private extension InterfaceUtil

	@Inject
	private extension JoynrCppGeneratorExtensions

	override generate(FInterface serviceInterface)
'''
«val interfaceName = serviceInterface.joynrName»
«val headerGuard = ("GENERATED_INTERFACE_"+getPackagePathWithJoynrPrefix(serviceInterface, "_")+
	"_Default"+interfaceName+"Provider_h").toUpperCase»
«warning()»
#ifndef «headerGuard»
#define «headerGuard»

#include <functional>

«getDllExportIncludeStatement()»
#include "«getPackagePathWithJoynrPrefix(serviceInterface, "/")»/I«interfaceName».h"
#include "joynr/Logger.h"

«FOR parameterType: getRequiredIncludesFor(serviceInterface)»
	#include «parameterType»
«ENDFOR»

#include "«getPackagePathWithJoynrPrefix(serviceInterface, "/")»/«interfaceName»AbstractProvider.h"

«getNamespaceStarter(serviceInterface)»

class «getDllExportMacro()» Default«interfaceName»Provider : public «getPackagePathWithJoynrPrefix(serviceInterface, "::")»::«interfaceName»AbstractProvider {

public:
	Default«interfaceName»Provider();

	~Default«interfaceName»Provider() override;

	«IF !serviceInterface.attributes.empty»
		// attributes
	«ENDIF»
	«FOR attribute : serviceInterface.attributes»
		«var attributeName = attribute.joynrName»
		«IF attribute.readable»
			void get«attributeName.toFirstUpper»(
					std::function<void(
							const «attribute.typeName»&
					)> onSuccess,
					std::function<void (const joynr::exceptions::ProviderRuntimeException&)> onError
			) override;
		«ENDIF»
		«IF attribute.writable»
			void set«attributeName.toFirstUpper»(
					const «attribute.typeName»& «attributeName»,
					std::function<void()> onSuccess,
					std::function<void (const joynr::exceptions::ProviderRuntimeException&)> onError
			) override;
		«ENDIF»

	«ENDFOR»
	«val methodToErrorEnumName = serviceInterface.methodToErrorEnumName»
	«IF !serviceInterface.methods.empty»
		// methods
	«ENDIF»
	«FOR method : serviceInterface.methods»
		«val outputTypedParamList = method.commaSeperatedTypedConstOutputParameterList»
		«val inputTypedParamList = getCommaSeperatedTypedConstInputParameterList(method)»
		void «method.joynrName»(
				«IF !method.inputParameters.empty»
					«inputTypedParamList.substring(1)»,
				«ENDIF»
				«IF method.outputParameters.empty»
					std::function<void()> onSuccess,
				«ELSE»
					std::function<void(
							«outputTypedParamList.substring(1)»
					)> onSuccess,
				«ENDIF»
				«IF method.hasErrorEnum»
					«IF method.errors != null»
						«val packagePath = getPackagePathWithJoynrPrefix(method.errors, "::")»
						std::function<void (const «packagePath»::«methodToErrorEnumName.get(method)»::«nestedEnumName»& errorEnum)> onError
					«ELSE»
						std::function<void (const «method.errorEnum.typeName»& errorEnum)> onError
					«ENDIF»
				«ELSE»
				std::function<void (const joynr::exceptions::ProviderRuntimeException&)> onError
				«ENDIF»
		) override;

	«ENDFOR»
protected:
	«FOR attribute : getAttributes(serviceInterface)»
		«attribute.typeName» «attribute.joynrName»;
	«ENDFOR»

private:
	ADD_LOGGER(Default«interfaceName»Provider);

};

«getNamespaceEnder(serviceInterface)»

#endif // «headerGuard»
'''
}
