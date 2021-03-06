package io.joynr.capabilities;

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

import java.io.Serializable;
import java.util.Arrays;

import javax.persistence.Entity;
import javax.persistence.Id;
import javax.persistence.Lob;
import javax.persistence.Table;

import joynr.types.CustomParameter;
import joynr.types.ProviderQos;
import joynr.types.ProviderScope;

import com.google.common.base.Function;
import com.google.common.collect.Collections2;

@Entity
@Table(name = "providerqos")
public class ProviderQosPersisted extends ProviderQos implements Serializable {
    private static final long serialVersionUID = 1L;

    public ProviderQosPersisted() {
    }

    private String participantId;

    public ProviderQosPersisted(final String participantId, ProviderQos providerQos) {
        super(Collections2.transform(Arrays.asList(providerQos.getCustomParameters()),
                                     new Function<CustomParameter, CustomParameterPersisted>() {
                                         @Override
                                         public CustomParameterPersisted apply(CustomParameter input) {
                                             return new CustomParameterPersisted(participantId, input);
                                         }

                                     }).toArray(new CustomParameter[providerQos.getCustomParameters().length]),
              providerQos.getProviderVersion(),
              providerQos.getPriority(),
              providerQos.getScope(),
              providerQos.getSupportsOnChangeSubscriptions());
        this.participantId = participantId;
    }

    @Id
    protected final String getParticipantId() {
        return participantId;
    }

    protected final void setParticipantId(String participantId) {
        this.participantId = participantId;
    }

    @Override
    @Lob
    public CustomParameter[] getCustomParameters() {
        return super.getCustomParameters();
    }

    @Override
    public void setCustomParameters(CustomParameter[] customParameters) {
        super.setCustomParameters(customParameters);
    }

    @Override
    public Long getPriority() {
        return super.getPriority();
    }

    @Override
    public void setPriority(Long priority) {
        super.setPriority(priority);
    }

    @Override
    public Integer getProviderVersion() {
        return super.getProviderVersion();
    }

    @Override
    public void setProviderVersion(Integer providerVersion) {
        super.setProviderVersion(providerVersion);
    }

    @Override
    public ProviderScope getScope() {
        return super.getScope();
    }

    @Override
    public void setScope(ProviderScope scope) {
        super.setScope(scope);
    }

    @Override
    public Boolean getSupportsOnChangeSubscriptions() {
        return super.getSupportsOnChangeSubscriptions();
    }

    @Override
    public void setSupportsOnChangeSubscriptions(Boolean supportsOnChangeSubscriptions) {
        super.setSupportsOnChangeSubscriptions(supportsOnChangeSubscriptions);
    }

    @Override
    public int hashCode() {
        final int prime = 31;
        int result = super.hashCode();
        result = prime * result + ((participantId == null) ? 0 : participantId.hashCode());
        return result;
    }

    @Override
    public boolean equals(Object obj) {
        if (this == obj) {
            return true;
        }
        if (!super.equals(obj)) {
            return false;
        }
        if (getClass() != obj.getClass()) {
            return false;
        }
        ProviderQosPersisted other = (ProviderQosPersisted) obj;
        if (participantId == null) {
            if (other.participantId != null) {
                return false;
            }
        } else if (!participantId.equals(other.participantId)) {
            return false;
        }
        return true;
    }
}
