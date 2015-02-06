class PercolationTest {

    public static void main(String[] args) {
        Percolation p1 = new Percolation(1);
        if (p1.percolates())
            System.out.println("Failed corner case N=1");
        else
            System.out.println("Passed N=1");
        Percolation p2 = new Percolation(2);
        if (p2.percolates())
            System.out.println("Failed corner case N=2");
        else
            System.out.println("Passed N=2");

        Percolation p3 = new Percolation(2);
        p3.open(1, 1);
        p3.open(2, 2);
        p3.open(1, 2);
        if (!p3.percolates())
            System.out.println("Failed corner case N=2");
        else
            System.out.println("Passed N=2");

        Percolation backwash = new Percolation(4);
        backwash.open(4, 1);
        backwash.open(1, 3);
        backwash.open(2, 3);
        backwash.open(3, 3);
        backwash.open(4, 3);


        if (backwash.isFull(4, 1))
            System.out.println("Failed backwash test!");
        else
            System.out.println("Backwash test passed");

        Percolation reverse = new Percolation(4);
        reverse.open(4, 3);
        reverse.open(3, 3);
        reverse.open(2, 3);
        reverse.open(1, 3);

        if (!reverse.percolates()) {
            System.out.println("Failed reverse percolation opening sequence.");
        } else {
            System.out.println("Reverse passed!");
        }

    }

}
