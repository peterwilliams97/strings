#ifndef KARPRABINHASH
#define KARPRABINHASH

#include "characterhash.h"

class KarpRabinHash 
{
    CharacterHash _hasher;
    static const hashvaluetype B = 37;
    const hashvaluetype _HASHMASK;
    hashvaluetype _BtoN;

public:
    const int _n, _wordsize;
    hashvaluetype _hashvalue;

    KarpRabinHash(int n, int wordsize=19) :  
        _n(n), 
        _wordsize(wordsize), 
        _hasher((1 << wordsize ) - 1),
        _HASHMASK((0x1 << wordsize) - 1),
        _BtoN(1) 
    {
       _hashvalue = 0;
        for (int i = 0; i < _n; i++) {
            _BtoN *= B;
	    _BtoN &= _HASHMASK;  
        }
    }
    
    template<class container> hashvaluetype hash(container &c) 
    {
        assert(c.size() == static_cast<uint>(_n));
    	hashvaluetype answer = 0;
    	for (int k = 0;  k < (int)c.size(); k++) {
    	    hashvaluetype x = 1;
    	    for (int j = 0; j < (int)c.size()-1-k; j++) {
    	        x = (x * B) & _HASHMASK;
    	    }
    	    x = (x * _hasher.hashvalues[c[k]]) & _HASHMASK;
    	    answer = (answer+x) & _HASHMASK;
    	}
    	return answer;
    }
    
    void eat(chartype inchar) 
    {
    	_hashvalue = (B*_hashvalue +  _hasher.hashvalues[inchar]) & _HASHMASK;
    }


    
    inline void update(chartype outchar, chartype inchar) 
    {
    	_hashvalue = (B*_hashvalue +  _hasher.hashvalues[inchar] - _BtoN *  _hasher.hashvalues[outchar]) & _HASHMASK; 
    } 



};

#endif


