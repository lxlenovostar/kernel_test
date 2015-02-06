/*************************************************************************
 *  author:    lx
 **************************************************************************/

public class Percolation {
    private boolean[] site;
    private WeightedQuickUnionUF uf;
    private int count = 0;
    private int top = 0;
    private int bottom = 1;

    // create N-by-N grid, with all sites blocked
    public Percolation(int N)
    {
        int i, j;

        if (N <= 0)
            throw new java.lang.IllegalArgumentException("N is not a fit value");

        count = N;
        site = new boolean[N*N + 2];
        uf = new WeightedQuickUnionUF(N*N + 2);

        // handle the first and last row.
        for (i = 1; i <= N; ++i) {
            uf.union(top, xyTo1D(1, i) + 2);
            uf.union(bottom, xyTo1D(count, i) + 2);
        }
    }

    private int xyTo1D(int i, int j)
    {
        validate(i, j);
        return (i - 1) * count + j - 1;
    }

    private void validate(int i, int j)
    {

        if (i < 1 || i > count || j < 1 || j > count)
            throw new IndexOutOfBoundsException("row index i out of bounds");
    }

    private void unionTwoSites(int i, int j, int m, int n)
    {
        uf.union(xyTo1D(i, j) + 2, xyTo1D(m, n) + 2);
    }

    // open site (row i, column j) if it is not open already
    public void open(int i, int j)
    {
        validate(i, j);

        if (isOpen(i, j))
            return;

        site[xyTo1D(i, j)] = true;

        if (i > 1 && isOpen(i-1, j)) // top
            unionTwoSites(i, j, i-1, j);
        if (i < count && isOpen(i+1, j)) // bottom
            unionTwoSites(i, j, i+1, j);
        if (j > 1 && isOpen(i, j-1)) // left
            unionTwoSites(i, j, i, j-1);
        if (j < count && isOpen(i, j+1)) // right
            unionTwoSites(i, j, i, j+1);
    }

    // is site (row i, column j) open?
    public boolean isOpen(int i, int j)
    {
        validate(i, j);

        return site[xyTo1D(i, j)];
    }

    // is site (row i, column j) full?
    public boolean isFull(int i, int j)
    {
        if (isOpen(i, j) && uf.connected(top, xyTo1D(i, j)+2))
        {
            if (i == count)
            {
                if (i > 1 && isOpen(i-1, j) && uf.connected(top, xyTo1D(i-1, j)+2))
                    return true;
                if (j > 1 && isOpen(i, j-1) && uf.connected(top, xyTo1D(i, j-1)+2))
                    return true;
                if (j < count && isOpen(i, j+1) && uf.connected(top, xyTo1D(i, j+1)+2))
                    return true;
                return false;
            }
            else
            {
                return true;
            }
        }
        else
        {
            return false;
        }
    }

    // does the system percolate?
    public boolean percolates()
    {
        if (count == 1)
            return false;

        return uf.connected(top, bottom);
    }

    // test client (optional)}
    public static void main(String[] args)
    {
        Percolation t = new Percolation(1);
        t.open(1, 2);
    }
}
