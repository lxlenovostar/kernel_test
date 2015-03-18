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

        /*System.out.printf("origin is %s; slop is %s: %f and %s:%f \n", points[0].toString(), points[1].toString(), points[0].slopeTo(points[1]), points[2].toString(), points[0].slopeTo(points[2]));
        Double obj1 = new Double(points[0].slopeTo(points[1]));
        Double obj2 = new Double(points[0].slopeTo(points[2]));
        if (obj1.compareTo(obj2) == 0)
            System.out.printf("fuck1\n");
        else
            System.out.printf("fuck2\n");
        */

        /*for (int i = 0; i < N; ++i) {
            System.out.printf("point is %s\n", points[i].toString());
        }*/

        for (int i = 0; i < N; ++i) {
            int iline = 0;
            Point[] slope_sorted = points.clone();
            Point origin = points[i]; //The point we will be comparing to
            String strpoints = ""; //this will be used later to construct the output
            Arrays.sort(slope_sorted, origin.SLOPE_ORDER); //sort the temp set by slope to the origin

            /*for (int z = 0; z < N; ++z) {
                System.out.printf("point1 is %s\n", slope_sorted[z].toString());
            }*/

            for (int j = 1; j < N - 1; ++j) {
                //System.out.printf("j is %d and N-1 is %d\n", j, (N - 1));
                if (origin.slopeTo(slope_sorted[j]) == origin.slopeTo(slope_sorted[j+1]))
                {
                    //System.out.printf("origin is %s; slop is %s: %f and %s:%f \n", origin.toString(), slope_sorted[j].toString(), origin.slopeTo(slope_sorted[j]), slope_sorted[j+1].toString(), origin.slopeTo(slope_sorted[j+1]));
                    ++iline;
                    //System.out.printf("%d and N - 1 is %d\n", j, (N-1));
                    if (j + 2 != N)
                        continue;
                    else
                        ++j;
                }

                if (iline >= 2)
                {
                    //System.out.printf("iline is %d and j is %d\n", iline, j);
                    if (index == 0)
                    {
                        slopd[index] = origin.slopeTo(slope_sorted[j - iline]);
                        ++index;
                        //System.out.printf("slopd1 is %f\n", slopd[index-1]);
                    }
                    else
                    {
                        for(int m = 0; m < index; ++m)
                        {
                            if (slopd[m] == origin.slopeTo(slope_sorted[j - iline]))
                            {
                                //System.out.printf("slopdm is %f\n", slopd[m]);
                                iline = 0;
                                flag = true;
                                break;
                            }
                        }

                        if (flag)
                        {
                            flag = false;
                            continue;
                        }

                        slopd[index] = origin.slopeTo(slope_sorted[j - iline]);
                        ++index;
                        //System.out.printf("slopd2 is %f and index is %d\n", slopd[index-1], index-1);
                    }

                    //System.out.printf("whati3\n");
                    int m = 1;
                    Point[] tmpPoint = new Point[iline+2];
                    tmpPoint[0] = origin;
                    //System.out.printf("N is %d and j is %d\n", N, j);
                    for(int k = j - iline; k < j + 1; ++k)
                    {
                        if (origin.compareTo(slope_sorted[k]) != 0)
                        {
                            tmpPoint[m++] = slope_sorted[k];
                            //strpoints = strpoints + " -> " + slope_sorted[k].toString();
                        }
                    }

                    /*for (int z = 0; z < iline+2; ++z) {
                        System.out.printf("point1 is %s\n", tmpPoint[z].toString());
                    }*/

                    Arrays.sort(tmpPoint);

                    strpoints = tmpPoint[0].toString();
                    for (int k = 1; k < m; ++k)
                    {
                        strpoints = strpoints + " -> " + tmpPoint[k].toString();
                    }

                    System.out.println(strpoints);
                    strpoints = origin.toString();
                    iline = 0;
                    break;
                }
                iline = 0;

            }
        }
    }
}
