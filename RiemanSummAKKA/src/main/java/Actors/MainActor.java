package Actors;

import Mesagges.CalculateMessage;
import Mesagges.ExitMessage;
import Mesagges.FunctionMessage;
import Mesagges.InicioMessage;
import akka.actor.ActorRef;
import akka.actor.Props;
import akka.actor.UntypedAbstractActor;
import akka.event.Logging;
import akka.event.LoggingAdapter;

public class MainActor extends UntypedAbstractActor {
    private final LoggingAdapter log = Logging.getLogger(getContext().system(), this);
    private int numActors = 4;
    private int cantMessageCalculate = 0;
    private Double sumParcial = 0.0d;
    @Override
    public void onReceive(Object message) throws Throwable {
        if(message instanceof FunctionMessage){
            Double start = ((FunctionMessage) message).getStart();
            Double end = ((FunctionMessage) message).getEnd();
            Integer numParticiones = ((FunctionMessage) message).getNumIntervals();
            Double delta = (end - start) / numParticiones;
            for(int i = 0; i < numActors; i++){
                Double li = start + delta *i*numParticiones/(double)numActors;
                Double lf = start + delta * (i+1) * numParticiones/(double)numActors;
                CalculateMessage calculateMessage = new CalculateMessage(li, lf, delta);
                ActorRef worker = getContext().actorOf(Props.create(WorkerActor.class));
                worker.tell(calculateMessage, getSelf());
            }
        } else if (message instanceof ExitMessage){
            sumParcial += ((ExitMessage) message).getTotal();
            cantMessageCalculate++;
            if(cantMessageCalculate == numActors){
                System.out.println("La suma en total es " + sumParcial);
            }
        }
    }
}
