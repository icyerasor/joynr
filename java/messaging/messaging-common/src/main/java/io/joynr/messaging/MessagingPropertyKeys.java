package io.joynr.messaging;

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

public final class MessagingPropertyKeys {
    public static final String CHANNELID = "joynr.messaging.channelid"; //NOT USUALLY SET BY THE APPLICATION!
    public static final String CAPABILITIESDIRECTORYURL = "joynr.messaging.capabilitiesdirectoryurl";
    public static final String CHANNELURLDIRECTORYURL = "joynr.messaging.channelurldirectoryurl";
    public static final String BOUNCE_PROXY_URL = "joynr.messaging.bounceproxyurl";
    public static final String RECEIVERID = "joynr.messaging.receiverid"; //NEVER SET BY THE APPLICATION!
    public static final String PERSISTENCE_FILE = "joynr.messaging.persistence_file";
    public static final String DEFAULT_PERSISTENCE_FILE = "joynr.properties";
    public static final String DEFAULT_MESSAGING_PROPERTIES_FILE = "defaultMessaging.properties";

    public static final String JOYNR_PROPERTIES = "joynr.properties";

    //NOTE: all property identifiers must be lower-case only.
    public static final String PROPERTY_SERVLET_CONTEXT_ROOT = "joynr.servlet.context.root";
    public static final String PROPERTY_SERVLET_SHUTDOWN_TIMEOUT = "joynr.servlet.shutdown.timeout";
    public static final String PROPERTY_SERVLET_HOST_PATH = "joynr.servlet.hostpath";
    public static final String PROPERTY_SERVLET_SKIP_LONGPOLL_DEREGISTRATION = "joynr.servlet.skiplongpollderegistration";

    private MessagingPropertyKeys() {
        throw new AssertionError();
    }
}
