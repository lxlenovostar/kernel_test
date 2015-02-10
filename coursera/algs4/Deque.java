public class Deque<Item> implements Iterable<Item> {
    private Node first; // link to least recently added node
    private Node last; // link to most recently added node
    private int N;  // number of items on the queue
    private class Node
    {   // nested class to define nodes
        Item item;
        Node next;
        Node priv;
    }

    public Deque()                           // construct an empty deque
    {
        first = null;
        last  = null;
        N = 0;
    }

    public boolean isEmpty()                 // is the deque empty?
    {
        return first == null;
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

        Node<Item> oldfirst = first;
        first = new Node<Item>();
        first.item = item;
        first.next = oldfirst;
        first.priv = null;
        N++;
    }

    public void addLast(Item item)           // insert the item at the end
    {
        checknull(item);
        Node<Item> oldlast = last;
        last = new Node<Item>();
        last.item = item;
        last.next = null;
        last.priv = oldlast;
        oldlast.next = last;
        N++;
    }

    public Item removeFirst()                // delete and return the item at the front
    {
        if (isEmpty())
            throw new NoSuchElementException("Deque underflow");

        Item item = first.item;
        first = first.next;
        N--;

        if (isEmpty())
            last = null;   // to avoid loitering

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
            last = null;   // to avoid loitering

        return item;

    }

    public Iterator<Item> iterator()         // return an iterator over items in order from front to end
    {
        return new ListIterator<Item>(first);
    }

    private class ListIterator<Item> implements Iterator<Item> {
        private Node<Item> current;

        public ListIterator(Node<Item> first) {
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
}
