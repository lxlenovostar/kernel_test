import java.util.*;

public class RandomizedQueue<Item> implements Iterable<Item> {
    private Item[] a;
    private int N;

    public RandomizedQueue()                 // construct an empty randomized queue
    {
        a = (Item[]) new Object[1];
        N = 0;
    }

    public boolean isEmpty()                 // is the queue empty?
    {
        return N == 0;
    }

    public int size()                        // return the number of items on the queue
    {
        return N;
    }

    private void resize(int max)
    {
        Item[] temp = (Item[]) new Object[max];
        for (int i = 0; i < N; ++i)
            temp[i] = a[i];
        a = temp;
    }

    private void checknull(Item item)
    {
        if (item == null)
            throw new  NullPointerException("null value to add or remove");
    }

    public void enqueue(Item item)           // add the item
    {
        checknull(item);

        if (N == a.length) resize(2*a.length);
        a[N++] = item;
    }

    public Item dequeue()                    // delete and return a random item
    {
        if (isEmpty())
            throw new NoSuchElementException("dequeue underflow");
        //
        //first call swap, and second dequeue.
        //待修改,,需要将最后一个值拷贝过来。然长度减少1
        int r = StdRandom.uniform(N);

        //StdOut.printf("r is %d\n", r);

        Item swap = a[r];
        a[r] = a[N-1];
        a[N-1] = null;
        --N;

        if (N > 0 && N == a.length/4)
            resize(a.length/2);
        return swap;
    }

    public Item sample()                     // return (but do not delete) a random item
    {
        if (isEmpty())
            throw new NoSuchElementException("dequeue underflow");

        int r = StdRandom.uniform(N);
        return a[r];
    }

    public Iterator<Item> iterator()         // return an independent iterator over items in random order
    {
        return new ReverseArrayIterator();
    }

    private class ReverseArrayIterator implements Iterator<Item>
    {
        private int counter = 0;
        private Item[] output;

        private ReverseArrayIterator() {
            output = (Item[]) new Object[N];
            for (int i = 0; i < N; i++)
                output[i] = a[i];
            
            /*
             *  这里为什么要这样实现，而不是我每次随机取一个数
             * */
            StdRandom.shuffle(output);
        }


        public boolean hasNext()
        {
            return counter < N;
        }

        public Item next()
        {
            if (!hasNext())
                throw new NoSuchElementException();

            return output[counter++];
        }

        public void remove()
        {
            throw new UnsupportedOperationException();
        }
    }

    public static void main(String[] args)   // unit testing
    {
        RandomizedQueue r = new RandomizedQueue();
        r.enqueue(1);
        r.enqueue(2);
        r.enqueue(3);
        //r.dequeue();
        //r.dequeue();
        Iterator<Integer> iterator = r.iterator();
        StdOut.printf("value is %d\n", iterator.next());
        StdOut.printf("value is %d\n", iterator.next());
        StdOut.printf("value is %d\n", iterator.next());
        Iterator<Integer> iterator1 = r.iterator();
        StdOut.printf("value1 is %d\n", iterator1.next());
        StdOut.printf("value1 is %d\n", iterator1.next());
        StdOut.printf("value1 is %d\n", iterator1.next());
    }
}
