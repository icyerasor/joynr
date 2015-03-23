package io.joynr.examples.android_location_provider;

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

import io.joynr.messaging.MessagingPropertyKeys;

import java.util.Properties;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

public class LocationProviderActivity extends Activity {

    private static final String TAG = "android-location-provider";
    private JoynrAndroidLocationProviderApplication application;
    private boolean runtimeStarted = false;
    private boolean providerRegistered = false;

    /**
     * Called when the activity is first created.
     * 
     * @param savedInstanceState
     *            If the activity is being re-initialized after previously being shut down then this Bundle contains the
     *            data it most recently supplied in onSaveInstanceState(Bundle). <b>Note: Otherwise it is null.</b>
     */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.i(TAG, "onCreate");
        setContentView(R.layout.main);
        application = (JoynrAndroidLocationProviderApplication) getApplication();

        application.setOutput(new Output() {
            TextView textView = (TextView) findViewById(R.id.progressoutput);

            @Override
            public void append(final String text) {
                // Android prohibits access UI elements from outside the UI thread
                // with "someUiElement".post(..) you can pass a runnable to the ui thread to modify UI content.
                // see http://developer.android.com/guide/components/processes-and-threads.html
                textView.post(new Runnable() {
                    @Override
                    public void run() {
                        textView.append(text);
                    }
                });
            }

        });

    }

    public void btnOnClickStartJoynrRuntime(View v) {
        runtimeStarted = true;
        EditText editTextBackendHost = (EditText) findViewById(R.id.editTextBackendHost);
        String backendHost = editTextBackendHost.getText().toString();
        editTextBackendHost.setEnabled(false);
        Log.i(TAG, "Bounceproxy URL: " + backendHost);

        Properties joynrConfig = new Properties();
        joynrConfig.setProperty(MessagingPropertyKeys.BOUNCE_PROXY_URL, "http://" + backendHost + "/bounceproxy/");
        joynrConfig.setProperty(MessagingPropertyKeys.CHANNELURLDIRECTORYURL, "http://" + backendHost
                + "/discovery/channels/discoverydirectory_channelid/");
        joynrConfig.setProperty(MessagingPropertyKeys.CAPABILITIESDIRECTORYURL, "http://" + backendHost
                + "/discovery/channels/discoverydirectory_channelid/");
        application.initJoynrRuntime(joynrConfig);
    }

    public void btnOnClickRegisterLocationProvider(View v) {
        if (!runtimeStarted) {
            btnOnClickStartJoynrRuntime(v);
        }
        EditText editTextDomain = (EditText) findViewById(R.id.editTextDomain);
        String domain = editTextDomain.getText().toString();
        Log.i(TAG, "Domain: " + domain);
        Button btnRegisterLocationProvider = (Button) findViewById(R.id.btnRegisterLocationProvider);
        if (providerRegistered) {
            application.unregisterProvider(domain);
            btnRegisterLocationProvider.setText("Register Location Provider");
            editTextDomain.setEnabled(true);
            providerRegistered = false;
        } else {
            application.registerProvider(domain);
            btnRegisterLocationProvider.setText("Unregister Location Provider");
            editTextDomain.setEnabled(false);
            providerRegistered = true;
        }
    }
}