//import edu.princeton.cs.algs4.*;
import java.util.*;

public class Outcast {
    private WordNet this_wordnet;

    public Outcast(WordNet wordnet)         // constructor takes a WordNet object
    {
        this_wordnet = wordnet;
    }

    public String outcast(String[] nouns)   // given an array of WordNet nouns, return an outcast
    {
        for (int i = 0; i < nouns.length; i++) {
            if
        }
        
        return "";        
    }    
    
    public static void main(String[] args)  // see test client below}
    {
        WordNet wordnet = new WordNet(args[0], args[1]);
        Outcast outcast = new Outcast(wordnet);
    
        for (int t = 2; t < args.length; t++) {
            In in = new In(args[t]);
            String[] nouns = in.readAllStrings();
            StdOut.println(args[t] + ": " + outcast.outcast(nouns));
        }
    }
}
