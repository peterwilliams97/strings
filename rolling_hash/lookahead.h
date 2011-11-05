#ifndef _LOOKAHEAD_H_
#define _LOOKAHEAD_H_

/*
 * The following is a very simple look ahead buffer
 * It should be efficient enough as 
 *      - contains() is called far more than push()
 *      - n > _highest + _max_lookahead far more often than it is not.
 */
class LookAheadBuffer {
    const int _max_lookahead;
    list<int> _lookahead_buffer;
    int _highest;
public:
    LookAheadBuffer(int max_lookahead) : _max_lookahead(max_lookahead) { _highest = 0; }
    
    bool contains(int n) 
    {
        if (n > _highest + _max_lookahead) {
            return false;
        }
        for (list<int>::iterator it = _lookahead_buffer.begin(); it != _lookahead_buffer.end(); it++) {
            if (*it == n) {
                return true;
            }
        }
        return false;
    }

    void push(int n)
    {
        if ((int)_lookahead_buffer.size() > _max_lookahead) {
           _lookahead_buffer.pop_front();
        }
        _lookahead_buffer.push_back(n);

        if (n > _highest) {
            _highest = n;
        }
    }
};


#endif  // _LOOKAHEAD_H_
