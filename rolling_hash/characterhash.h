#ifndef CHARACTERHASH
#define CHARACTERHASH

typedef unsigned long long uint64;
typedef unsigned int uint32;
typedef unsigned int uint;

#include <cassert>
#include <iostream>
#include "mersennetwister.h"

using namespace std;

typedef uint32 hashvaluetype;
typedef unsigned char chartype;

class mersenneRNG {
    MTRand mtr;
    int n;
public:
    mersenneRNG(uint32 maxval) : mtr(25),n(maxval) {};
    uint32 operator()()         { return mtr.randInt(n);} 
    void seed(uint32 seedval)   { mtr.seed(seedval);}
    void seed()                 { mtr.seed();}
    uint32 rand_max()            { return n;}
};


struct CharacterHash 
{
    enum { nbrofchars = 1 << (8*sizeof(chartype)) };
    hashvaluetype hashvalues[nbrofchars];
    
    CharacterHash(uint32 maxval) 
    {
        mersenneRNG randomgenerator(maxval);
        for (int k = 0; k < nbrofchars; k++) {
  	    hashvalues[k] = randomgenerator();
        }
    }
};

#endif

