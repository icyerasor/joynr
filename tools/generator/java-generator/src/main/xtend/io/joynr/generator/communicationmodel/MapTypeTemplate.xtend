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
import io.joynr.generator.templates.MapTemplate
import io.joynr.generator.templates.util.NamingUtil
import io.joynr.generator.util.JavaTypeUtil
import io.joynr.generator.util.JoynrJavaGeneratorExtensions
import io.joynr.generator.util.TemplateBase
import org.franca.core.franca.FMapType
import org.franca.core.franca.FType

class MapTypeTemplate implements MapTemplate {

	@Inject	extension JoynrJavaGeneratorExtensions
	@Inject extension JavaTypeUtil
	@Inject extension TemplateBase
	@Inject extension NamingUtil

	override generate(FMapType mapType) {
		val typeName = mapType.joynrName
		val mapTypePackageName = mapType.buildPackagePath(".", true)

'''
«warning()»

package «mapTypePackageName»;
import java.util.HashMap;

import io.joynr.subtypes.JoynrType;

«val keyType = getDatatype(mapType.keyType)»
«val valueType = getDatatype(mapType.valueType)»
«IF keyType instanceof FType»
import «getIncludeOf(keyType)»;
«ENDIF»
«IF valueType instanceof FType»
import «getIncludeOf(valueType)»;
«ENDIF»

// NOTE: serialVersionUID is not defined since we don't support Franca versions right now.
//       The compiler will generate a serialVersionUID based on the class and its members
//       (cf. http://docs.oracle.com/javase/6/docs/platform/serialization/spec/class.html#4100),
//       which is probably more restrictive than what we want.

/**
«appendJavadocSummaryAndWriteSeeAndDescription(mapType, " *")»
 */
@SuppressWarnings("serial")
public class «typeName» extends HashMap<«mapType.keyType.typeName», «mapType.valueType.typeName»> implements JoynrType {
    public «typeName»() {
        super();
    }

    public «typeName»(«typeName» other) {
        super(other);
    }
}
'''
}

}
