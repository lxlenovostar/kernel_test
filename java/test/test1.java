import java.io.BufferedInputStream;
import java.io.IOException;

/* 输出为什么是这个样子
 [root@localhost test]# java  -cp "/root/algs4/lib/*:." test1
 begin
 123
 buffer is 49
 begin1
 begin
 buffer is 50
 begin2
 */

public final class test1 {
    private static BufferedInputStream in = new BufferedInputStream(System.in);
    private static final int EOF = -1;    // end of file

    private static int buffer;            // one character buffer
    private static int N;                 // number of bits left in buffer

    // static initializer
    static {
        fillBuffer();
    }

    // don't instantiate
    private test1() { }

    private static void fillBuffer() {
        try {
            StdOut.println("begin");
            buffer = in.read();
            N = 8;
            StdOut.println("buffer is " + buffer);
        }
        catch (IOException e) {
            System.out.println("EOF");
            buffer = EOF;
            N = -1;
        }
    }


    public static void main(String[] args) {
        StdOut.println("begin1");
        test1.fillBuffer(); 
        StdOut.println("begin2");

        /*int count;
        for (count = 0; !BinaryStdIn.isEmpty(); count++) {
            StdOut.println(count + " bits");
        }*/
    }
}
