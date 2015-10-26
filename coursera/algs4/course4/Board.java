public class Board {
    // construct a board from an N-by-N array of blocks
    // (where blocks[i][j] = block in row i, column j)
    int[][] i_blocks;
    int N;

    public Board(int[][] blocks) {
        int i, j;

        N = blocks.length;
        for (int i = 0; i < N; i++)
            for (int j = 0; j < N; j++)
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
    
        for (int i = 1; i <= this.dimension(); i++)
            for (int j = 0; j < this.distances(); j++) {
                if (i_blocks[i-1][j] != 0 && i_blocks[i-1][j] != (j*this.dimension()+i))
                    ++num;     
            }
         
        return num;
    }

    public int manhattan()                 // sum of Manhattan distances between blocks and goal
    {

    }

    public boolean isGoal()                // is this board the goal board?
    {
        int i, j;
        boolean result;

        result = FALSE;
        for (int i = 1; i <= this.dimension(); i++)
            for (int j = 0; j < this.distances(); j++) {
                if (i = this.dimension() && j == (this.dimension() - 1) && i_blocks[i-1][j] == 0) {
                    result = TRUE;
                    return result;
                }                
    
                if (i_blocks[i-1][j] != (j*this.dimension()+i)) {
                    return result;
                }
            }    

        return result;
    }

    public Board twin()                    // a board that is obtained by exchanging any pair of blocks
    {
        int i, j;
        boolean flag;
        int num = 0;
        int mis_array1[2];
        int mis_array2[2];
        int count = 0;
        flag = FALSE;
        
        for (int i = 1; i <= this.dimension(); i++)
            for (int j = 0; j < this.distances(); j++) {
                count++;

                if (count == (N*N-1)) {
                    if (num == 2)
                        continue;
                    if (num == 1 || num == 0)
                        return flag;
                }

                if (i_blocks[i-1][j] != (j*this.dimension()+i)) {
                    num++;

                    if (num > 2)
                        return flag;
                    
                    if (num == 1) {
                        mis_array1[0] = j*this.dimension()+i;
                        mis_array1[1] = i_blocks[i-1][j];
                    }
                    else {
                        mis_array2[0] = j*this.dimension()+i;
                        mis_array2[1] = i_blocks[i-1][j];
                    }
                }
        }

        if (mis_array1[0] == mis_array2[1] && mis_array1[1] == mis_array2[0]) {
            flag = True;
            return flag;
        }
        else 
            return flag;
    }
    
    public boolean equals(Object y)        // does this board equal y?
    {
        int i, j;
        int dim = this.dimension();
        boolean flag = FALSE;
        Board y_cmp = (Board)y;

        if (dim != y_cmp.dimension())
            return flag;

        for (i = 0; i < dim; ++i)
            for (j = 0; j < dim; ++j) {
                if (i_blocks[i][j] != y_cmp.i_blocks[i][j])
                    return falg;
            }        

        return TRUE;
    }

    public Iterable<Board> neighbors()     // all neighboring boards
    {
        int i, j;
        int row, col;

        row = 0;
        col = 0;
        
        for (i = 0; i < this.dimension(); ++i)
            for (j = 0; j < this.dimension(); ++j) {
                if (i_blocks[i][j] == 0) {
                    row = i;
                    col = j;
                    break;
                }              
            }

        /*
         * 上下左右怎么操作
         */
         
    }

    public String toString()               // string representation of this board (in the output format specified below)

    public static void main(String[] args) // unit tests (not graded)
}
