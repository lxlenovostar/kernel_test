public class Solver {
    private finally boolean solved;
    private finally int move_count; 

    public Solver(Board initial)           // find a solution to the initial board (using the A* algorithm)
    {
        this.solved = false;
        this.move_count = 0;

        Board 
    }    

    public boolean isSolvable()            // is the initial board solvable?
    {
        return this.solved;
    }

    public int moves()                     // min number of moves to solve initial board; -1 if unsolvable
    {
        return this.move_count;
    }

    public Iterable<Board> solution()      // sequence of boards in a shortest solution; null if unsolvable
    {

    }

    //public static void main(String[] args) // solve a slider puzzle (given below)
}
