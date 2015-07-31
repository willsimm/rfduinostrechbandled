package adriangradinar.com.snapino.Classes;

/**
 * Created by adriangradinar on 27/07/15.
 */
public class HistoricDataEntry {

    private int id;
    private long startTime;
    private long endTime;
    private long realTime;
    private int duration;

    public HistoricDataEntry(long startTime, long endTime, long realTime, int duration) {
        this.startTime = startTime;
        this.endTime = endTime;
        this.realTime = realTime;
        this.duration = duration;
    }

    public HistoricDataEntry(int id, long startTime, long endTime, long realTime, int duration) {
        this.id = id;
        this.startTime = startTime;
        this.endTime = endTime;
        this.realTime = realTime;
        this.duration = duration;
    }

    public long getStartTime() {
        return startTime;
    }

    public void setStartTime(long startTime) {
        this.startTime = startTime;
    }

    public long getEndTime() {
        return endTime;
    }

    public void setEndTime(long endTime) {
        this.endTime = endTime;
    }

    public long getRealTime() {
        return realTime;
    }

    public void setRealTime(long realTime) {
        this.realTime = realTime;
    }

    public int getDuration() {
        return duration;
    }

    public void setDuration(int duration) {
        this.duration = duration;
    }
}
