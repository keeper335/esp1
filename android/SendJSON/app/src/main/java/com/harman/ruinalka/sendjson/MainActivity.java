package com.harman.ruinalka.sendjson;

import android.app.Activity;
import android.app.Dialog;
import android.os.Bundle;
import android.support.v4.app.FragmentManager;
import android.support.v7.app.AlertDialog;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.Menu;
import android.view.View;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.ToggleButton;

import org.json.JSONException;
import org.json.JSONObject;

import java.sql.Connection;

public class MainActivity extends AppCompatActivity implements SendMain.AppWSListener, View.OnClickListener {

    ToggleButton b1;
    ToggleButton b2;
    TextView tv1;
    ImageView ivOn, ivOff;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        b1 = findViewById(R.id.button1);
        b2 = findViewById(R.id.button2);
        tv1 = findViewById(R.id.app_online_cap);
        ivOn = findViewById(R.id.imageViewOn);
        ivOff = findViewById(R.id.imageViewOff);

        LinearLayout conPan1 = findViewById(R.id.connectionPanel1);
        conPan1.setOnClickListener(this);

        SendMain app = (SendMain) getApplicationContext();
        app.setAppWebsocketListener(this);

        if (app.isWebsocketConnected()) {
            this.onWebsocketConnect();
        }
        else {
            this.onWebsocketDisconnect();
        }

    }

    protected void onDestroy() {
        super.onDestroy();

        SendMain app = (SendMain) getApplicationContext();
        app.setAppWebsocketListener(null);
    }

    public void onWebsocketMessage(String msg) {
        Toast.makeText(this, msg, Toast.LENGTH_LONG).show();
    }

    public void onWebsocketConnect() {
        if (tv1 != null) {
            tv1.setText(R.string.app_online);
        }

        if (ivOff != null)
            ivOff.setVisibility(View.INVISIBLE);
        if (ivOn != null)
            ivOn.setVisibility(View.VISIBLE);

        Toast toast1 = Toast.makeText(this, "Connection established", Toast.LENGTH_LONG);
        toast1.show();
    }

    public void onWebsocketDisconnect() {
        if (tv1 != null) {
            tv1.setText(R.string.app_offline);
        }

        if (ivOn != null)
            ivOn.setVisibility(View.INVISIBLE);
        if (ivOff != null)
            ivOff.setVisibility(View.VISIBLE);

    }

    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.activity_main, menu);
        return super.onCreateOptionsMenu(menu);
    }

    private void onB1Click() {
        JSONObject postData = new JSONObject();
        try {
            postData.put("name", "Hello!!");

            new SendJSONAsync().execute("http://10.0.2.2:3000/IOT", postData.toString());
        } catch (JSONException e) {
            e.printStackTrace();
        }
    }

    private void onB2Click() {
        SendMain app = (SendMain) getApplicationContext();
        WebsocketClient mWebSocketClient = app.getWS();

        mWebSocketClient.send("Test string 2");
    }

    private void showConnectionAlert() {
        ConnectionDialog dialog_ = new ConnectionDialog();
        FragmentManager fm = getSupportFragmentManager();
        dialog_.show(fm, "fragment_edit_address");
    }


    public void onClick(View v) {
        switch(v.getId()) {
            case R.id.button1:
                onB1Click();
                break;
            case R.id.button2:
                onB2Click();
                break;
            case R.id.connectionPanel1:
                showConnectionAlert();
                break;
            default:
        }
    }


}
