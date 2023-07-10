package Actors;

import Mesagges.CalculateMessage;
import Mesagges.ExitMessage;
import Utils.Util;
import akka.actor.UntypedAbstractActor;

public class WorkerActor extends UntypedAbstractActor {

    @Override
    public void onReceive(Object message) throws Throwable {
        if(message instanceof CalculateMessage){
            Double inicio =((CalculateMessage) message).getStart();
            Double fin = ((CalculateMessage) message).getEnd();
            Double delta = ((CalculateMessage) message).getDelta();
            Double suma = 0.d;
            for(double i = inicio; i<fin- 0.000001; i = i + delta){
                suma += Util.calculateValueOfFx(i) * delta;
            }
            ExitMessage exitMessage = new ExitMessage(suma);
            getSender().tell(exitMessage, getSelf());

        }
    }
}
