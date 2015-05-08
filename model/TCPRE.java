import java.math.BigInteger;
import java.util.Random;
import java.util.Hashtable;
import java.math.BigInteger;

public class TCPRE {
    private long Q = 1;          // a large prime, small enough to avoid long overflow
    private int R;           // radix
    private long RM;         // R^(M-1) % Q
    private int zero_num = 5;    // the number of bits which equals 0
    private int zero_value = 1;
    private int chunk_num = 64;   // divide the string into chunk
    private int remedy = 12;
    private Hashtable<Long, String> hashmap; // hash the cache data
    public int count = 0;
    public int count_comp = 0;

    public TCPRE(String source) {
        R = 1048583;

        for (int i = 0; i < 60; ++i)
            Q = (2 * Q);

        //Q = longRandomPrime();
        //R = 256;

        hashmap = new Hashtable<Long, String>();

        // precompute R^(M-1) % Q for use in removing leading digit
        RM = 1;
        for (int i = 1; i <= chunk_num - 1; ++i)
            RM = (R * RM) % Q;

        for (int i = 0; i < zero_num; ++i)
            zero_value = (2 * zero_value);
        zero_value = zero_value - 1;

        if (this.calculateFingerprint(source) == false)
            throw new RuntimeException("the length of  source data less than " + chunk_num);
    }
    
    private String longToString(long data) {
        Long R = new Long(data);
        String str = Long.toString(R);
        return str;
    }

    // Compute hash for key[0..M-1].
    private long hash(String key, int M) {
        long h = 0;
        for (int j = 0; j < M; j++)
            h = (R * h + key.charAt(j)) % Q;
        return h;
    }

    private boolean calculateFingerprint(String txt) {
        int N = txt.length();
        if (N < chunk_num) return false;

        long txtHash = hash(txt, chunk_num);
        if ((txtHash & zero_value) == 0) {
            if (hashmap.get(txtHash>>zero_num) == null) {
                ++count;
                hashmap.put(txtHash>>zero_num, "source");
                //hashmap.put(txtHash>>zero_num, txt.substring(0, chunk_num));
            }
        }

        // check for hash match; if hash match, check for exact match
        for (int i = chunk_num; i < N; i++) {
            // Remove leading digit, add trailing digit, check for match.
            txtHash = (txtHash + Q - RM*txt.charAt(i-chunk_num) % Q) % Q;
            txtHash = (txtHash*R + txt.charAt(i)) % Q;

            if ((txtHash & zero_value) == 0) {
                //判断是否自身有相同的指纹
                if (hashmap.get(txtHash>>zero_num) != null)
                    continue;
                ++count;
                hashmap.put(txtHash>>zero_num, "source");
                //hashmap.put(txtHash>>zero_num, txt.substring(i - chunk_num + 1, i));
                //StdOut.println("old is:" + (txtHash>>zero_num));
                //StdOut.println("str is: " + txt.substring(i - chunk_num + 1, i));
            }
        }

        return true;
    }

    public int cacheFingrprint(String txt) {
        int N = txt.length();
        if (N < chunk_num)
            throw new RuntimeException("the length of cache data less than " + chunk_num);

        int cache_num = N;
        long txtHash = hash(txt, chunk_num);
        int delay_time = 0;

        if ((txtHash & zero_value) == 0) {
            if (hashmap.get(txtHash>>zero_num) != null) {
                cache_num = cache_num - chunk_num + remedy;
                //StdOut.println("new is:" + (txtHash>>zero_num));
            }
        }

        for (int i = chunk_num; i < N; i++) {
            txtHash = (txtHash + Q - RM*txt.charAt(i-chunk_num) % Q) % Q;
            txtHash = (txtHash*R + txt.charAt(i)) % Q;

            if (delay_time == 0) {
                if ((txtHash & zero_value) == 0) {
                    if (hashmap.get(txtHash>>zero_num) != null)
                    {
                        ++count_comp;
                        cache_num = cache_num - chunk_num + remedy;
                        delay_time = chunk_num;
                        //StdOut.println("new is:" + (txtHash>>zero_num) + " and index is: " + i);
                        //StdOut.println("str is: " + txt.substring(i - chunk_num + 1, i));
                    }
                }
            }
            else
            {
                --delay_time;
            }
        }
        return cache_num;
    }

    // a random 31-bit prime
    private static long longRandomPrime() {
        BigInteger prime = BigInteger.probablePrime(31, new Random());
        return prime.longValue();
    }

    public static void main(String[] args) {
        String source_filename = args[0];
        String cache_filename = args[1];

        In in = new In(source_filename);
        String source = "";
        String txt = "";
        do {
            txt = in.readString();
            source += txt;
        } while (in.isEmpty() == false);

        In cache_in = new In(cache_filename);
        String cache = "";
        do {
            txt = cache_in.readString();
            cache += txt;
        } while (cache_in.isEmpty() == false);

        TCPRE searcher = new TCPRE(source);
        StdOut.println("old is: " + cache.length() + " and new is: " + searcher.cacheFingrprint(cache));
        StdOut.println("count is: " + searcher.count + " and del is: " + searcher.count_comp);

    }
}
