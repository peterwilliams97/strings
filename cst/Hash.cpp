
// Closed hash table

#include "Hash.h"

// creates a table to store up to n values with guaranteed load factor.
// vbits = # of bits per entry, ENTRIES CANNOT HAVE ZERO VALUE
Hash::Hash(ulong n, unsigned vbits, double factor)  { 
     ulong N = (ulong)(n*factor); 
     if (N <= n) 
        N = n+1;
     size = (1lu << Tools::bits(N-1)) - 1;
     bits = vbits;
     table = new ulong[((size+1)*vbits+W-1)/W];
#ifdef INDEXREPORT
       printf ("     Also created hash table of %i bits\n",
		  (((size+1)*vbits+W-1)/W)*W);
#endif
     for (ulong i=0; i < (((size+1)*vbits+W-1)/W); i++) 
        table[i] = 0;
}

Hash::~Hash() { 
    delete [] table;
}

// inserts an entry, not prepared for overflow

void Hash::insertHash(ulong key, ulong value)  { 
    ulong pos = (key*PRIME1) & size;
    if (Tools::GetVariableField(table, bits, pos*bits) != 0) { 
        do { 
            pos = (pos + PRIME2) & size;
        } while (Tools::GetVariableField(table, bits, pos*bits));
    }
    Tools::SetVariableField(table, bits, pos*bits, value);
}

// looks for a key, returns first value (zero => no values)
// writes in pos a handle to get next values

unsigned Hash::searchHash(ulong key, ulong *h)   { 
    *h = (key*PRIME1) & size;
    return Tools::GetVariableField(table, bits, *h*bits);
   }

// gets following values using handle *pos, which is rewritten
// returns next value (zero => no more values)

unsigned Hash::nextHash (ulong *h)  {
    *h = (*h +PRIME2) & size;
    return Tools::GetVariableField(table, bits, *h*bits);
 }


