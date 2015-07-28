package adriangradinar.com.snapino;

import android.app.Activity;
import android.app.Fragment;
import android.content.Context;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AbsListView;
import android.widget.AdapterView;
import android.widget.ListAdapter;
import android.widget.SimpleAdapter;
import android.widget.TextView;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class DeviceListFragment extends Fragment implements AbsListView.OnItemClickListener {
	private static final String KEY_MAC_ADDRESS = "KEY_MAC_ADDRESS";
	private static final String[] KEYS = {"KEY_MAC_ADDRESS"};
	private static final int[] IDS = {android.R.id.text1};

	private OnDeviceListFragmentInteractionListener mListener;

	private AbsListView mListView;
	private TextView mEmptyView;

	private ListAdapter mAdapter;

	private String[] mDevices = null;

	private String mScanning;
	private String mNoDevices;

	public static DeviceListFragment newInstance() {
		return new DeviceListFragment();
	}

	public DeviceListFragment() {
	}

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
	}

	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
		View view = inflater.inflate(R.layout.fragment_device_list, container, false);
		mScanning = inflater.getContext().getString(R.string.scanning);
		mNoDevices = inflater.getContext().getString(R.string.no_devices);

		if (view != null) {
			// Set the adapter
			mListView = (AbsListView) view.findViewById(android.R.id.list);
			mListView.setAdapter(mAdapter);

			mEmptyView = (TextView) view.findViewById(android.R.id.empty);
			mListView.setEmptyView(mEmptyView);

			// Set OnItemClickListener so we can be notified on item clicks
			mListView.setOnItemClickListener(this);
		}

		return view;
	}

	@Override
	public void onAttach(Activity activity) {
		super.onAttach(activity);
		try {
			mListener = (OnDeviceListFragmentInteractionListener) activity;
		} catch (ClassCastException e) {
			throw new ClassCastException(activity.toString() + " must implement OnFragmentInteractionListener");
		}
	}

	@Override
	public void onDetach() {
		super.onDetach();
		mListener = null;
	}


	@Override
	public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
		if (null != mListener) {
			// Notify the active callbacks interface (the activity, if the
			// fragment is attached to one) that an item has been selected.
			mListener.onDeviceListFragmentInteraction(mDevices[position]);
		}
	}

	public void setDevices(Context context, String[] devices) {
		mDevices = devices;
		List<Map<String, String>> items = new ArrayList<>();
		if (devices != null) {

			Log.e("Devices", Arrays.toString(devices));

			for (String device : devices) {
				Map<String, String> item = new HashMap<>();
				item.put(KEY_MAC_ADDRESS, device);
				items.add(item);
			}
		}
		mAdapter = new SimpleAdapter(context, items, android.R.layout.simple_list_item_1, KEYS, IDS);
		mListView.setAdapter(mAdapter);
	}

	public void setEmptyText(CharSequence emptyText) {
		if (mEmptyView != null) {
			mEmptyView.setText(emptyText);
		}
	}

	public interface OnDeviceListFragmentInteractionListener {
		void onDeviceListFragmentInteraction(String macAddress);
	}

	public void setScanning(boolean scanning) {
		mListView.setEnabled(!scanning);
		setEmptyText(scanning ? mScanning : mNoDevices);
	}
}
