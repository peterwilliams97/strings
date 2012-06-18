#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <sstream>
#include <list>
#include <map>
#include <set>
#include <vector>
#include <algorithm>

#define NUMELEMS(a) (sizeof(a)/sizeof(a[0]))

/*
 * Convert a string to type T
 */
template <class T>
typename T
from_string(const std::string s, typename T &x) {
    stringstream str(s);
    str >> x;
    return x;
}

/*
 * Return keys of map mp as a list
 */
template <class K, class V>
std::list<K> 
get_keys(const std::map<K,V> mp) {
    std::list<K> keys;
    for (std::map<K,V>::const_iterator it = mp.begin(); it != mp.end(); it++) {
        keys.push_back(it->first);
    }
    return keys;
}

/*
 * Return keys of map mp as a vector
 */
template <class K, class V>
std::vector<K> 
get_keys_vector(const std::map<K,V> mp) {
    std::list<K> keys = get_keys(mp);
    return std::vector<K>(keys.begin(), keys.end());
}

/*
 * Return keys of map mp as a set
 */
template <class K, class V>
std::set<K> 
get_keys_set(const std::map<K,V> mp) {
    std::list<K> keys = get_keys(mp);
    return std::set<K>(keys.begin(), keys.end());
}

/*
 * Return a copy of map mp
 */
template <class K, class V>
std::map<K,V>
copy_map(const std::map<K,V> &mp) {
    map<K,V> result;
    for (std::map<K,V>::const_iterator it = mp.begin(); it != mp.end(); it++) {
        K key = it->first;
        V val = it->second; 
        result[key] = val;
    }
    return result;
}

/*
 * Print list lst to stdout
 */
template <class T>
void
print_list(const std::string &name, const std::list<T> &lst) {
    cout << name << ": " << lst.size() << " [";
    for (std::list<T>::const_iterator it = lst.begin(); it != lst.end(); ++it) {
	cout << *it << ", ";
    }  
    cout << "]" << endl;
}

/*
 * Print vector lst to stdout
 */
template <class T>
void 
 print_vector(const std::string &name, const std::vector<T> &lst) {
    cout << name << ": " << lst.size() << " [";
    for (std::vector<T>::const_iterator it = lst.begin(); it != lst.end(); ++it) {
	cout << *it << ", ";
    }  
    cout << "]" << endl;
}

/*
 * Return largest x: *begin <= x < *end &&  x <= val
 *  or end if val < *begin
 */
template <class T>
typename std::vector<T>::const_iterator
get_lteq(typename std::vector<T>::const_iterator begin, 
         typename std::vector<T>::const_iterator end, 
         const T val) { 
    if (val < *begin)
        return end;
    else if (val > *(end - 1)) 
        return end - 1;
    else 
        return std::upper_bound(begin, end, val) - 1;
}

/*
 * Return smallest x: *begin <= x < *end &&  x > val
 *  or end if val > *end
 */
template <class T>
typename std::vector<T>::const_iterator
get_gt(typename std::vector<T>::const_iterator begin, 
         typename std::vector<T>::const_iterator end, 
         const T val) { 
    if (val < *begin)
        return begin;
    else if (val > *(end - 1)) 
        return end;
    else 
        return std::upper_bound(begin, end, val);
}

/*
 * Return intersection of v1 and v2
 * Try to make v1 the smaller set as run time is
 *  v1.size() * log(v2.size())
 */
template <class T>
typename std::set<T>
get_intersection(typename const std::set<T> &v1, 
                 typename const std::set<T> &v2) {
    std::set<T> v;
    for (std::set<T>::const_iterator it = v1.begin(); it != v1.end(); it++) {
        if (v2.find(*it) != v2.end()) {
            v.insert(*it);
        }
    }
    return v;
} 
    
/*
 * Trim mp to contain only the keys in keys
 */
template <class K, class V>
void
trim_keys(std::map<K,V> &mp, const std::set<K> &keys) {
    std::vector<K> map_keys = get_keys_vector(mp);
    for (std::vector<K>::iterator it = map_keys.begin(); it < map_keys.end(); it++) {
        if (keys.find(*it) == keys.end()) {
            mp.erase(*it);
        } 
    } 
}

// Functions in utils.cpp
int string_to_int(const std::string s); 
size_t get_file_size(const std::string fn);

#endif // #define UTILS_H
