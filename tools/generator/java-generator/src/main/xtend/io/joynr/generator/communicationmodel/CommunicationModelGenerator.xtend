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
import io.joynr.generator.templates.util.NamingUtil
import io.joynr.generator.templates.util.TypeUtil
import io.joynr.generator.util.JoynrJavaGeneratorExtensions
import java.io.File
import org.eclipse.xtext.generator.IFileSystemAccess
import org.franca.core.franca.FMapType
import org.franca.core.franca.FModel

class CommunicationModelGenerator {

	@Inject
	extension JoynrJavaGeneratorExtensions

	@Inject
	private extension TypeUtil

	@Inject
	private extension NamingUtil

	@Inject
	MapTypeTemplate mapTemplate

	@Inject
	EnumTypeTemplate enumTemplate

	@Inject
	ComplexTypeTemplate complexTypeTemplate

	def doGenerate(FModel fModel, IFileSystemAccess fsa){
		for( type: getCompoundDataTypes(fModel)){
			var path = getPackagePathWithJoynrPrefix(type, File::separator) + File::separator
			if (type.isPartOfTypeCollection) {
				path += type.typeCollectionName + File::separator
			}
			generateFile(
				fsa,
				path + type.joynrName + ".java",
				complexTypeTemplate,
				type
			)
		}

		for( type: getEnumDataTypes(fModel)){
			var path = getPackagePathWithJoynrPrefix(type, File::separator) + File::separator
			if (type.isPartOfTypeCollection) {
				path += type.typeCollectionName + File::separator
			}
			generateFile(
				fsa,
				path + type.joynrName + ".java",
				enumTemplate,
				type
			)
		}

		for( type: getMapDataTypes(fModel)){
			var path = getPackagePathWithJoynrPrefix(type, File::separator) + File::separator
			if (type.isPartOfTypeCollection) {
				path += type.typeCollectionName + File::separator
			}
			generateFile(
				fsa,
				path + type.joynrName + ".java",
				mapTemplate,
				type
			)
		}
	}
}
