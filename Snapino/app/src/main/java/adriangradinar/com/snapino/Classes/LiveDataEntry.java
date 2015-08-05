package adriangradinar.com.snapino.Classes;

/**
 * Created by adriangradinar on 29/07/15.
 */
public class LiveDataEntry {

    private int id;
    private long time;
    private double value;

    public LiveDataEntry(long time, double value) {
        this.time = time;
        this.value = value;
    }

    public LiveDataEntry(int id, long time, double value) {
        this.id = id;
        this.time = time;
        this.value = value;
    }

    public long getTime() {
        return time;
    }

    public void setTime(long time) {
        this.time = time;
    }

    public double getValue() {
        return value;
    }

    public void setValue(double value) {
        this.value = value;
    }
}
