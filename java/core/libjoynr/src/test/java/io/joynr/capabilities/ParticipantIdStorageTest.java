package io.joynr.capabilities;

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

import static org.junit.Assert.assertEquals;

import com.google.inject.AbstractModule;
import com.google.inject.Guice;
import io.joynr.common.JoynrPropertiesModule;
import io.joynr.proxy.RpcStubbingTest.TestProvider;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.runners.MockitoJUnitRunner;

import com.google.inject.Injector;

import java.util.Properties;

@RunWith(MockitoJUnitRunner.class)
public class ParticipantIdStorageTest {
    private static final String TOKEN1_PARTICIPANT = "token1Participant";
    private static final String TOKEN2_PARTICIPANT = "token2Participant";
    ParticipantIdStorage storage;

    @Before
    public void setUp() {
        Injector injector = Guice.createInjector(new AbstractModule() {
            @Override
            protected void configure() {
                bind(ParticipantIdStorage.class).to(PropertiesFileParticipantIdStorage.class);
            }
        }, new JoynrPropertiesModule(new Properties()));
        storage = injector.getInstance(ParticipantIdStorage.class);
    }

    @Test
    public void test() {
        storage.getProviderParticipantId("domain", TestProvider.class, TOKEN1_PARTICIPANT);
        String participant2 = storage.getProviderParticipantId("domain", TestProvider.class, TOKEN2_PARTICIPANT);

        assertEquals(TOKEN2_PARTICIPANT, participant2);
    }
}
