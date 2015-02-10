public class Subset {
    public static void main(String[] args)
    {
        int N;
        RandomizedQueue r ;

        N =  Integer.parseInt(args[0]);
        StdOut.print(N + "\n");
        r = new RandomizedQueue();

        while (!StdIn.isEmpty())
        {
            String item = StdIn.readString();
            r.enqueue(item);
        }

        while(N-- > 0)
        {
            StdOut.print(r.dequeue() + "\n");
        }

    }
}
