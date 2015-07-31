package adriangradinar.com.snapino.Database;

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.util.Log;

import java.math.BigDecimal;
import java.util.ArrayList;

import adriangradinar.com.snapino.Classes.HistoricDataEntry;

public class DatabaseHandler extends SQLiteOpenHelper {

    private static final String TAG = DatabaseHandler.class.getSimpleName();
    //database version
    private static final int DATABASE_VERSION = 1;
    //database name
    private static final String DATABASE_NAME = "snapino_database";

    //declaring the variable names for the table of transaction
    private static final String TBL_HISTORIC_DATA = "tbl_historic_data";
    private static final String HISTORIC_ID = "historic_id";
    private static final String START_TIME = "start_time";
    private static final String END_TIME = "end_time";
    private static final String REAL_TIME = "real_time";
    private static final String DURATION = "duration";

    public DatabaseHandler(Context context){
        super(context, DATABASE_NAME, null, DATABASE_VERSION);
    }

    @Override
    public void onCreate(SQLiteDatabase db) {
        //declare and create the transaction table
        String CREATE_HISTORIC_DATA_TABLE = "CREATE TABLE IF NOT EXISTS " + TBL_HISTORIC_DATA + "("
                + HISTORIC_ID + " INTEGER PRIMARY KEY, " + START_TIME + " TEXT, "
                + END_TIME + " TEXT, " + REAL_TIME + " TEXT, "
                + DURATION + " INTEGER"
                + ")";
        db.execSQL(CREATE_HISTORIC_DATA_TABLE);
    }

    @Override
    public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {}

    public void addHistoricDataEntry(HistoricDataEntry entry){
        SQLiteDatabase db = this.getWritableDatabase();

        //allow the database to create the values to be insert
        ContentValues values = new ContentValues();
        values.put(START_TIME, entry.getStartTime());
        values.put(END_TIME, entry.getEndTime());
        values.put(REAL_TIME, entry.getRealTime());
        values.put(DURATION, entry.getDuration());

        //insert the data into the database
        db.insert(TBL_HISTORIC_DATA, null, values);
        //close the database
//        db.close();
    }

    public ArrayList<HistoricDataEntry> getAllHistoricEntries(){

        ArrayList<HistoricDataEntry> historicDataEntries = new ArrayList<>();

        String sql = "SELECT * FROM " + TBL_HISTORIC_DATA + " ORDER BY " + HISTORIC_ID;
        SQLiteDatabase db = this.getWritableDatabase();

        Cursor cursor = db.rawQuery(sql, null);

        //save every event to the events list array
        if(cursor.moveToFirst()){
            do{
                //create a new temporary transaction
                HistoricDataEntry historicDataEntry = new HistoricDataEntry(cursor.getInt(0), cursor.getLong(1), cursor.getLong(2), cursor.getLong(3), cursor.getInt(4));
                //add to the transaction array list
                historicDataEntries.add(historicDataEntry);
            }
            while (cursor.moveToNext());
        }
        db.close();
        return historicDataEntries;
    }

    public void deleteAllRecords(){
        SQLiteDatabase db = this.getWritableDatabase();
        db.delete(TBL_HISTORIC_DATA,null,null);
        db.close();
    }

    public long getLastEntryStartTime(){
        String sql = "SELECT " + START_TIME + " FROM " + TBL_HISTORIC_DATA + " ORDER BY " + HISTORIC_ID + " DESC LIMIT 1";
        SQLiteDatabase db = this.getWritableDatabase();

        Cursor cursor = db.rawQuery(sql, null);

        //save every event to the events list array
        if(cursor.moveToFirst()){
            return cursor.getInt(0);
        }
        return 0;
    }
}
