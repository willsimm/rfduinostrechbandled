package adriangradinar.com.snapino;

import android.app.Fragment;
import android.app.ProgressDialog;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import android.os.Message;
import android.os.Messenger;
import android.os.RemoteException;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;

import adriangradinar.com.snapino.services.BleService;

public class DisplayFragment extends Fragment {

	private static final String TAG = "DisplayFragment";

    private static final int SET_SNAPINO_PASSIVE = 0;
    private static final int SET_SNAPINO_ACTIVE = 1;
    private static final int GET_HISTORIC_DATA = 2;

	public DisplayFragment() {}

	public static DisplayFragment newInstance() {
		return new DisplayFragment();
	}

	private Button getHistoricDataButton = null;
	private Button setModeActiveButton = null;
	private Button setModePassiveButton = null;
	private Intent mServiceIntent;
	private Messenger mService = null;
	private ServiceConnection mConnection = new ServiceConnection() {
		@Override
		public void onServiceConnected(ComponentName name, IBinder service) {
			mService = new Messenger(service);
			try {
				Message msg = Message.obtain(null, BleService.MSG_REGISTER);
				if (msg != null) {
					mService.send(msg);
				} else {
					mService = null;
				}
			} catch (Exception e) {
				Log.e(TAG, "Error connecting to BleService", e);
				mService = null;
			}
		}

		@Override
		public void onServiceDisconnected(ComponentName name) {
			mService = null;
		}
	};

	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
		View v = inflater.inflate(R.layout.fragment_display, container, false);
		mServiceIntent = new Intent(v.getContext(), BleService.class);

		if (v != null) {
            setModePassiveButton = (Button) v.findViewById(R.id.btn_passive);
            setModePassiveButton.setOnClickListener(modePassiveListener);

            setModeActiveButton = (Button) v.findViewById(R.id.btn_active);
            setModeActiveButton.setOnClickListener(modeActiveListener);

            getHistoricDataButton = (Button) v.findViewById(R.id.btn_get_historic_data);
            getHistoricDataButton.setOnClickListener(getHistoricData);
		}
		return v;
	}

    View.OnClickListener modePassiveListener = new View.OnClickListener() {
        @Override
        public void onClick(View view) {
            Message msg = Message.obtain(null, BleService.MSG_WRITE_DATA);
            Bundle bundle = new Bundle();
            bundle.putInt("flag", SET_SNAPINO_PASSIVE);
            msg.setData(bundle);
            try {
                mService.send(msg);
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        }
    };

    View.OnClickListener modeActiveListener = new View.OnClickListener() {
        @Override
        public void onClick(View view) {
            Message msg = Message.obtain(null, BleService.MSG_WRITE_DATA);
            Bundle bundle = new Bundle();
            bundle.putInt("flag", SET_SNAPINO_ACTIVE);
            msg.setData(bundle);
            try {
                mService.send(msg);
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        }
    };

    View.OnClickListener getHistoricData = new View.OnClickListener() {
        @Override
        public void onClick(View view) {
            Message msg = Message.obtain(null, BleService.MSG_WRITE_DATA);
            Bundle bundle = new Bundle();
            bundle.putInt("flag", GET_HISTORIC_DATA);
            msg.setData(bundle);
            try {
                mService.send(msg);
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        }
    };

	@Override
	public void onStart() {
		super.onStart();
		getActivity().bindService(mServiceIntent, mConnection, Context.BIND_AUTO_CREATE);
	}

	@Override
	public void onStop() {
		super.onStop();
		getActivity().unbindService(mConnection);
	}
}
