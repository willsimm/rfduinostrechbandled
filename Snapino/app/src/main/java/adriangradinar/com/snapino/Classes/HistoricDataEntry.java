package adriangradinar.com.snapino.Classes;

/**
 * Created by adriangradinar on 27/07/15.
 */
public class HistoricDataEntry {

    private int id;
    private long startTime;
    private int duration;

    public HistoricDataEntry(long startTime, int duration) {
        this.startTime = startTime;
        this.duration = duration;
    }

    public HistoricDataEntry(int id, long startTime, int duration) {
        this.id = id;
        this.startTime = startTime;
        this.duration = duration;
    }

    public long getStartTime() {
        return startTime;
    }

    public void setStartTime(long startTime) {
        this.startTime = startTime;
    }

    public int getDuration() {
        return duration;
    }

    public void setDuration(int duration) {
        this.duration = duration;
    }
}
