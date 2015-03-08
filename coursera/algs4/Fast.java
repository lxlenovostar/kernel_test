import java.util.*;
public class Fast {
    public static void main(String[] args) {
        int index = 0;
        boolean flag = false;

        String filename = args[0];
        In in = new In(filename);
        int N = in.readInt();
        Point[] points = new Point[N];
        double[] slopd = new double[N];
        //StdDraw.setXscale(0, 32768);
        //StdDraw.setYscale(0, 32768);
        for (int i = 0; i < N; i++) {
            int x = in.readInt();
            int y = in.readInt();
            points[i] = new Point(x, y);
            //points[i].draw();
        }

        //System.out.printf("what\n");
        Arrays.sort(points);

        for (int i = 0; i < N; ++i) {
            int iline = 0;
            Point[] slope_sorted = points.clone();
            Point origin = points[i]; //The point we will be comparing to
            String strpoints = origin.toString(); //this will be used later to construct the output
            Arrays.sort(slope_sorted, origin.SLOPE_ORDER); //sort the temp set by slope to the origin

            for (int j = i; j < N - 1; ++j) {
                if (origin.slopeTo(slope_sorted[j]) == origin.slopeTo(slope_sorted[j+1]))
                {
                    System.out.printf("slop is %f\n", origin.slopeTo(slope_sorted[j]));
                    ++iline;
                    continue;
                }

                //System.out.printf("%d\n", iline);
                if (iline >= 2)
                {
                    if (index == 0)
                    {
                        slopd[index] = origin.slopeTo(slope_sorted[j - iline]);
                        ++index;    
                        System.out.printf("slopd1 is %f\n", slopd[index-1]);
                    }
                    else
                    {
                        for(int m = 0; m < index; ++m)
                        {
                            if (slopd[m] == origin.slopeTo(slope_sorted[j - iline]))
                            {
                                System.out.printf("slopdm is %f\n", slopd[m]);
                                iline = 0;
                                flag = true;
                                break;
                            }
                        }

                        if (flag)
                        {
                            continue;
                        }

                        slopd[index] = origin.slopeTo(slope_sorted[j - iline]);
                        ++index;    
                        System.out.printf("slopd2 is %f\n", slopd[index-1]);
                    }

                    //System.out.printf("whati3\n");
                    for (int k = j - iline; k < j + 1; k++)
                    {
                        if (origin.compareTo(slope_sorted[k]) != 0)
                        {
                            strpoints = strpoints + " -> " + slope_sorted[k].toString();
                        }
                    }
                    System.out.println(strpoints);
                    strpoints = origin.toString();
                    iline = 0;
                    break;
                }
                
            }
        }
    }
}
