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

define("joynr/capabilities/ParticipantIdStorage", [], function() {

    /**
     * @constructor
     * @name ParticipantIdStorage
     *
     * @param {Persistency} persistency - the persistence object to be used to store the participantIds
     * @param {Function} uuid - a function generating a uuid string
     */
    function ParticipantIdStorage(persistency, uuid) {

        /**
         * @function
         * @name ParticipantIdStorage#getParticipantId
         *
         * @param {String}
         *            authToken - currently used to differentiate between multiple providers for the same domain/interface. Use an empty
         *            string if only registering one provider per domain/interface
         * @param {String}
         *            domain
         * @param {Object}
         *            provider
         * @param {String}
         *            provider.interfaceName
         *
         * @returns {String} the retrieved or generated participantId
         */
        this.getParticipantId = function getParticipantId(authToken, domain, provider) {
            var interfaceNameDotted = provider.interfaceName.replace("/", ".");
            var key = "joynr.participant." + domain + "." + interfaceNameDotted + "." + authToken;
            var participantId = persistency.getItem(key);
            if (!participantId) {
                participantId = domain + "." + interfaceNameDotted + "." + uuid();
                persistency.setItem(key, participantId);
            }
            return participantId;
        };

    }

    return ParticipantIdStorage;

});