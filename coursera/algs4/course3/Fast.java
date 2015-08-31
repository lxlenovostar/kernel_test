import java.util.*;
public class Fast {
    public static void main(String[] args) {
        int index = 0;
        boolean flag = false;

        String filename = args[0];
        In in = new In(filename);
        int N = in.readInt();
        Point[] points = new Point[N];
        double[] slopd = new double[N-1];

        //StdDraw.setXscale(0, 32768);
        //StdDraw.setYscale(0, 32768);

        for (int i = 0; i < N; i++) {
            int x = in.readInt();
            int y = in.readInt();
            points[i] = new Point(x, y);
            //points[i].draw();
        }
       
        /* 
        for (int i = 0; i < N; i++) { 
            StdOut.println(points[i].toString());
        }
        StdOut.println("what1");
        */

        Arrays.sort(points);
       
        /*
        for (int i = 0; i < N; i++) { 
            StdOut.println(points[i].toString());
        }
        StdOut.println("what2");
        */

        /*
        Point origin = points[0];
        Arrays.sort(points, origin.SLOPE_ORDER);
        
        for (int i = 0; i < N; i++) { 
            StdOut.println(points[i].toString());
        }
        */
        
        //HashMap<Double, ArrayList<Point>> result_map = new HashMap<Double, ArrayList<Point>>(); 
        HashMap<Double, ArrayList<Point>> map = new HashMap<Double, ArrayList<Point>>(); 
        for (int i = 0; i < N; i++) {
            Point origin = points[i];
            Point[] unsort_points = points.clone();
            Arrays.sort(unsort_points, origin.SLOPE_ORDER);
           
            /* 
            for (Point q: unsort_points) 
                StdOut.println(q.toString());
            StdOut.println("");
            */

            StdOut.printf("%d %s\n", i, origin.toString());
            for (Point p: unsort_points) {
                //StdOut.println(p.toString());
                double slope = origin.slopeTo(p);
                //StdOut.println(map.get(slope));
                //StdOut.println("what3");
                
                if (map.containsKey(slope) == false) {
                    ArrayList<Point> point_list = new ArrayList<Point>();
                    point_list.add(p);
                    map.put(slope, point_list);    
                } 
                else {
                    ArrayList<Point> point_list = map.get(slope);
                    //StdOut.println("old" + map.get(slope).toString());
                    point_list.add(p);
                    map.remove(slope);
                    //if (map.containsKey(slope) == false) 
                    //        StdOut.println("delete it" + point_list.toString());
                        
                    map.put(slope, point_list);    
                }
                
                //for (Double d: map.keySet()) {
                //    StdOut.printf("%f and %s %s\n", d, origin.toString(), (map.get(d)).toString());
                //}
                
                
                for (Double d : map.keySet()) {
                    if ((map.get(d).size()) >= 3) {
                        StdOut.printf("%f and %s\n", d, origin.toString());
                        for (Point result : map.get(d)) {
                            StdOut.print(result + " -> ");
                        }    
                        StdOut.println("");
                    }
                }
            }
            map.clear();
        }
    }
}
