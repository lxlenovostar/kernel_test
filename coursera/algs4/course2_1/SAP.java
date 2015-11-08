import edu.princeton.cs.algs4.*;
import java.util.*;

public class SAP {
    private final int[][] length;
    private final int[][][] c_length;
    private final int[][] ancestor;
    private final int V;
    private final int E;

    // constructor takes a digraph (not necessarily a DAG)
    public SAP(Digraph G)
    {
        int N = G.V();
        length = new int[N][N];
        c_length = new int[N][N][N];
        ancestor = new int[N][N];
        
        V = G.V();
        E = G.E();

        for (int s = 0; s < G.V(); ++s) {
            BreadthFirstDirectedPaths bfs = new BreadthFirstDirectedPaths(G, s); 
            
            for (int v = 0; v < G.V(); v++) {
                if (bfs.hasPathTo(v)) {
                    /*
                    StdOut.printf("%d to %d (%d):  ", s, v, bfs.distTo(v));
                    for (int x : bfs.pathTo(v)) {
                        if (x == s) StdOut.print(x);
                        else        StdOut.print("->" + x);
                    }
                    StdOut.println();
                    */
                    length[s][v] = bfs.distTo(v);
                }
                else {
                    //StdOut.printf("%d to %d (-):  not connected\n", s, v);
                    length[s][v] = -1;
                }

            }
        }

        for (int v = 0; v < G.V(); ++v) {
           for (int w = 0; w < G.V(); ++w) {
                for (int d = 0; d < G.V(); ++d) {
                    int height_v = length[v][d];
                    int height_w = length[w][d];

                    if (height_v != -1 && height_w != -1) {
                        c_length[v][w][d] = height_v + height_w;
                    }
                    else {
                        c_length[v][w][d] = -1;
                    }
                }     
           } 
        }
         
    }

    // throw an IndexOutOfBoundsException unless 0 <= v < V
    private void validateVertex(int v) {
        if (v < 0 || v >= this.V)
            throw new IndexOutOfBoundsException("vertex " + v + " is not between 0 and " + (this.V-1));
    }   

    // length of shortest ancestral path between v and w; -1 if no such path
    public int length(int v, int w)
    {
        int res = this.E;
        boolean flag = false;
    
        validateVertex(v);
        validateVertex(w);

        for (int a = 0; a < this.V; ++a) {
            if (c_length[v][w][a] != -1 && c_length[v][w][a] <= res ) {
                res = c_length[v][w][a];
                flag = true;
            }
        }        

        if (flag)
            return res;
        else 
            return -1;
    }
    
    // a common ancestor of v and w that participates in a shortest ancestral path; -1 if no such path
    public int ancestor(int v, int w)
    {
        int short_dis;
        
        validateVertex(v);
        validateVertex(w);

        if ((short_dis = this.length(v, w)) == -1)
            return -1;
        else 
        {
            for (int a = 0; a < this.V; ++a) {
                if (short_dis == c_length[v][w][a])
                    return a;
            }
        }
            
        return -1;
    }

    // length of shortest ancestral path between any vertex in v and any vertex in w; -1 if no such path
    public int length(Iterable<Integer> v, Iterable<Integer> w)
    {
        int res = this.E;
        boolean flag = false;
        int temp;

        for (int i : v)
            for (int j : w) {
                validateVertex(i);
                validateVertex(j);
                temp = this.E;
 
                if ((temp = this.length(i, j)) != -1) {
                    flag = true;
                    if (temp <= res)
                        res = temp;
                } 
            }        

        if (flag) 
            return res;
        else  
            return -1;
    }
    
    // a common ancestor that participates in shortest ancestral path; -1 if no such path
    public int ancestor(Iterable<Integer> v, Iterable<Integer> w)
    {
        int short_dis;
            
        if ((short_dis = this.length(v, w)) != -1) {
            for (int i : v)
                for (int j : w) {
                    validateVertex(i);
                    validateVertex(j);

                    if (this.length(i, j) == short_dis) { 
                        StdOut.printf("i = %d, j = %d\n", i, j);
                        return this.ancestor(i, j);
                    }
                }
        }
        else { 
            return -1;
        }
        
        return -1;
    }
   
    // do unit testing of this class
    public static void main(String[] args)
    {
        In in = new In(args[0]);
        Digraph G = new Digraph(in);
        //StdOut.println(G);
        SAP sap = new SAP(G);
       
        /* 
        while (!StdIn.isEmpty()) {
            int v = StdIn.readInt();
            int w = StdIn.readInt();
            int length   = sap.length(v, w);
            int ancestor = sap.ancestor(v, w);
            StdOut.printf("length = %d, ancestor = %d\n", length, ancestor);
        }
        */

        /*
        Queue<Integer> v = new Queue<Integer>(); 
        Queue<Integer> w = new Queue<Integer>();
        v.enqueue(3);
        v.enqueue(9);
        v.enqueue(7);
        v.enqueue(1);
        w.enqueue(11);
        w.enqueue(12);
        w.enqueue(2);
        w.enqueue(6);
        int length   = sap.length(v, w);
        int ancestor = sap.ancestor(v, w);
        StdOut.printf("length = %d, ancestor = %d\n", length, ancestor);
        */
    }
}


