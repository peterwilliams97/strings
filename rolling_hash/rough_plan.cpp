#include <memory.h>
#include <list>
#include <string>
#include <vector>
#include <map>
#include "rabinkarphash.h"
#include "lookahead.h"
#include "rough_plan.h"

/*
 * Rough plan for faster than disk-speed regular expressions
 *
 *  1) Run a rolling hash over input stream.
 *  2) Look up hash in "needs action" table
 *  3) If needs action then look up regex(es) and action functions(s) assocated with that hash
 *  4) For each match on regexes perform action
 *
 *  Setup is based on a set of regexes and action functions.
 *      Find longest static substring in all regexes 
 *      Compute hash h for each substring x
 *          Set needs_action[h] = 1
 *          Add regex, action, h to action table
 *
 */
using namespace std;

static const int WORDSIZE = 19;
static const int HASH_TABLE_SIZE = 1 << WORDSIZE;

// The "needs action" hash table
static byte *_potential_match_table = 0;

// Maximum number of bytes to look ahead
int _max_lookahead;

// A lookahead buffer
LookAheadBuffer *_lookahead;

static const byte *dup_data(int len, const byte *data)
{
    byte *dup = new byte[len];
    return (byte *)memcpy(dup, data, len*sizeof(byte));
}

BinString::BinString(int len, const byte *data):
    _len(len),
    _data(dup_data(len, data))
{   
}

const vector<byte> BinString::get_as_vector() const 
{
    vector<byte> v = vector<byte>(_len);
    for (int i = 0; i < _len; i++) {
        v[i] = _data[i];
    }
    return v;
}

/*
 * Create  "needs action" hash table with no entries 
 */
static byte *make_hash_table() 
{
    byte *table = new byte[HASH_TABLE_SIZE];
    memset(table, 0, HASH_TABLE_SIZE*sizeof(byte));
    return table;
}

static bool is_match(const RegexResults *results)
{
    return true;  // !@#$ stub
}

RegexResults *apply_regex(const BinString input, int regex_offset, const Regex *_regex)
{
    return 0; // !@#$ stub
}

static Regex *compile_regex(const BinString pattern)
{
    return 0; // !@#$ dummy value
}

class RegexAction 
{
public:  // Hack !@#$ Make this private and write accessor functions
    // User specified params
    const RegexActionParams _params;
        
    // Static string to hash on
    const BinString _static_string;

    // Offset of _static_string in _params._pattern
    const int _offset;

    // Hash value of the _static_string
    const hashvaluetype _static_string_hash;

    // Compiled regex
    const Regex *_regex;

public:
    RegexAction(const RegexActionParams params, const BinString static_string, int offset, hashvaluetype static_string_hash) :
        _params(params),
        _static_string(static_string),
        _offset(offset),
        _static_string_hash(static_string_hash),
        _regex(compile_regex(params._pattern)) 
        {}
};

/*
 * Return longest static string in a regex pattern
 */
static BinString get_largest_static_string(BinString pattern)
{
    return BinString(5, (const byte*)"Do this later"); // !@#$ dummy value
}

/*
 * Return offset of static string in a regex pattern
 */
static int get_offset(BinString pattern, BinString static_string)
{
    return 3;   // !@#$ dummy value
}

static int get_longest_string(const vector<RegexActionParams *> action_params_list)
{
    int longest = 0;
    for (vector<RegexActionParams *>::const_iterator it = action_params_list.begin(); it != action_params_list.end(); it++) {
        BinString s = get_largest_static_string((*it)->_pattern);
        if (s.get_len() > longest) {
            longest = s.get_len();
        }
    }
    return longest;
}


static KarpRabinHash *_hash_central = 0;

// _action_map[static_string_hash] is a list of actions which match static_string_hash 
static map<hashvaluetype, vector<const RegexAction *>> _action_map;

/*
 * Add a new action to _action_map
 */
static void add_to_action_map(const RegexAction* action) 
{
    hashvaluetype static_string_hash = action->_static_string_hash;
    if (_action_map.find(static_string_hash) == _action_map.end()) {
        _action_map[static_string_hash] = vector<const RegexAction *>();
    }
    _action_map[static_string_hash].push_back(action);
}

/*
 * Run through all the candidate regex's for a potential match and perform appropriate 
 *  actions on actual matches.
 * Params:
 *  input: Entire string being processed
 *  static_string_hash: rolling hash value
 *  static_string_offset: offset of start of static string in input
 * Returns: 
 *  true if all action functions that were run returned true
 */
static bool perform_actions(const BinString input, hashvaluetype static_string_hash, int static_string_offset)
{
     // This cannot happen by design
    assert(_action_map.find(static_string_hash) != _action_map.end());
    
    vector<const RegexAction *> action_list = _action_map[static_string_hash];

    for (vector<const RegexAction *>::iterator it = action_list.begin(); it != action_list.end(); it++) {
        const RegexAction* action = *it;
        
        // Offset of start of regex in input
        int regex_offset = static_string_offset - action->_offset;

        // Run the full regex on the data
        RegexResults *results = apply_regex(input, regex_offset, action->_regex);
        
        // If it is a match then perform the action function
        if (is_match(results)) {
            if (!action->_params._action_fn(input, results, regex_offset)) {
                return false;
            }
        }
    } 
    return true;
}

/*
 * Initialize the fast regex module.
 * Setup hash table and map of hash value to actions.
 */
bool fastregex_init(const vector<RegexActionParams *> action_params_list)
{
    // Set up the hash table
    _potential_match_table = make_hash_table();
    int hash_len = get_longest_string(action_params_list); 

    // Need this to be high as possible to reject as many non-matches as possible in rolling hash phase
    // 5 is a guess
    assert(hash_len >= 5);
    
    // Initialize the rolling hash
    _hash_central = new KarpRabinHash(hash_len);

     _max_lookahead = 0;
    // Build the action map
    for (int i = 0; i < (int)action_params_list.size(); i++) {
        BinString static_string(hash_len, action_params_list[i]->_pattern.get_data());
        int offset = get_offset(action_params_list[i]->_pattern, static_string);
        hashvaluetype static_string_hash = _hash_central->hash(static_string.get_as_vector());
        add_to_action_map(new RegexAction(*action_params_list[i], static_string, offset, static_string_hash));
        if (offset >  _max_lookahead) {
             _max_lookahead = offset;
        }
    }

    _lookahead = new LookAheadBuffer(_max_lookahead);
    return true;
}

void fastregex_term()
{
    // !@#$ Need to destroy all the tables created in fastregex_init()

}

/*
 * Process some data using fast regex's
 * 
 * Run through the data and compute a rolling hash for each byte
 *  If the rolling hash shows a potential match then call perform_actions()
 *      on the rolling hash value to find exact regex matches and run 
 *      appropriate functions on those exact matches.
 */
bool fastregex_process(const BinString input) 
{
    const byte *data = input.get_data();
    int numchars = input.get_len();
    int n = _hash_central->_n;
    
    // Prime the first n has values
    for (int k = 0; k < n; k++) {
        _hash_central->eat(data[k]);
    }

    // Compute the remaining hash values by the rolling hash method
    for (int k = n; k < numchars; k++) {
        _hash_central->update(data[k-n], data[k]);
        // If there is a hit then do something
        if (_potential_match_table[_hash_central->_hashvalue]) {
            if (!perform_actions(input, _hash_central->_hashvalue, k)) {
                return false;
            }
        }
    }

    return true;
}

/*
 * Process some data using fast regex's.
 *
 * Like fastregex_process() except that it guarantees the order of processing is the 
 *  same as if the regex's where performed sequentially on every byte on the input data.
 *
 * This is achieved with a lookahead buffer that runs all the regex's that match the
 *  the static string.
 *
 * Run through the data and compute a rolling hash for each byte
 *  If the rolling hash shows a potential match then call perform_actions()
 *      on the rolling hash value to find exact regex matches and run 
 *      appropriate functions on those exact matches.
 */

bool fastregex_process_in_order(const BinString input) 
{
    const byte *data = input.get_data();
    int numchars = input.get_len();
    int n = _hash_central->_n;          // Hash length
    
    // Prime the first n has values
    for (int offset = 0; offset < n; offset++) {
        _hash_central->eat(data[offset]);
    }

    // Compute the remaining hash values by the rolling hash method
    for (int offset = n; offset < numchars; offset++) {
        _hash_central->update(data[offset-n], data[offset]);
        if (_potential_match_table[_hash_central->_hashvalue]) {
            // If there is a hit then look ahead for all potential matches around this offset
            for (int i = 0; i < _max_lookahead; i++) {
                // Careful here. offset + i is the offset we are looking ahead to
                int lookahead_offset = offset + i;
                // !@#$ This is slow. We could re-use the rolling hash here
                hashvaluetype lookahead_hashvalue = _hash_central->get_hash(data + lookahead_offset);
                // If we have not already processessed this offset in a previous lookahead
                if (!_lookahead->contains(lookahead_offset)) {
                    // Match anywere in the look ahead?
                    if (_potential_match_table[lookahead_hashvalue]) {
                        // Act 
                        if (!perform_actions(input, lookahead_hashvalue, lookahead_offset)) {
                            return false;
                        }
                        // Mark this offset as done so we don't hit it again
                       _lookahead->push(lookahead_offset);
                    }
                }
            }
        }
    }

    return true;
}

