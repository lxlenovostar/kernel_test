//import edu.princeton.cs.algs4.*;
import java.util.*;
import java.awt.Color;

public class SeamCarver 
{   
    private final int W;
    private final int H;
    private final Picture pic;
    private final double[][] pic_energy;
    private final double min_h;
    private final double min_w;
    private final int index_h;
    private final int index_w;

    public SeamCarver(Picture picture) // create a seam carver object based on the given picture   
    {
        int i, j;
        pic = new Picture(picture);
        this.W = pic.width();
        this.H = pic.height();
        this.pic_energy = new double[this.H][this.W];
        this.min_h = Double.POSTIVE_INFINITY;
        this.min_w = Double.POSTIVE_INFINITY;       
        this.index_h = 0;
        this.index_w = 0;

        //calculate the energy.        
        //first handle the  border pixels.
        for (i = 0; i < this.W; i++) {
            this.pic_energy[0][i] = 1000;
            this.pic_energy[this.H - 1][i] = 1000;
        }
        
        for (i = 0; i < this.H; i++) { 
            this.pic_energy[i][0] = 1000;
            this.pic_energy[i][this.W - 1] = 1000;
        }
        
        //second calculate the interior pixels.
        for (i = 1; i < this.H - 1; i++)  
            for (j = 1; j < this.W - 1; j++) {
                this.pic_energy[i][j] = this.energy(i, j); 
        
                // find the min in row 1
                if (i == 1) {
                    if (this.pic_energy[i][j] < min_w) {
                        min_w = this.pic_energy[i][j];
                        
                    }
                }

                // find the min in col 1
                if (j == 1) {

                }
        }  
    }
   
    public Picture picture() // current picture   
    {
        return pic; 
    }
    
    public int width() // width of current picture   
    {
        return this.W;
    }
    
    public int height() // height of current picture   
    {
        return this.H;
    }
  
    public double energy(int x, int y) // energy of pixel at column x and row y   
    {
        //calculate x
        Color x_left = this.picture().get(x-1, y);
        Color x_right = this.picture().get(x+1, y);
        double energy_x = Math.pow((x_right.getRed() - x_left.getRed()), 2) + Math.pow((x_right.getGreen() - x_left.getGreen()), 2) + Math.pow((x_right.getBlue() - x_left.getBlue()), 2); 

        //calculate y
        Color y_up = this.picture().get(x, y-1);
        Color y_down = this.picture().get(x, y+1);
        double energy_y = Math.pow((y_down.getRed() - y_up.getRed()), 2) + Math.pow((y_down.getGreen() - y_up.getGreen()), 2) + Math.pow((y_down.getBlue() - y_up.getBlue()), 2); 
    
        return Math.sqrt(energy_x + energy_y);
    }

    /*
    public int[] findHorizontalSeam() // sequence of indices for horizontal seam   
    {

    }
    
    public int[] findVerticalSeam() // sequence of indices for vertical seam   
    {
       
    }
    
    public void removeHorizontalSeam(int[] seam)   // remove horizontal seam from current picture   
    {

    }
    
    public void removeVerticalSeam(int[] seam)     // remove vertical seam from current picture
    {

    }
    */

    public static void main(String[] args)
    {
        Picture p = new Picture("65.png"); 
        StdOut.println("hello");
        StdOut.printf("w is:%d, h is:%d\n", p.width(), p.height());
    }
}
