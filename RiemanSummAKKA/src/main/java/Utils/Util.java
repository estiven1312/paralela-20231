package Utils;

public class Util {
    public static Double calculateValueOfFx(Double x){
        Double y = Math.pow(x, 2) + 2*Math.pow(x,0.5) + Math.exp(2);
        return y;
    }
}
