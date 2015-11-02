//https://github.com/merwan/algs4/blob/master/8-puzzle/src/Board.java

//import edu.princeton.cs.algs4.*;
import java.util.*;
public class Board {
    // construct a board from an N-by-N array of blocks
    // (where blocks[i][j] = block in row i, column j)
    private final int[][] i_blocks;
    private final int N;

    public Board(int[][] blocks) {
        int i, j;

        N = blocks.length;
        i_blocks = new int[N][N];
        for (i = 0; i < N; i++)
            for (j = 0; j < N; j++)
                i_blocks[i][j] = blocks[i][j];                    
    }          
 
    public int dimension()                 // board dimension N
    {
        return N;
    }

    public int hamming()                   // number of blocks out of place
    {
        int i, j;
        int num = 0;
    
        for (i = 1; i <= this.dimension(); i++)
            for (j = 1; j <= this.dimension(); j++) {
                if (i_blocks[i-1][j-1] != 0 && i_blocks[i-1][j-1] != ((j-1)*this.dimension()+i))
                    ++num;     
            }
         
        return num;
    }

    public int manhattan()                 // sum of Manhattan distances between blocks and goal
    {
        int i, j, m_row, m_col, right_row, right_col, orig_row, orig_col;
        int mh = 0;
        
        for (i = 1; i <= this.dimension(); i++)
            for (j = 1; j < this.dimension(); j++) {
                if (i_blocks[i-1][j-1] != ((j-1)*this.dimension()+i)) {
                    right_row = i_blocks[i-1][j-1] / this.dimension();
                    right_col = i_blocks[i-1][j-1] % this.dimension();
                    orig_row = i;
                    orig_col = j;
                    m_row = (right_row > orig_row) ? (right_row - orig_row) : (orig_row - right_row);        
                    m_col = (right_col > orig_col) ? (right_col - orig_col) : (orig_col - right_col);
                    mh += (m_row + m_col);
                }
            }

        return mh;
    }

    public boolean isGoal()                // is this board the goal board?
    {
        int i, j;
        boolean result;

        result = false;
        for (i = 1; i <= this.dimension(); i++)
            for (j = 1; j <= this.dimension(); j++) {
                if (i == this.dimension() && j == this.dimension() && i_blocks[i][j] == 0) {
                    result = true;
                    return result;
                }                
    
                if (i_blocks[i-1][j-1] != ((j-1)*this.dimension()+i)) {
                    return result;
                }
            }    

        return result;
    }

    public Board twin()                    // a board that is obtained by exchanging any pair of blocks
    {
        int i, j;
        int[][] twin = new int[N][N];
        int row, col;

        Random randrow = new Random();
        row = randrow.nextInt(N);
        Random randcol = new Random();
        col = randcol.nextInt(N-1);   

        //StdOut.printf("row is:%d; col is:%d\n", row, col);
        
        for (i = 0; i < this.dimension(); ++i)
            for (j = 0; j < this.dimension(); ++j) {
                twin[i][j] = i_blocks[i][j];
            } 

        
        Board board = new Board(twin);        
        board.swap(row, col, row, col+1);

        return board;
    }
    
    public boolean equals(Object y)        // does this board equal y?
    {
        int i, j;
        int dim = this.dimension();
        boolean flag = false;
        Board y_cmp = (Board)y;

        if (dim != y_cmp.dimension())
            return flag;

        for (i = 0; i < dim; ++i)
            for (j = 0; j < dim; ++j) {
                if (i_blocks[i][j] != y_cmp.i_blocks[i][j])
                    return flag;
            }        

        return true;
    }

    private boolean swap(int i, int j, int it, int jt) {
        if (it < 0 || it >= N || jt < 0 || jt >= N) {
            return false;
        }
        int temp = i_blocks[i][j];
        i_blocks[i][j] = i_blocks[it][jt];
        i_blocks[it][jt] = temp;
        return true;
    } 

    public Iterable<Board> neighbors() {
        int i0 = 0, j0 = 0;
        boolean found = false;
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                if (i_blocks[i][j] == 0) {
                    i0 = i;
                    j0 = j;
                    found = true;
                    break;
                }
            }
            if (found) {
                break;
            }
        }

        Stack<Board> boards = new Stack<Board>();
        
        Board board = new Board(i_blocks);        
        boolean isNeighbor = board.swap(i0, j0, i0 - 1, j0);
        if (isNeighbor) {
            boards.push(board);
        }

        board = new Board(i_blocks);
        isNeighbor = board.swap(i0, j0, i0, j0 - 1);
        if (isNeighbor) {
            boards.push(board);
        }

        board = new Board(i_blocks);
        isNeighbor = board.swap(i0, j0, i0 + 1, j0);
        if (isNeighbor) {
            boards.push(board);
        }

        board = new Board(i_blocks);
        isNeighbor = board.swap(i0, j0, i0, j0 + 1);
        if (isNeighbor) {
            boards.push(board);
        }

        return boards; 
    }


    // string representation of this board (in the output format specified below)
    public String toString()               
    {
        StringBuilder s = new StringBuilder();
        s.append(N + "\n");
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                s.append(String.format("%2d ", i_blocks[i][j]));
            }
            s.append("\n");
        }
        return s.toString();
    }
   
    /* 
    public static void main(String[] args)
    {        
        // create initial board from file
        In in = new In(args[0]);
        int N = in.readInt();
        int[][] blocks = new int[N][N];
    
        for (int i = 0; i < N; i++)
            for (int j = 0; j < N; j++)
                blocks[i][j] = in.readInt();
        Board initial = new Board(blocks);
        StdOut.printf("%s", initial.toString());
    }*/ 
}

