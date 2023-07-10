package Mesagges;

public class CalculateMessage {
    private final Double start;
    private final Double end;

    private final Double delta;

    public Double getStart() {
        return start;
    }

    public Double getEnd() {
        return end;
    }

    public Double getDelta() {
        return delta;
    }

    public CalculateMessage(Double start, Double end, Double delta){
        this.start = start;
        this.end = end;
        this.delta = delta;
    }
}
