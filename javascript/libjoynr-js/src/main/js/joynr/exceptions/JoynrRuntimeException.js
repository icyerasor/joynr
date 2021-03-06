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

define("joynr/exceptions/JoynrRuntimeException", [
    "joynr/types/TypeRegistrySingleton",
    "joynr/util/UtilInternal",
    "joynr/exceptions/JoynrException",
    "joynr/system/LoggerFactory"
], function(TypeRegistrySingleton, Util, JoynrException, LoggerFactory) {
    var defaultSettings;

    /**
     * @classdesc
     *
     * @summary
     * Constructor of JoynrRuntimeException object used for reporting
     * error conditions. This serves as superobject for other more specific
     * runtime exception objects and inherits from JoynrException.
     *
     * @constructor
     * @name JoynrRuntimeException
     *
     * @param {Object}
     *            [settings] the settings object for the constructor call
     * @param {String}
     *            [settings.detailMessage] message containing details
     *            about the error
     * @returns {JoynrRuntimeException}
     *            The newly created IllegalAccessException object
     */
    function JoynrRuntimeException(settings) {
        if (!(this instanceof JoynrRuntimeException)) {
            // in case someone calls constructor without new keyword (e.g. var c
            // = Constructor({..}))
            return new JoynrRuntimeException(settings);
        }

        var log = LoggerFactory.getLogger("joynr.exceptions.JoynrRuntimeException");
        var exception = new JoynrException(settings);

        /**
         * Used for serialization.
         * @name JoynrRuntimeException#_typeName
         * @type String
         * @field
         */
        Util.objectDefineProperty(this, "_typeName", "joynr.exceptions.JoynrRuntimeException");
        Util.checkPropertyIfDefined(settings, "Object", "settings");
        Util.extend(this, defaultSettings, settings, exception);
    }

    defaultSettings = {};

    TypeRegistrySingleton.getInstance().addType(
            "joynr.exceptions.JoynrRuntimeException",
            JoynrRuntimeException);

    JoynrRuntimeException.prototype = new Error();
    JoynrRuntimeException.prototype.constructor = JoynrRuntimeException;
    JoynrRuntimeException.prototype.name = "JoynrRuntimeException";

    return JoynrRuntimeException;

});
