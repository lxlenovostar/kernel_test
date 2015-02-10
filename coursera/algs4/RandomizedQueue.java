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

        //待修改
        int r = StdRandom.uniform(a.length + 1);
        Item item = a[r-1];
        a[r-1] = null;
        --N;

        if (N > 0 && N == a.length/4) resize(a.length/2);
        return item;
    }

    public Item sample()                     // return (but do not delete) a random item
    {
        if (isEmpty())
            throw new NoSuchElementException("dequeue underflow");

        int r = StdRandom.uniform(a.length + 1);
        return a[r-1];
    }

    public Iterator<Item> iterator()         // return an independent iterator over items in random order
    {
        return new ReverseArrayIterator();
    }

    private class ReverseArrayIterator implements Iterator<Item>
    {
        private int i = N;

        public boolean hasNext()
        {
            return i > 0;
        }

        public Item next()
        {
            if (!hasNext())
                throw new NoSuchElementException();

            return a[--i];
        }

        public void remove()
        {
            throw new UnsupportedOperationException();
        }
    }

    public static void main(String[] args)   // unit testing
    {

    }
}
