#include "ReplacePattern.h"

//This class returns the changed bit stream used for querries kike ramk() rank )( select() and select)(

ReplacePattern::ReplacePattern(unsigned check, unsigned _sampleRate) {
    this->_sampleRate = _sampleRate;
    createtable(check);
}

ReplacePattern::~ReplacePattern() {
    delete [] _answer;
}

void ReplacePattern::createtable(unsigned check) {
    
    ulong k = 1 << _sampleRate;
    ulong *B = new ulong[_sampleRate/W + 1];
    ulong *C = new ulong[_sampleRate/W + 1];
    _answer = new ulong[_sampleRate*k/W + 1];
    
    // Iterate through all possible numbers of length  _sampleRate bits
    for (ulong i = 0; i < k; i++) {

        Tools::SetVariableField(B, _sampleRate, 0, i);

        for (ulong j = 0; j < _sampleRate - 1; j++) {
            if (check) {
                if ((Tools::GetField(B, 1, j)) && !(Tools::GetField(B, 1, j + 1)))
                    Tools::SetField(C, 1, j, 1); 
                else 
                    Tools::SetField(C, 1, j, 0);
            } else {
                if (!(Tools::GetField(B, 1, j)) && (Tools::GetField(B, 1, j + 1)))
                    Tools::SetField(C, 1, j, 1); 
                else 
                    Tools::SetField(C, 1, j, 0);
            }
        }
        Tools::SetField(C, 1, _sampleRate - 1, 0);

        Tools::SetField(_answer, _sampleRate, i, Tools::GetVariableField(C, _sampleRate, 0));
    }
    delete [] B;
    delete [] C;
}

// Replace pattern for vector of length W bits
ulong ReplacePattern::returnWord(ulong *data, ulong index, ulong n) {
    // Init result
    ulong result = 0;
    unsigned len = W;
    if (len + index > n)
        len = n - index;

    ulong k = len / (_sampleRate - 1);
    ulong i = index;
    ulong value;
    
    while (k > 0 && i + _sampleRate <= n)   {
        value = Tools::GetVariableField(data, _sampleRate, i);
        value = Tools::GetField(_answer, _sampleRate, value);
        
        result |= value << (i - index);
        
        k--;
        i += _sampleRate - 1;
    }

    if (i < index + len)  {
        k = index + len - i;
        if (index + len == n)
            value = Tools::GetVariableField(data, k, i);
        else
            value = Tools::GetVariableField(data, k+1, i);
        value = Tools::GetField(_answer, _sampleRate, value);
        result |= value << (i - index);
    }

    return result;
}

ulong *ReplacePattern::returnRP(ulong *data, ulong len, ulong index, ulong n) {
    if (len + index > n)
        len = n - index;
    ulong *A = new ulong[len / W + 1];
    for (ulong j = 0; j < len / W + 1; j++)
        A[j] = 0;
        
    ulong k = len / (_sampleRate - 1);
    ulong i = index;
    ulong value;
    
    while (k > 0 && i + _sampleRate <= n)  {
        value = Tools::GetVariableField(data, _sampleRate, i);
        value = Tools::GetField(_answer, _sampleRate, value);
        Tools::SetVariableField(A, _sampleRate - 1, i - index, value);
        
        k --;
        i += _sampleRate - 1;
    }

    if (i < index + len)  {
        k = index + len - i;
        
        if (index + len == n)
            value = Tools::GetVariableField(data, k, i);
        else
            value = Tools::GetVariableField(data, k+1, i);
        value = Tools::GetField(_answer, _sampleRate, value);
        Tools::SetVariableField(A, k, i - index, value);
    }
    return A;
}

