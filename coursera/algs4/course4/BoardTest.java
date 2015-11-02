class BoardTest {
    public static void main(String[] args) {
         // create initial board from file
        In in = new In(args[0]);
        int N = in.readInt();
        int[][] blocks = new int[N][N];
    
        for (int i = 0; i < N; i++)
            for (int j = 0; j < N; j++)
                blocks[i][j] = in.readInt();
        
        Board initial = new Board(blocks);
        StdOut.printf("%s", initial.toString());

        for(Board neg : initial.neighbors())
            StdOut.printf("%s", neg.toString());
        
        StdOut.printf("%s", initial.twin().toString());
    }

}
