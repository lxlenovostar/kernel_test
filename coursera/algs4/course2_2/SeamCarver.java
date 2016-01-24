//import edu.princeton.cs.algs4.*;
import java.util.*;
import java.awt.Color;

public class SeamCarver 
{   
    private final int W;
    private final int H;
    private Picture pic;
    private final double[][] pic_energy;
    private final double min_h;
    private final double min_w;
    private final int index_h;
    private final int index_w;

    private void checknull(Object obj) {
        if (obj == null)
            throw new NullPointerException("NULL object.");
    }

    public SeamCarver(Picture picture) // create a seam carver object based on the given picture   
    {
        checknull(picture);

        int i, j;
        pic = new Picture(picture);
        this.W = pic.width();
        this.H = pic.height();
        this.pic_energy = new double[this.W][this.H];
        this.min_h = Double.POSITIVE_INFINITY;
        this.min_w = Double.POSITIVE_INFINITY;       
        this.index_h = 0;
        this.index_w = 0;

        //calculate the energy.        
        //first handle the  border pixels.
        for (i = 0; i < this.H; i++) {
            this.pic_energy[0][i] = 1000;
            this.pic_energy[this.W - 1][i] = 1000;
        }
        
        for (i = 0; i < this.W; i++) { 
            this.pic_energy[i][0] = 1000;
            this.pic_energy[i][this.H - 1] = 1000;
        }
        
        //second calculate the interior pixels.
        for (j = 1; j <= this.H - 2; j++) 
            for (i = 1; i <= this.W - 2; i++)  // i is:col, j is:row
            {        
                //StdOut.printf("what0 row is:%d, col is:%d:", j, i);
                this.pic_energy[i][j] = this.energy(i, j); 
                StdOut.printf("%d;%d;%f; ", i, j, this.pic_energy[i][j]);
                if (i == this.W - 2)
                    StdOut.printf("\n");
        }  
    }
   
    public Picture picture() // current picture   
    {
        return this.pic; 
    }
    
    public int width() // width of current picture   
    {
        return this.W;
    }
    
    public int height() // height of current picture   
    {
        return this.H;
    }
      
    private void checkrowcol(int row, int col) 
    {
         if (row < 0 || row >= this.H || col < 0 || col >= this.W)
            throw new IndexOutOfBoundsException("error row or col.");
    }
 
    public double energy(int x, int y) // energy of pixel at column x and row y   
    {
        checkrowcol(y, x);
        checkrowcol(y, x-1);
        checkrowcol(y, x+1);
        checkrowcol(y-1, x);
        checkrowcol(y+1, x);

        //calculate x
        //StdOut.printf("what1 row is:%d, col is:%d\n",x-1, y);
        Color x_left = this.picture().get(x-1, y);
        //StdOut.printf("what2 row is:%d, col is:%d\n",x+1, y);
        Color x_right = this.picture().get(x+1, y);
        double energy_x = Math.pow((x_right.getRed() - x_left.getRed()), 2) + Math.pow((x_right.getGreen() - x_left.getGreen()), 2) + Math.pow((x_right.getBlue() - x_left.getBlue()), 2); 

        //calculate y
        //StdOut.printf("what3 row is:%d, col is:%d\n",x, y-1);
        Color y_up = this.picture().get(x, y-1);
        //StdOut.printf("what4 row is:%d, col is:%d\n",x, y+1);
        Color y_down = this.picture().get(x, y+1);
        double energy_y = Math.pow((y_down.getRed() - y_up.getRed()), 2) + Math.pow((y_down.getGreen() - y_up.getGreen()), 2) + Math.pow((y_down.getBlue() - y_up.getBlue()), 2); 
    
        return Math.sqrt(energy_x + energy_y);
    }

    private int select_min(int x, int y) // x is col, y is row. 
    {
        int result = x;
        
        checkrowcol(y, x);

        //StdOut.printf("what1 is:%d; x -1 is:%f; x is:%f;x + 1 is:%f\n", x, this.pic_energy[x-1][y], this.pic_energy[x][y], this.pic_energy[x+1][y]);

        if (x - 1 >= 0 && x + 1 <= this.W -1) {
            if (this.pic_energy[x-1][y] <= this.pic_energy[x+1][y]) {
                if (this.pic_energy[x-1][y] <= this.pic_energy[x][y])
                    result = x - 1;
            } else {
                if (this.pic_energy[x+1][y] <= this.pic_energy[x][y])
                    result = x + 1;
            }
            
            return result;
        } else if (x + 1 <= this.W - 1) {
            if (this.pic_energy[x+1][y] <= this.pic_energy[x][y])
                result = x + 1;
            return result;
        } else {
            if (this.pic_energy[x-1][y] <= this.pic_energy[x][y])
                result = x - 1;
            return result;
        }

        //StdOut.printf("result is:%d\n", result);
        //return result;
    }
    
    /*
     实现原理第一排的节点都执行一遍，相当于 W(节点)*H(边)
     */
    public int[] findVerticalSeam() // sequence of indices for vertical seam   
    { 
        int[] result = new int[this.H];
        int[] tmp = new int[this.H];
        double result_value = Double.POSITIVE_INFINITY;
        double tmp_value = 0;      
        int col = 0;
 
        for (int i = 1; i < this.W - 1; i++) {
        //for (int i = 1; i <= 1; i++) {
            col = i;
            for (int j = 0; j < this.H; j++) {
                if (j == 0) {
                    tmp[j] = col;
                    tmp_value += this.pic_energy[col][j];
                    StdOut.printf("col is:%d; energy is:%f\n", i, this.pic_energy[i][j]);
                }  
                else {
                    int t_col = select_min(col, j);
                    tmp[j] = t_col;
                    tmp_value += this.pic_energy[t_col][j];
                    col = t_col;
                    StdOut.printf("col is:%d; energy is:%f\n", t_col, this.pic_energy[t_col][j]);
                }    
            }
            
            if (tmp_value <= result_value) {
                System.arraycopy(tmp, 0, result, 0, this.H);
            }
                    
            StdOut.printf("\n");
        }

        return result; 
    }

    private int select_min_row(int x, int y) // x is col, y is row. 
    {
        int result = y;
        
        checkrowcol(y, x);

        if (y - 1 >= 0 && y + 1 <= this.H -1) {
            if (this.pic_energy[x][y-1] <= this.pic_energy[x][y+1]) { 
                if (this.pic_energy[x][y-1] <= this.pic_energy[x][y]) 
                    result = y - 1;
            }
            else  { 
                if (this.pic_energy[x][y+1] <= this.pic_energy[x][y])
                    result = y + 1;
            }

            return result;
        } else if (y - 1 >= 0) {
            if (this.pic_energy[x][y-1] <= this.pic_energy[x][y])
                result = y - 1;
            return result;
        } else {
            if (this.pic_energy[x][y+1] <= this.pic_energy[x][y])
                result = y + 1;
            return result;
        }
    }
 
    public int[] findHorizontalSeam() // sequence of indices for horizontal seam   
    {
        int[] result = new int[this.W];
        int[] tmp = new int[this.W];
        double result_value = Double.POSITIVE_INFINITY;
        double tmp_value = 0;      
        int row = 0;
 
        for (int i = 1; i < this.H - 1; i++) {
        //for (int i = 1; i <= 1; i++) {
            row = i;
            for (int j = 0; j < this.W; j++) {
                if (j == 0) {
                    tmp[j] = row;
                    tmp_value += this.pic_energy[j][row];
                    StdOut.printf("row is:%d; energy is:%f\n", row, this.pic_energy[j][row]);
                }  
                else {
                    int t_row = select_min_row(j, row);
                    tmp[j] = t_row;
                    tmp_value += this.pic_energy[j][t_row];
                    row = t_row;
                    StdOut.printf("row is:%d; energy is:%f\n", t_row, this.pic_energy[j][t_row]);
                }    
            }
            
            if (tmp_value <= result_value) {
                System.arraycopy(tmp, 0, result, 0, this.W);
            }
                    
            StdOut.printf("\n");
        }

        return result; 
    }
    
    public void removeHorizontalSeam(int[] seam)   // remove horizontal seam from current picture   
    {
        checknull(seam);
        
        if (seam.length <= 1 || seam.length != this.width()) {
            throw new java.lang.IllegalArgumentException();
        }

        for (int i = 0; i < seam.length; ++i) {
            if (seam[i] < 0 || seam[i] >= this.height()) {
                throw new java.lang.IllegalArgumentException();
            }
        }
        
        for (int i = 1; i < seam.length; ++i) {
            if (Math.abs(seam[i] - seam[i - 1]) > 1) {
                throw new java.lang.IllegalArgumentException();
            }
        }
       
        Picture tmp = new Picture(this.W, this.H-1);

        for (int x = 0; x < this.width(); ++x) {
            for (int y = 0; y < this.height(); ++y) {
                if (y != seam[x])
                    tmp.set(x, y, pic.get(x, y));
            }
        }
        pic = tmp;
        
    }
   
     
    public void removeVerticalSeam(int[] seam)     // remove vertical seam from current picture
    {
        checknull(seam);
        
        if (seam.length <= 1 || seam.length != this.height()) {
            throw new java.lang.IllegalArgumentException();
        }

        for (int i = 0; i < seam.length; ++i) {
            if (seam[i] < 0 || seam[i] >= this.width()) {
                throw new java.lang.IllegalArgumentException();
            }
        }
        
        for (int i = 1; i < seam.length; ++i) {
            if (Math.abs(seam[i] - seam[i - 1]) > 1) {
                throw new java.lang.IllegalArgumentException();
            }
        }
       
        Picture tmp = new Picture(this.W-1, this.H);

        for (int y = 0; y < this.height(); ++y) {
            for (int x = 0; x < this.width(); ++x) {
                if (x != seam[y])
                    tmp.set(x, y, pic.get(x, y));
            }
        }
        pic = tmp;
    }

    public static void main(String[] args)
    {
        Picture p = new Picture("65.png"); 
        //Picture p = new Picture("34.png"); 
        StdOut.println("hello");
        StdOut.printf("w is:%d, h is:%d\n", p.width(), p.height());
        SeamCarver s = new SeamCarver(p);
        //StdOut.println(s.findVerticalSeam());
        //for (int i = 1; i < s.width() - 1; i++)  
       
        /* 
        int[] result = new int[s.height()];
        System.arraycopy(s.findVerticalSeam(), 0, result, 0, s.height());
        for (int j = 0; j <= s.height() - 1; j++) {
                StdOut.printf("%d:\n", result[j]);
        }
        */

        /*
        int[] result = new int[s.width()];
        System.arraycopy(s.findHorizontalSeam(), 0, result, 0, s.width());
        for (int j = 0; j <= s.width() - 1; j++) {
                StdOut.printf("%d:\n", result[j]);
        }
        */
    }
}
