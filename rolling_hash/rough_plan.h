/*
 * Rough plan for faster than disk-speed regular expressions
 *
 *  1) Run a rolling hash over input stream.
 *  2) Look up hash in "needs action" table
 *  3) If needs action then look up regex(es) and action functions(s) associated with that hash
 *  4) For each match on regexes perform action
 *
 *  Setup is based on a set of regexes and action functions.
 *      Find longest static substring in all regexes 
 *      Compute hash h for each substring x
 *          Set needs_action[h] = 1
 *          Add regex, action, h to action table
 *
 *  Note: Since the offset of the regex is determined by the rolling hash look-up, it should be 
 *      possible to anchor the regex searches. This may save some more time.
 *
 */
#include <vector>

typedef unsigned char byte;

class Regex;
class RegexResults;

// Binary data will be processed.
class BinString
{
    const int _len;
    const byte *_data;
public:
    BinString(int len, const byte *data);
    ~BinString()  { delete[] _data; }
    int get_len() const  { return _len; }
    const byte *get_data() const { return _data; }
    const std::vector<byte> get_as_vector() const;
};

struct RegexActionParams
{
    // Regular expression pattern
    BinString _pattern;

    /* 
     * Action function that is called after a regex match. 
     * Params
     *  input is the entire string being processes
     *  results are regex results, 
     *  offset is offset of the start of the regex match into input.
     * Returns true on success.
     */
    bool (*_action_fn)(const BinString input, const RegexResults *results, int offset);
};

/*
 * Initialize the fast regex module.
 * Setup hash table and map of hash value to actions.
 */
bool fastregex_init(const std::vector<RegexActionParams *> action_params_list);
/*
 * Destroy all the data allocated in fastregex_init()
 */
void  fastregex_term();

/*
 * Process some data using fast regex's
 * 
 * Run through the data and compute a rolling hash for each byte
 *  If the rolling hash shows a potential match then call perform_actions()
 *      on the rolling hash value to find exact regex matches and run 
 *      appropriate functions on those exact matches.
 */
bool fastregex_process(const BinString input); 

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

bool fastregex_process_in_order(const BinString input); 