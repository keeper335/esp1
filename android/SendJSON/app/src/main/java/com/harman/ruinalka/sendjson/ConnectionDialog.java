package com.harman.ruinalka.sendjson;

import android.os.Bundle;
import android.support.annotation.NonNull;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;

import java.net.URISyntaxException;

public class ConnectionDialog extends android.support.v4.app.DialogFragment implements View.OnClickListener {
    private EditText mEditView;
    private android.app.Activity mActivity;
    @Override
    public View onCreateView(@NonNull LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        super.onCreateView(inflater, container, savedInstanceState);
        View view = inflater.inflate(R.layout.dialog_connection, container);

        mActivity = getActivity();
        SendMain app_;
        String mUri = null;
        if (mActivity != null) {
            app_ = (SendMain) mActivity.getApplicationContext();
            if (app_ != null) {
                mUri = app_.getWSUri();
            }
        }

        mEditView = view.findViewById(R.id.dialog_connection_editText);
        if (mUri != null && mEditView != null) {
            mEditView.setText(mUri);
        }
        Button btn_ = view.findViewById(R.id.dialog_connection_btnOk);
        btn_.setOnClickListener(this);
        btn_ = view.findViewById(R.id.dialog_connection_btnCancel);
        btn_.setOnClickListener(this);

        getDialog().setTitle("Connection");
        return view;
    }

    @Override
    public void onClick(View v) {
        try {
            switch(v.getId()) {
                case R.id.dialog_connection_btnOk:
                    if (mEditView != null && mActivity != null) {
                        SendMain app_ = (SendMain) mActivity.getApplicationContext();
                        if (app_ != null) {
                            app_.setWSUri(mEditView.getText().toString());
                            Toast toast1 = Toast.makeText(mActivity, "Host changed", Toast.LENGTH_LONG);
                            toast1.show();
                        }

                    }
                case R.id.dialog_connection_btnCancel:
                default:
            }
        } catch (URISyntaxException ee) {
            Toast toast1 = Toast.makeText(mActivity, "Host wrong syntax", Toast.LENGTH_LONG);
            toast1.show();
        } catch (Exception ee) {
            Toast toast1 = Toast.makeText(mActivity, "Host empty", Toast.LENGTH_LONG);
            toast1.show();
        } finally {
            this.dismiss();
        }
    }

}
