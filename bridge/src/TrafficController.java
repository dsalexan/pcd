public class TrafficController {

    private boolean occupied = false;

    private synchronized void enter() {
        // entrando na ponte
        // verifica se tem algum carro ocupando
        while (occupied) { // se tiver, WAIT
            try {
                wait();
            } catch(InterruptedException e) {
                Thread.currentThread().interrupt();
            }
        }
        occupied = true; // re ocupa a ponte

        notifyAll();
    }

    private synchronized void leave() {
        // saindo da ponte

        occupied = false;
        notifyAll();
    }


    public void enterLeft() {
        enter();
    }

    public void enterRight() {
        enter();
    }

    public void leaveLeft() {
        leave();
    }

    public void leaveRight() {
        leave();
    }

}