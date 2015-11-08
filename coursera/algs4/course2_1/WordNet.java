
import edu.princeton.cs.algs4.*;
import java.util.*;
public class WordNet {
    private ST<String, Integer> st;
    private String[] keys;
    private Digraph G;
    private SAP sap;
 
    // constructor takes the name of the two input files
    public WordNet(String synsets, String hypernyms)
    {
        int V = 0;
        st = new ST<String, Integer>();
        In in = new In(synsets);
        while (in.hasNextLine()) {
            String[] a = in.readLine().split(",");
            int id =  Integer.parseInt(a[0]);
            String[] b = a[1].split(" ");
            for (int i = 0; i < b.length; i++) {
                if(!st.contains(b[i]))
                    st.put(b[i], id);
            }
            ++V;
        }

        keys = new String[V];
        while (in.hasNextLine()) {
            String[] a = in.readLine().split(",");
            keys[Integer.parseInt(a[0])] = a[1];
        }

        G = new Digraph(V);
        in = new In(hypernyms);
        while (in.hasNextLine()) {
            String[] a = in.readLine().split(",");
            int v = Integer.parseInt(a[0]);
            for (int i = 1; i < a.length; i++) {
                G.addEdge(v, Integer.parseInt(a[i]));
            }
        }

        DirectedCycle finder = new DirectedCycle(G);
        if (finder.hasCycle()) 
            throw new IllegalArgumentException("find cycle");       
 
        sap = new SAP(G);
    }
    
    // returns all WordNet nouns
    public Iterable<String> nouns()
    {
        return st.keys();
    }
  
    private void validateNULL(String v) {
        if (v == null)
            throw new NullPointerException("NULL word");
    }  
 
    // is the word a WordNet noun?
    public boolean isNoun(String word)
    {
        validateNULL(word);
        return st.contains(word);
    }
    
    private void validateWordNet(String v) {
        if (this.isNoun(v) == false)
            throw new IllegalArgumentException("Not WordNet");
    }  
    
    // distance between nounA and nounB (defined below)
    public int distance(String nounA, String nounB)
    {
        validateNULL(nounA);
        validateNULL(nounB);
        validateWordNet(nounA);
        validateWordNet(nounB);
        int v = st.get(nounA);
        int w = st.get(nounB);

        return sap.length(v, w);
    }

    // a synset (second field of synsets.txt) that is the common ancestor of nounA and nounB
    // in a shortest ancestral path (defined below)
    public String sap(String nounA, String nounB)
    {
        validateNULL(nounA);
        validateNULL(nounB);
        validateWordNet(nounA);
        validateWordNet(nounB);
        int v = st.get(nounA);
        int w = st.get(nounB);
        return   keys[sap.ancestor(v, w)];
    }
   
    // do unit testing of this class
    public static void main(String[] args)
    {
    }
}

