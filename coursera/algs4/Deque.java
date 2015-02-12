import java.util.*;

public class Deque<Item> implements Iterable<Item> {
    private Node<Item> first; // link to least recently added node
    private Node<Item> last; // link to most recently added node
    private int N;  // number of items on the queue
    private class Node<Item>
    {   // nested class to define nodes
        private Item item;
        private Node<Item> next;
        private Node<Item> priv;
    }

    public Deque()                           // construct an empty deque
    {
        first = null;
        last  = null;
        N = 0;
    }

    public boolean isEmpty()                 // is the deque empty?
    {
        return N == 0;
    }

    public int size()                        // return the number of items on the deque
    {
        return N;
    }

    private void checknull(Item item)
    {
        if (item == null)
            throw new  NullPointerException("null value to add or remove");
    }

    public void addFirst(Item item)          // insert the item at the front
    {
        checknull(item);

        if (isEmpty())
        {
            Node<Item> only;
            only = new Node<Item>();
            only.item = item;
            only.next = null;
            only.priv = null;
            last = only;
            first = only;
            N++;
        }
        else
        {
            Node<Item> oldfirst = first;
            first = new Node<Item>();
            first.item = item;
            first.next = oldfirst;
            first.priv = null;
            oldfirst.priv = first;
            N++;
        }
    }

    public void addLast(Item item)           // insert the item at the end
    {
        checknull(item);

        if (isEmpty())
        {
            Node<Item> only;
            only = new Node<Item>();
            only.item = item;
            only.next = null;
            only.priv = null;
            first = only;
            last = only;
            N++;
        }
        else
        {
            Node<Item> oldlast = last;
            last = new Node<Item>();
            last.item = item;
            last.next = null;
            last.priv = oldlast;
            oldlast.next = last;
            N++;
        }
    }

    public Item removeFirst()                // delete and return the item at the front
    {
        if (isEmpty())
            throw new NoSuchElementException("Deque underflow");

        Item item = first.item;
        first = first.next;
        N--;

        if (isEmpty())
            first = last = null;   // to avoid loitering

        return item;
    }

    public Item removeLast()                 // delete and return the item at the end
    {
        if (isEmpty())
            throw new NoSuchElementException("Deque underflow");

        Item item = last.item;
        last = last.priv;
        N--;

        if (isEmpty())
            first = last = null;   // to avoid loitering

        return item;

    }

    public Iterator<Item> iterator()         // return an iterator over items in order from front to end
    {
        return new ListIterator<Item>(first);
    }

    private class ListIterator<Item> implements Iterator<Item> {
        private Node<Item> current;

        public ListIterator(Node first) {
            current = first;
        }

        public boolean hasNext()
        {
            return current != null;
        }

        public void remove()
        {
            throw new UnsupportedOperationException();
        }

        public Item next()
        {
            if (!hasNext())
                throw new NoSuchElementException();

            Item item = current.item;
            current = current.next;
            return item;
        }
    }

    public static void main(String[] args)   // unit testing
    {   /*
           Deque<String> q1 = new Deque<String>();
           q1.addFirst("b");
           q1.addLast("c");

           Deque<String> q2 = new Deque<String>();
           q2.addFirst("b");
           q2.addFirst("b");
           q2.removeLast();

           Deque<String> q3 = new Deque<String>();
           q3.addLast("b");
           q3.removeLast();

           Deque<String> q4 = new Deque<String>();
           q3.addLast("b");
           q3.removeFirst();
           */
        /*Deque deque1 = new Deque<Integer>();
        deque1.addFirst(101);
        deque1.addFirst(102);
        System.out.println("what1");
        deque1.removeLast();
        System.out.println("what2");
        //assertEquals(1, deque1.size());
        deque1.removeLast();
        //assertEquals(0, deque1.size());
        */

        Deque deque = new Deque<Integer>();
        deque.addFirst(101);
        deque.addFirst(102);
        deque.addFirst(103);
        deque.addFirst(104);
        deque.addFirst(105);
        deque.addFirst(106);
        Iterator<Integer> i= deque.iterator();

        while (i.hasNext())
        {

            Integer value = i.next();
            System.out.println(value);
        }

    }
}
