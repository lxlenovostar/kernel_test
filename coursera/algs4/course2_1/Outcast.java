import edu.princeton.cs.algs4.*;
import java.util.*;

public class Outcast {
    private WordNet this_wordnet;

    public Outcast(WordNet wordnet)         // constructor takes a WordNet object
    {
        this_wordnet = wordnet;
    }

    public String outcast(String[] nouns)   // given an array of WordNet nouns, return an outcast
    {
        int out_max = -1;
        int index = 0;
        boolean flag = false;

        for (int i = 0; i < nouns.length; i++) {
            int out_sum = 0; 
            for (int j = 0; i < nouns.length; i++) {
                if (i == j)
                    continue;
                if (!this.this_wordnet.isNoun(nouns[i]) || !this.this_wordnet.isNoun(nouns[j]))
                    continue;
                out_sum += this.this_wordnet.distance(nouns[i], nouns[j]);
        }
            if (out_sum > out_max) {
                out_max = out_sum;
                index = i;
                flag = true;
            }
        }

        if (flag)
            return nouns[index];
        else
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
