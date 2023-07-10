import Actors.MainActor;
import Mesagges.FunctionMessage;
import akka.actor.ActorRef;
import akka.actor.ActorSystem;
import akka.actor.Props;

public class Main {
    public static void main(String[] args) {
        ActorSystem actorSystem = ActorSystem.create("RIEMMAN_SUM");
        try {
            ActorRef mainActor = actorSystem.actorOf(Props.create(MainActor.class), "MAIN_ACTOR");
            FunctionMessage functionMessage = new FunctionMessage(8.0d, 24d, 10000);
            mainActor.tell(functionMessage, null);
        } catch ( Exception ex){
            System.err.println(ex);
        } finally {
            actorSystem.terminate();
        }
    }
}
