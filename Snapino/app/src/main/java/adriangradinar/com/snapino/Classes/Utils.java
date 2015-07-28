package adriangradinar.com.snapino.Classes;

import java.text.SimpleDateFormat;

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
}
