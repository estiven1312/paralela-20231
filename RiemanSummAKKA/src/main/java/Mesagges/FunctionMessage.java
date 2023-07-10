package Mesagges;

public class FunctionMessage {
    private final Double start;
    private final Double end;
    private final Integer numIntervals;

    public Double getStart() {
        return start;
    }

    public Double getEnd() {
        return end;
    }

    public Integer getNumIntervals() {
        return numIntervals;
    }

    public FunctionMessage(Double start, Double end, Integer numIntervals) {
        this.start = start;
        this.end = end;
        this.numIntervals = numIntervals;
    }
}
