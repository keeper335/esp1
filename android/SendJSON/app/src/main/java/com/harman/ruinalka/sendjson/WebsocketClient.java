package com.harman.ruinalka.sendjson;

import android.os.Handler;
import android.os.HandlerThread;
import android.text.TextUtils;
import android.util.Base64;
import android.util.Log;

import java.io.EOFException;
import java.io.OutputStream;
import java.io.PrintWriter;
import java.io.IOException;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.SocketAddress;
import java.net.URI;
import java.net.Socket;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

public class WebsocketClient {
    private static final String TAG = "Websocket";
    private final Object mSendLock = new Object();

    private URI                      mURI;
    private Listener                 mListener;
    private Socket                   mSocket;
    private Thread                   mThread;
    private Handler                  mHandler;
    private WsParser mParser;

    WebsocketClient(URI uri, Listener listener) {
        mURI      = uri;
        mListener = listener;
        mParser   = new WsParser(this);

        HandlerThread mHandlerThread = new HandlerThread("websocket-thread");
        mHandlerThread.start();
        mHandler = new Handler(mHandlerThread.getLooper());
    }

    public void connect() {

        if (mThread != null && mThread.isAlive()) {
            return;
        }
        mThread = new Thread(new Runnable() {
            @Override
            public void run() {
                try {
                    String secret = createSecret();
                    InetAddress inetAddress = InetAddress.getByName(mURI.getHost());
                    SocketAddress socketAddress = new InetSocketAddress(inetAddress, mURI.getPort());
                    mSocket = new Socket();
                    mSocket.connect(socketAddress, 10*1000);
                    URI origin = new URI("http", "//" + mURI.getHost(), null);

                    PrintWriter out = new PrintWriter(mSocket.getOutputStream());
                    out.print("GET / HTTP/1.1\r\n");
                    out.print("Upgrade: websocket\r\n");
                    out.print("Connection: Upgrade\r\n");
                    out.print("Host: " + mURI.getHost() + "\r\n");
                    out.print("Origin: " + origin.toString() + "\r\n");
                    out.print("Sec-WebSocket-Key: " + secret + "\r\n");
                    out.print("Sec-WebSocket-Version: 13\r\n");
                    out.print("\r\n");
                    out.flush();

                    WsParser.DataInputStream stream = new WsParser.DataInputStream(mSocket.getInputStream());
                    // Read HTTP response headers.
                    String line;
                    //expect key validation
                    while(!TextUtils.isEmpty(line = readLine(stream))) {
                        Log.w("Stream header", line);
                        String[] header = line.split(": ");
                        if (header[0].contains("Sec-WebSocket-Accept")) {
                            String expected = createSecretValidation(secret);
                            if (!header[1].contains(expected))
                                throw new Exception("Bad Sec-WebSocket-Accept header value");
                        }
                    }

                    mListener.onConnect();
                    mParser.start(stream);

                }
                catch (EOFException ee) {
                    Log.w(TAG, "WebSocket EOF!");
                    mListener.onDisconnect(0, "EOF");
                }
                catch (IOException ee) {
                    Log.w(TAG, "Websocket IO error!");
                    mListener.onDisconnect(0, "IO");
                }
                catch (Exception ee) {
                    Log.w(TAG, "Websocket error!");
                    mListener.onDisconnect(0, "Exception");
                }
            }
        });
        mThread.start();
    }

    public void disconnect() {
        if (mSocket != null) {
            mHandler.post(new Runnable() {
                @Override
                public void run() {
                    try {
                        mSocket.close();
                        mSocket = null;
                    } catch (IOException ex) {
                        Log.d(TAG, "Error while disconnecting", ex);
                        mListener.onError(ex);
                    }
                }
            });
        }
    }

    public boolean isConnected() {
        return mSocket != null && mSocket.isConnected();
    }

    public void send(String data) {
        sendFrame(mParser.frame(data));
    }

    public void send(byte[] data) {
        sendFrame(mParser.frame(data));
    }

    private void sendFrame(final byte[] frame) {
        mHandler.post(new Runnable() {
            @Override
            public void run() {
                try {
                    synchronized (mSendLock) {
                        OutputStream outputStream = mSocket.getOutputStream();
                        outputStream.write(frame);
                        outputStream.flush();
                    }
                } catch (IOException e) {
                    mListener.onError(e);
                } catch (NullPointerException e) {
                    mListener.onError(e);
                }

            }
        });
    }

    public Listener getListener() {
        return mListener;
    }


    public interface Listener {
        void onConnect();
        void onMessage(String message);
        void onMessage(byte[] data);
        void onDisconnect(int code, String reason);
        void onError(Exception error);
    }

    private String createSecret() {
        byte[] nonce = new byte[16];
        for (int i = 0; i < 16; i++) {
            nonce[i] = (byte) (Math.random() * 256);
        }
        return Base64.encodeToString(nonce, Base64.DEFAULT).trim();
    }

    private String createSecretValidation(String secret) {
        try {
            MessageDigest md = MessageDigest.getInstance("SHA-1");
            md.update((secret + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11").getBytes());
            return Base64.encodeToString(md.digest(), Base64.DEFAULT).trim();
        } catch (NoSuchAlgorithmException e) {
            throw new RuntimeException(e);
        }
    }

    private String readLine(java.io.DataInputStream reader) throws IOException {
        int readChar = reader.read();
        if (readChar == -1) {
            return null;
        }
        StringBuilder string = new StringBuilder("");
        while (readChar != '\n' && readChar != 0) {
            if (readChar != '\r') {
                string.append((char) readChar);
            }

            readChar = reader.read();
            if (readChar == -1) {
                return null;
            }
        }
        return string.toString();
    }

}
