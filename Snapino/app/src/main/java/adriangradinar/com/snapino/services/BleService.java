package adriangradinar.com.snapino.services;

import android.app.ActionBar;
import android.app.Service;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothManager;
import android.bluetooth.BluetoothProfile;
import android.bluetooth.le.BluetoothLeScanner;
import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanFilter;
import android.bluetooth.le.ScanResult;
import android.bluetooth.le.ScanSettings;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.Messenger;
import android.os.RemoteException;
import android.util.Log;

import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.UUID;

import adriangradinar.com.snapino.Classes.HistoricDataEntry;
import adriangradinar.com.snapino.Classes.LiveDataEntry;
import adriangradinar.com.snapino.Classes.Utils;
import adriangradinar.com.snapino.Database.DatabaseHandler;
import adriangradinar.com.snapino.R;

public class BleService extends Service {

    private final static String TAG = BleService.class.getSimpleName();
    public static final int MSG_REGISTER = 1;
    public static final int MSG_UNREGISTER = 2;
    public static final int MSG_START_SCAN = 3;
    public static final int MSG_STATE_CHANGED = 4;
    public static final int MSG_DEVICE_FOUND = 5;
    public static final int MSG_DEVICE_CONNECT = 6;
    public static final int MSG_DEVICE_DISCONNECT = 7;
    public static final int MSG_DEVICE_DATA = 8;
    public static final int MSG_WRITE_DATA = 9;
    public static final String KEY_MAC_ADDRESSES = "KEY_MAC_ADDRESSES";

    private static final long SCAN_PERIOD = 3000;

    private static final String DEVICE_NAME = "Snapino";
    public static final UUID RFDUINO_SERVICE_UUID = UUID.fromString("00002220-0000-1000-8000-00805F9B34FB");
    public static final UUID RECEIVE_CHARACTERISTIC_UUID = UUID.fromString("00002221-0000-1000-8000-00805F9B34FB");
    public static final UUID SEND_CHARACTERISTIC_UUID = UUID.fromString("00002222-0000-1000-8000-00805F9B34FB");
    public static final UUID DISCONNECT_CHARACTERISTIC_UUID = UUID.fromString("00002223-0000-1000-8000-00805F9B34FB");
    // 0x2902 org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
    public final static UUID CLIENT_CHARACTERISTIC_CONFIGURATION_UUID = UUID.fromString("00002902-0000-1000-8000-00805F9B34FB");

    private final IncomingHandler mHandler;
    private final Messenger mMessenger;
    private final List<Messenger> mClients = new LinkedList<>();
    private static final Map<String, BluetoothDevice> mDevices = new HashMap<>();

    private BluetoothLeScanner mLEScanner;
    private ScanSettings settings;
    private List<ScanFilter> filters;


    private BluetoothGatt mGatt = null;
    private BluetoothGattService gattService;
    private BluetoothGattCharacteristic receiveCharacteristic;
    private BluetoothGattCharacteristic sendCharacteristic;
    private BluetoothGattCharacteristic disconnectCharacteristic;

    public enum State {
        UNKNOWN,
        IDLE,
        SCANNING,
        BLUETOOTH_OFF,
        CONNECTING,
        CONNECTED,
        DISCONNECTING
    }

    private BluetoothAdapter mBluetooth = null;
    private State mState = State.UNKNOWN;

    private DatabaseHandler db;
    private static SharedPreferences sharedPreferences;
    private boolean isStartTimeSet = false;

    public BleService() {
        mHandler = new IncomingHandler(this);
        mMessenger = new Messenger(mHandler);
    }

    private String[] receivedValues;
    private int value1, value2, diff;
    private long startTime, lastStartTime;
    private int sentFlag;

    @Override
    public void onCreate() {
        super.onCreate();

        db = new DatabaseHandler(this);
//        db.deleteAllRecords();
        sharedPreferences = getSharedPreferences(getString(R.string.shared_preferences), 0);
    }

    @Override
    public IBinder onBind(Intent intent) {
        return mMessenger.getBinder();
    }

    private static class IncomingHandler extends Handler {
        private final WeakReference<BleService> mService;

        public IncomingHandler(BleService service) {
            mService = new WeakReference<>(service);
        }

        @Override
        public void handleMessage(Message msg) {
            BleService service = mService.get();
            Log.e(TAG, "Sent message: " + msg.what);
            if (service != null) {
                switch (msg.what) {
                    case MSG_REGISTER:
                        service.mClients.add(msg.replyTo);
                        Log.d(TAG, "Registered");
                        break;
                    case MSG_UNREGISTER:
                        Log.e(TAG, "Remove messenger!");
                        service.mClients.remove(msg.replyTo);
                        if (service.mState == State.CONNECTED && service.mGatt != null) {
                            service.mGatt.disconnect();
                        }
                        Log.d(TAG, "Unregistered");
                        break;
                    case MSG_START_SCAN:
                        service.startScan();
                        Log.d(TAG, "Start Scan");
                        break;
                    case MSG_DEVICE_CONNECT:
                        for(BluetoothDevice device : mDevices.values()){
                            if((msg.obj).equals(device.getName())){
                                service.connect(device.getAddress());
                            }
                        }
                        //service.connect((String) msg.obj);
                        break;
                    case MSG_DEVICE_DISCONNECT:
                        if (service.mState == State.CONNECTED && service.mGatt != null) {
                            service.mGatt.disconnect();
                        }
                        break;
                    case MSG_WRITE_DATA:
                        Log.e(TAG, "Send request!");
                        if (service.mState == State.CONNECTED && service.mGatt != null) {
                            Bundle bundle = msg.getData();
                            service.writeDataToCharacteristic(service.mGatt, service.sendCharacteristic, bundle.getInt("flag"));
                            sharedPreferences.edit().putInt("sentFlag", bundle.getInt("flag")).commit();
                        }
                        break;
                    default:
                        super.handleMessage(msg);
                }
            }
        }
    }

    private ScanCallback mScanCallback = new ScanCallback() {
        @Override
        public void onScanResult(int callbackType, ScanResult result) {
            BluetoothDevice device = result.getDevice();
            if (device != null && !mDevices.containsValue(device) && device.getName() != null) {
                mDevices.put(device.getAddress(), device);
                Message msg = Message.obtain(null, MSG_DEVICE_FOUND);
                if (msg != null) {
                    Bundle bundle = new Bundle();

                    //The Old way via the BL address
                    //String[] addresses = mDevices.keySet().toArray(new String[mDevices.size()]);

                    //the new way via the device name
                    String[] addresses = new String[mDevices.size()];
                    int i = 0;
                    for(BluetoothDevice device1 : mDevices.values()){
                        addresses[i++] = (device1.getName());
                    }

                    bundle.putStringArray(KEY_MAC_ADDRESSES, addresses);
                    msg.setData(bundle);
                    sendMessage(msg);
                }
                Log.d(TAG, "Added " + device.getName() + ": " + device.getAddress());
            }
        }

        @Override
        public void onBatchScanResults(List<ScanResult> results) {
            Log.e(TAG, "onBatchResultCalled");
            for (ScanResult sr : results) {
                Log.i("ScanResult - Results", sr.toString());
            }
        }

        @Override
        public void onScanFailed(int errorCode) {
            Log.e("Scan Failed", "Error Code: " + errorCode);
        }
    };

    private void startScan() {
        mDevices.clear();
        setState(State.SCANNING);

        if (mBluetooth == null) {
            BluetoothManager bluetoothMgr = (BluetoothManager) getSystemService(BLUETOOTH_SERVICE);
            mBluetooth = bluetoothMgr.getAdapter();
        }
        if (mBluetooth == null || !mBluetooth.isEnabled()) {
//            mBluetooth.enable();
            setState(State.BLUETOOTH_OFF);
            Log.e(TAG, "Darn, our user has not turned on the bluetooth but still wants to get data from... Damn... Magic is not happening... Yet");
        }
        else {

            mLEScanner = mBluetooth.getBluetoothLeScanner();
            settings = new ScanSettings.Builder().setScanMode(ScanSettings.SCAN_MODE_LOW_LATENCY).build();
            filters = new ArrayList<>();

            mHandler.postDelayed(new Runnable() {
                @Override
                public void run() {
                    if (mState == State.SCANNING) {
                        mLEScanner.stopScan(mScanCallback);
                        setState(State.IDLE);
                    }
                }
            }, SCAN_PERIOD);
            mLEScanner.startScan(filters, settings, mScanCallback);
        }
    }

    public void connect(String macAddress) {
        BluetoothDevice device = mDevices.get(macAddress);
        if (device != null) {
            mGatt = device.connectGatt(this, true, gattCallback);
        }
    }

    private final BluetoothGattCallback gattCallback = new BluetoothGattCallback() {
        @Override
        public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
            Log.v(TAG, "Connection State Changed: " + (newState == BluetoothProfile.STATE_CONNECTED ? "Connected" : "Disconnected"));
            switch (newState) {
                case BluetoothProfile.STATE_CONNECTED:
                    setState(State.CONNECTED);
                    gatt.discoverServices();
                    break;
                default:
                    setState(State.IDLE);
            }
        }

        @Override
        public void onServicesDiscovered(BluetoothGatt gatt, int status) {
            Log.v(TAG, "onServicesDiscovered: " + status);
            if (status == BluetoothGatt.GATT_SUCCESS) {
                subscribe(gatt);
            }
            else{
                Log.e(TAG, "Darn... onServiceDiscovered (gatt) no success... Damn... Crap... Whyyyyyy....");
            }
        }

        @Override
        public void onCharacteristicWrite(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {
            Log.v(TAG, "onCharacteristicWrite: " + status);
        }

        @Override
        public void onDescriptorWrite(BluetoothGatt gatt, BluetoothGattDescriptor descriptor, int status) {
            Log.v(TAG, "onDescriptorWrite: " + status);
        }

        @Override
        public void onCharacteristicRead(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {
            Log.i("onCharacteristicRead", characteristic.toString());
            gatt.disconnect();
        }

        @Override
        public void onCharacteristicChanged(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic) {
//            Log.i(TAG, "onCharacteristicChanged: " + characteristic.getUuid());
            sentFlag = sharedPreferences.getInt("sentFlag", -1);
            switch (sentFlag){
                case 0:
                    //wristband is set to passive mode
                    break;
                case 1:
                    //wristband is set to active mode - receive data constantly
                    parseLiveData(characteristic.getStringValue(0));
                    break;
                case 2:
                    //wristband is set to historic mode
                    parseHistoricData(characteristic.getStringValue(0));
                    break;
                case 3:
                    parseBatteryLevel(characteristic.getStringValue(0));
                    break;
                default:
                    //bugger...
                    Log.e(TAG, "Wrong flag!");
            }
        }
    };

    private void subscribe(BluetoothGatt gatt) {
        gattService = gatt.getService(RFDUINO_SERVICE_UUID);
        receiveCharacteristic = gattService.getCharacteristic(RECEIVE_CHARACTERISTIC_UUID);
        sendCharacteristic = gattService.getCharacteristic(SEND_CHARACTERISTIC_UUID);
        disconnectCharacteristic = gattService.getCharacteristic(DISCONNECT_CHARACTERISTIC_UUID);
        gatt.setCharacteristicNotification(receiveCharacteristic, true);
        BluetoothGattDescriptor receiveConfigDescriptor = receiveCharacteristic.getDescriptor(CLIENT_CHARACTERISTIC_CONFIGURATION_UUID);
        if (receiveConfigDescriptor != null) {
            receiveConfigDescriptor.setValue(BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE);
            gatt.writeDescriptor(receiveConfigDescriptor);
            Log.w(TAG, "Characteristic configured!");
        } else {
            Log.e(TAG, "Receive Characteristic can not be configured.");
        }
    }

    private void writeDataToCharacteristic(final BluetoothGatt gatt, final BluetoothGattCharacteristic ch, final int flagToSend) {
        if (ch == null)
            return;

        Log.w(TAG, "Writing");
        ch.setValue(new byte[]{(byte) flagToSend});

        if(gatt.writeCharacteristic(ch)) {
            Log.w(TAG, "Write success");
        } else {
            Log.w(TAG, "Write failed");
        }
    }

    private void setState(State newState) {
        if (mState != newState) {
            mState = newState;
            Message msg = getStateMessage();
            if (msg != null) {
                sendMessage(msg);
            }
        }
    }

    private Message getStateMessage() {
        Message msg = Message.obtain(null, MSG_STATE_CHANGED);
        if (msg != null) {
            msg.arg1 = mState.ordinal();
        }
        return msg;
    }

    private void sendMessage(Message msg) {
        for (int i = mClients.size() - 1; i >= 0; i--) {
            Messenger messenger = mClients.get(i);
            if (!sendMessage(messenger, msg)) {
                mClients.remove(messenger);
            }
        }
    }

    private boolean sendMessage(Messenger messenger, Message msg) {
        boolean success = true;
        try {
            messenger.send(msg);
        } catch (RemoteException e) {
            Log.w(TAG, "Lost connection to client", e);
            success = false;
        }
        return success;
    }

    private void parseLiveData(String receivedMessage){
        long time = Calendar.getInstance().getTimeInMillis();
        db.addLiveDataEntry(new LiveDataEntry(time, Double.parseDouble(receivedMessage)));
        Log.e(TAG, "Live data: " + Utils.parseDate(time) + " - " + receivedMessage);
    }

    private void parseHistoricData(String receivedMessage){
        receivedValues = receivedMessage.split("/");
        value1 = Integer.parseInt(receivedValues[0]);
        value2 = Integer.parseInt(receivedValues[1]);
        if(value1 == -1){
            //this is where we get the current time (from the device) and calculate the original time (when the device started recoding)
            if(!isStartTimeSet){
                startTime = Calendar.getInstance().getTimeInMillis() - value2;
//                startTime = sharedPreferences.getLong("startTime", 0);
//                if(startTime == 0){
//
//                    sharedPreferences.edit().putLong("startTime", startTime).commit();
//                }
                isStartTimeSet = true;
            }
            lastStartTime = db.getLastEntryStartTime();
        }
        else if(value1 == -2){
            //this is the end of the transmitted data - we'll see what we want to do with it!
            Log.e(TAG, "Transmission ended!");

//            ArrayList<HistoricDataEntry> entries = db.getAllHistoricEntries();
//            Log.e(TAG, "total: " + entries.size());
//            for(HistoricDataEntry entry : entries){
//                Log.e(TAG, "\tStart time: " + entry.getStartTime() +
//                        "\t\tEnd time: " + entry.getEndTime() +
//                        "\t\tReal time: " + Utils.parseDate(entry.getRealTime()) +
//                        "\t\tDuration: " + entry.getDuration());
//            }
        }
        else{
            //this is just reading historic data - start/end time :)
            //and saving to the sqlite database
            Log.e(TAG, "Time: " + Utils.parseDate(startTime + value1) + "\tDuration: " + (value2 - value1));
            //verify if we already have saved data in the database
            if(value1 > lastStartTime){
                db.addHistoricDataEntry(new HistoricDataEntry(value1, value2, (startTime + value1), (value2 - value1)));
            }
        }
    }

    private void parseBatteryLevel(String batteryLevel){
        Log.e(TAG, batteryLevel);
    }
}
