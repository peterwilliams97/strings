#include <stdlib.h>
#include <vector>
#include "timer.h"
#include "rabinkarphash.h"


using namespace std;

// Uncomment the following line to compute the min and max hash values
//#define FIND_MIN_MAX

static const int WORDSIZE = 19;

/*
 * What we do when we get a hit on the hash table
 */
static void do_something()
{

}

/*
 * Make numchars of chartype points with random values
 */
static chartype *make_test_data(int numchars)
{
    chartype *data = new chartype[numchars];
    srand(1);
    for (int k = 0; k < numchars; k++) {
         data[k] = rand();
    }
    return data;
}

/*
 * Make a fake hash table to look stuff up in
 * 
 * Params:
 *      size = number of entries in hash table
 *      fraction_on = fraction of entries that lead to action
 */
typedef unsigned char byte; 
static byte *make_hash_table(int size, double fraction_on) 
{
    byte *table = new byte[size];
    memset(table, 0, size*sizeof(byte));

    int target_on = (int)(fraction_on * (double)size);
    int number_on = 0;
    while (number_on < target_on) {
        int i = rand() % size;
        if (!table[i]) {
            table[i] = 1;
            number_on++;
        }
    }
    cout << "make_hash_table: size=" << size << ",fraction_on=" << fraction_on <<  ",number_on=" << number_on << endl;
    return table;
}

static vector<double> exercise_hash(int n, const chartype *data, int numtests, int numchars)
{
    KarpRabinHash hf = KarpRabinHash(n);
    cout << "       n=" << hf._n << endl;  
    cout << "wordsize=" << hf._wordsize << " (" << (1 << hf._wordsize) << ")" << endl;

    unsigned char *hashtable = make_hash_table(1 << hf._wordsize, 0.001);
#ifdef FIND_MIN_MAX
    hashvaluetype min_hashval, max_hashval;
#endif
    
    timer_init();
    double start_time = get_elapsed_time();

    for (int times = 0; times < numtests; times++) {
        // Prime the first n has values
        for (int k = 0; k < n; k++) {
	    hf.eat(data[k]);
	}

#ifdef FIND_MIN_MAX        
        min_hashval = max_hashval = hf._hashvalue;
#endif
        // Compute the remainibng hash values by the rolling hash method
        for (int k = n; k < numchars; k++) {
	    hf.update(data[k-n], data[k]);
#ifdef FIND_MIN_MAX
            if (hf._hashvalue < min_hashval)
                min_hashval = hf._hashvalue;
            else if (hf._hashvalue > max_hashval)
                max_hashval = hf._hashvalue;
#endif
            // If there is a hit then do something
            if (hashtable[hf._hashvalue]) {
                do_something();
            }
	}
    }

    // Compute and display performance
    double end_time = get_elapsed_time();
    double duration = end_time - start_time;
    double num_hashes = (double)numtests * (double)numchars;

#ifdef FIND_MIN_MAX
    cout << "min hash=" << min_hashval << endl;
    cout << "max hash=" << max_hashval << endl;
#endif
    cout << "    time=" << duration << endl;
    cout << "hash/sec=" << num_hashes / duration << endl;

    vector<double> retval(2);
    retval[0] = duration;
    retval[1] = num_hashes;
    return retval;
}

static void test()
{
    int numtests = 5;
    int numchars = 100 * 1000 * 1000;
    
    cout << "numchars=" << numchars << endl;
    cout << "numtests=" << numtests << endl;
    cout << "=============================" << endl;
    
    chartype *data = make_test_data(numchars);
    int n_vals[] = {5, 10, 20, 50, 100};
    
    double duration = 0.0;
    double num_hashes = 0.0;
    for (int j = 0; j < numtests; j++) {
        cout << "test=" << j << endl;
        for (int i = 0; i < sizeof(n_vals)/sizeof(n_vals[0]); i++) {
            vector<double> retval = exercise_hash(n_vals[i], data, numtests, numchars);
            duration += retval[0];
            num_hashes += retval[1];
            cout << "-----------------------------" << endl;
        }
        cout << "=============================" << endl;
    }

    cout << "Totals" << endl; 
    cout << "    time=" << duration << endl;
    cout << "hash/sec=" << num_hashes / duration << endl;
   
    delete[] data;
}

#if 1
int main() 
{
    test();
    cout << "done!" << endl;
}
   
#endif

