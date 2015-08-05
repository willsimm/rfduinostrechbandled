package adriangradinar.com.snapino.Classes;

import android.content.Context;
import android.os.Environment;
import android.util.Log;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;

import adriangradinar.com.snapino.Database.DatabaseHandler;

/**
 * Created by adriangradinar on 28/07/15.
 */
public class Utils {

    private static final String TAG = Utils.class.getSimpleName();
    private static final SimpleDateFormat sdf = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");

    public static final String parseDate(Long millis){
        return sdf.format(millis);
    }

    private String convertByteArrayToString(byte[] array){
        long value = 0;
        for (int i = 0; i < array.length; i++) {
            value += ((long) array[i] & 0xffL) << (8 * i);
        }
        return String.valueOf(value);
    }

    public static void writeHistoricDataToFile(Context c, String filename){
        DatabaseHandler db = new DatabaseHandler(c);
        ArrayList<HistoricDataEntry> allEntries = db.getAllHistoricEntries();

        for(HistoricDataEntry entry : allEntries){
            String message = entry.getRealTime() + "," + entry.getDuration() + "\n";
            writeToFile(Environment.getExternalStorageDirectory().getAbsolutePath() + "/Snap", filename, message);
        }
    }

    private static void writeToFile(String fullPath, String fileName, String msg) {
        try {
            File dir = new File(fullPath);
            if (!dir.exists()) {
                dir.mkdirs();
            }

            FileOutputStream fos;
            File myFile = new File(fullPath, fileName);
            if (!myFile.exists())
                myFile.createNewFile();
            byte[] data = msg.getBytes();
            try {
                fos = new FileOutputStream(myFile, true);
                fos.write(data);
                fos.flush();
                fos.close();
            } catch (FileNotFoundException e) {
                e.printStackTrace();
            }
        } catch (FileNotFoundException e) {
            Log.e(TAG, "Couldn't find the file");
            e.printStackTrace();
        } catch (IOException e) {
            Log.e(TAG, "An I/O Error occurred");
            e.printStackTrace();
        }
    }
}
