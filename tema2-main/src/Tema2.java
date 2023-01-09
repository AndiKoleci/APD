import java.io.*;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.Semaphore;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicInteger;

public class Tema2 {
    public static Semaphore semaphore;
    public static void main(String[] args) throws IOException, InterruptedException {
        String path = args[0];
        int P = Integer.parseInt(args[1]);
        semaphore = new Semaphore(P);
        Thread[] threads = new Thread[P];

        ArrayBlockingQueue<String> queue = new ArrayBlockingQueue<>(1000000);
        FileWriter fw1 = new FileWriter("orders_out.txt");
        PrintWriter out1 = new PrintWriter(fw1);
        FileWriter fw2 = new FileWriter("order_products_out.txt");
        PrintWriter out2 = new PrintWriter(fw2);

        for(int i=0; i<P; i++){
            threads[i] = new Thread(new ReadingThread(queue, path, i, P));
            threads[i].start();
        }
        for(int i=0; i<P; i++){
            threads[i].join();
        }

        for(int i=0; i<P; i++){
            threads[i] = new Thread(new OrderThread(queue, path, out1, out2, P));
            threads[i].start();
        }
        for(int i=0; i<P; i++){
            threads[i].join();
        }
        out1.close();
        out2.close();
    }
}

class ReadingThread implements Runnable{
    ArrayBlockingQueue<String> queue;
    String path;
    int contor;
    int P;


    public ReadingThread(ArrayBlockingQueue<String> queue, String path, int contor, int P) {
        this.queue = queue;
        this.path = path;
        this.contor = contor;
        this.P = P;
    }

    @Override
    public void run() {
        BufferedReader reader;
        int times = -1;
        try {
            reader = new BufferedReader(new FileReader(path + "/orders.txt"));
            String line = reader.readLine();
            while (line != null) {
                times++;
                if(times % P == contor){
                    queue.put(line);
                }
                line = reader.readLine();
            }

            reader.close();
        } catch (IOException e) {
            e.printStackTrace();
        } catch (InterruptedException e) {
            throw new RuntimeException(e);
        }
    }
}

class OrderThread implements Runnable{
    ArrayBlockingQueue<String> queue;
    String path;
    PrintWriter out1;
    PrintWriter out2;
    int P;

    public OrderThread(ArrayBlockingQueue<String> queue, String path, PrintWriter out1, PrintWriter out2, int P) {
        this.queue = queue;
        this.path = path;
        this.out1 = out1;
        this.out2 = out2;
        this.P = P;
    }

    @Override
    public void run() {
        BufferedReader reader;


        while(true) {
            String first;
            int last;
            try {
                //lista comenzi
                String line = queue.poll();
                if (line == null) {
                    return;
                }
                String[] array = line.split(",");
                first = array[0];
                last = Integer.parseInt(array[1]);

                if (last == 0) {
                    continue;
                }
                Thread[] productThreads = new Thread[last];
                ArrayBlockingQueue<String> productsFromCommand = new ArrayBlockingQueue<>(100000);
                AtomicInteger productsShipped = new AtomicInteger();



                try {
                    //lista produse
                    reader = new BufferedReader(new FileReader(path + "/order_products.txt"));
                    String line2 = reader.readLine();

                    while (line2 != null) {
                        String[] array2 = line2.split(",");
                        if(array2[0].equals(first)){
                            productsFromCommand.put(line2);
                        }
                        line2 = reader.readLine();
                    }
                    reader.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }




                for(int i=0; i<last; i++){
                    productThreads[i] = new Thread(new ProductThread(productsFromCommand, path, out1, out2, P, productsShipped));
                    productThreads[i].start();
                }
                for(int i=0; i<last; i++){
                    productThreads[i].join();
                }
                if(productsShipped.get()==last){
                    out1.println(line + ",shipped");
                }

            } catch (InterruptedException e) {
                throw new RuntimeException(e);
            }
        }
    }
}

class ProductThread implements Runnable{
    ArrayBlockingQueue<String> queue;
    String path;
    PrintWriter out1;
    PrintWriter out2;
    int P;
    AtomicInteger productsShipped;

    public ProductThread(ArrayBlockingQueue<String> queue, String path, PrintWriter out1, PrintWriter out2, int P, AtomicInteger productsShipped) {
        this.path = path;
        this.out1 = out1;
        this.out2 = out2;
        this.P = P;
        this.queue = queue;
        this.productsShipped = productsShipped;
    }

    @Override
    public void run() {
        try {
            Tema2.semaphore.acquire();
            //lista produse
            String line = queue.poll(1, TimeUnit.SECONDS);
            if (line == null) {
                return;
            }
            out2.println(line + ",shipped");
            productsShipped.incrementAndGet();

        } catch (InterruptedException e) {
            throw new RuntimeException(e);
        }
        Tema2.semaphore.release();
    }
}

