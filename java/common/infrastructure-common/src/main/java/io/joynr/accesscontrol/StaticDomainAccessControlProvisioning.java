package io.joynr.accesscontrol;

/*
 * #%L
 * %%
 * Copyright (C) 2011 - 2013 BMW Car IT GmbH
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

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

import joynr.infrastructure.DomainRoleEntry;
import joynr.infrastructure.MasterAccessControlEntry;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.fasterxml.jackson.core.type.TypeReference;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.google.inject.Inject;
import com.google.inject.name.Named;

public class StaticDomainAccessControlProvisioning implements DomainAccessControlProvisioning {
    public static final String PROPERTY_PROVISIONED_DOMAIN_ROLES = "joynr.accesscontrol.provisioned.domainroles";
    public static final String PROPERTY_PROVISIONED_MASTER_ACCESSCONTROLENTRIES = "joynr.accesscontrol.provisioned.masteraces";
    private List<DomainRoleEntry> domainRoleEntries = new ArrayList<DomainRoleEntry>();
    private List<MasterAccessControlEntry> masterControlEntries = new ArrayList<MasterAccessControlEntry>();
    private static Logger logger = LoggerFactory.getLogger(StaticDomainAccessControlProvisioning.class);

    @SuppressWarnings("unchecked")
    @Inject(optional = true)
    private void loadDomainRoleEntries(@Named(PROPERTY_PROVISIONED_DOMAIN_ROLES) String entries,
                                       ObjectMapper objectMapper) {
        List<DomainRoleEntry> newEntries = null;
        try {
            newEntries = objectMapper.readValue(entries, new TypeReference<List<DomainRoleEntry>>() {
            });
            for (DomainRoleEntry domainRole : newEntries) {
                domainRoleEntries.add(domainRole);
            }
        } catch (Exception e) {
            logger.error("unable to load provisioned domain roles. "
                    + (newEntries != null ? "to be processed entry: " + newEntries : ""), e);
        }
    }

    @SuppressWarnings("unchecked")
    @Inject(optional = true)
    private void loadMasterAccessControlEntries(@Named(PROPERTY_PROVISIONED_MASTER_ACCESSCONTROLENTRIES) String entries,
                                                ObjectMapper objectMapper) {
        List<MasterAccessControlEntry> newEntries = null;
        try {
            newEntries = objectMapper.readValue(entries, new TypeReference<List<MasterAccessControlEntry>>() {
            });
            for (MasterAccessControlEntry masterControlEntry : newEntries) {
                masterControlEntries.add(masterControlEntry);
            }
        } catch (Exception e) {
            logger.error("unable to load provisioned master access control entries. "
                    + (newEntries != null ? "to be processed entry: " + newEntries : ""), e);
        }
    }

    @Override
    public Collection<DomainRoleEntry> getDomainRoleEntries() {
        return domainRoleEntries;
    }

    @Override
    public Collection<MasterAccessControlEntry> getMasterAccessControlEntries() {
        return masterControlEntries;
    }
}