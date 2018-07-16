package com.harman.ruinalka.sendjson;

import android.app.Application;
import android.content.SharedPreferences;
import android.os.Handler;
import android.os.Looper;
import android.preference.PreferenceManager;
import android.util.Log;

import java.net.URI;
import java.net.URISyntaxException;

public class SendMain extends Application {

    public static final String PREFS_API_HOST = "api_host";
    public static final String PREFS_API_PORT = "api_port";
    public static final String PREFS_API_WS_PORT = "api_socket_port";

    private SharedPreferences prefs;
    //private String USER_KEY;
    //private String HTTP_PORT;

    private Handler connectHandler = new Handler();
    private WebsocketClient mWebSocketClient;
    private AppWSListener mActivityListener;

    private Handler looper = new Handler(Looper.getMainLooper());

    Runnable connectTimer = new Runnable() {
        @Override
        public void run() {
            Log.w("Main thread", "trying to re-connect...");
            mWebSocketClient.connect();
        }
    };

    public WebsocketClient.Listener mListener = new WebsocketClient.Listener() {
        @Override
        public void onConnect() {
            Log.w("Main thread", "Socket connected");
            //Make call in main thread
            looper.post(new Runnable() {
                @Override
                public void run() {
                    if (mActivityListener != null)
                        mActivityListener.onWebsocketConnect();
                }
            });
        }
        @Override
        public void onError(Exception error) {
            Log.w("Main thread", "Socket error");
        }
        @Override
        public void onMessage(String message) {
            Log.w("Main thread onmsg str", message);
            final String msg = message;
            //Make call in main thread
            looper.post(new Runnable() {
                @Override
                public void run() {
                    if (mActivityListener != null)
                        mActivityListener.onWebsocketMessage(msg);
                }
            });
        }
        @Override
        public void onMessage(byte[] data) {
            Log.w("Main thread onmsg bytes", new String(data));
        }
        @Override
        public void onDisconnect(int code, String reason) {
            Log.w("Main thread", "Socket disconnected");
            connectHandler.postDelayed(connectTimer, 10000);
            looper.post(new Runnable() {
                @Override
                public void run() {
                    if (mActivityListener != null)
                        mActivityListener.onWebsocketDisconnect();
                }
            });
        }
    };


    @Override
    public void onCreate() {
        super.onCreate();

        //USER_KEY = this.getString(R.string.user_key);
        prefs = PreferenceManager.getDefaultSharedPreferences(this);
        createWebSocket();
        connectHandler.postDelayed(connectTimer, 3000);
    }

    @Override
    public void onTerminate() {
        super.onTerminate();
        mWebSocketClient.disconnect();
    }

    public WebsocketClient getWS() {
        return mWebSocketClient;
    }

    public boolean isWebsocketConnected() {
        return mWebSocketClient != null && mWebSocketClient.isConnected();
    }

    public interface AppWSListener {
        void onWebsocketConnect();
        void onWebsocketMessage(String message);
        void onWebsocketDisconnect();
    }

    void setAppWebsocketListener(AppWSListener listener) {
        mActivityListener = listener;
    }

    public String getWSUri() {
        return prefs.getString(PREFS_API_HOST, this.getString(R.string.api_host)) +
                ":" +
                prefs.getString(PREFS_API_WS_PORT, this.getString(R.string.api_socket_port));
    }

    public void setWSUri(String sUri) throws Exception, URISyntaxException {
        URI uri = new URI("ws://" + sUri);
        String host_ = uri.getHost();
        int port_ = uri.getPort();
        if (host_ != null) {
            SharedPreferences.Editor prefEditor = prefs.edit();
            prefEditor.putString(PREFS_API_HOST, host_);
            if (port_ > 0) {
                prefEditor.putString(PREFS_API_WS_PORT, Integer.toString(port_));
            }

            prefEditor.apply();
            Log.w("Main thread", "Reconnect using new URI");
            mWebSocketClient.disconnect();
            createWebSocket();


        }
        else throw new URISyntaxException("", "No host passed");

    }

    private void createWebSocket() {
        String uri = this.getWSUri();
        Log.w("Main thread", uri);
        mWebSocketClient = new WebsocketClient(URI.create("ws://" + uri), mListener);
    }
}
