#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <sstream>
#include <list>
#include <map>
#include <set>
#include <vector>
#include <algorithm>
#include <cmath>
#include "mytypes.h"

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
get_keys_list(const std::map<K,V> mp) {
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
    std::vector<K> keys;
    keys.reserve(mp.size());
    for (std::map<K,V>::const_iterator it = mp.begin(); it != mp.end(); it++) {
        keys.push_back(it->first);
    }
#if 0
    int count = 0;
    K val;
    K total = K();
    for (std::vector<K>::const_iterator it = keys.begin(); it != keys.end(); it++) {
        count++;
        val = *it;
        total += val;
    }
    assert(count == keys.size());
#endif 
    return keys;
}

/*
 * Return keys of map mp as a set
 */
template <class K, class V>
std::set<K> 
get_keys_set(const std::map<K,V> mp) {
    std::vector<K> keys = get_keys_vector(mp);
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
print_vector(const std::string &name, const std::vector<T> &lst, size_t n = std::numeric_limits<size_t>::max();) {
    cout << name << ": " << lst.size() << " [";
    std::vector<T>::const_iterator end = lst.begin() + min(n, lst.size());
    for (std::vector<T>::const_iterator it = lst.begin(); it != end; ++it) {
	cout << *it << ", ";
    }  
    cout << "] "  << lst.size() << endl;
}

/*
 * Print vector set to stdout
 */
template <class T>
void 
print_set(const std::string &name, const std::set<T> &lst) {
    cout << name << ": " << lst.size() << " [";
    for (std::set<T>::const_iterator it = lst.begin(); it != lst.end(); ++it) {
	cout << *it << ", ";
    }  
    cout << "]" << endl;
}

template <class K, class V>
size_t
get_map_vector_size(const std::map<K,std::vector<V>> mp) {
    size_t size = 0;
    for (std::map<K,std::vector<V>>::const_iterator it = mp.begin(); it != mp.end(); it++) {
        size += it->second.size(); 
    }
    return size;
}

/*
 * Return smallest x: *begin <= x < *end &&  x >= val
 *  or end if val > *end
 */
template <class T>
typename std::vector<T>::const_iterator
get_gteq(typename std::vector<T>::const_iterator begin, 
         typename std::vector<T>::const_iterator end, 
         const T val) { 
   
    // val <= smallest element in array so return smallest element
    if (val <= *begin) {
        return begin;
    }
   
    begin++;
    if (begin == end)  {
        return end;
    }
       
    std::vector<T>::const_iterator ge = std::upper_bound(begin, end, val) - 1;
    return (val == *ge) ? ge : ge + 1;
}

/*
 * Return smallest x: *begin2 <= x < *end2 &&  x >= val
 *  or end2 if val > *end2
 */
template <class T>
typename std::vector<T>::const_iterator
get_gteq2(typename std::vector<T>::const_iterator begin2, 
         typename std::vector<T>::const_iterator end2, 
         const T val, size_t step_size) { 

    // val <= smallest element in array so return smallest element
    if (val <= *begin2) {
        return begin2;
    }
   
    begin2++;
    if (begin2 == end2)  {
        return end2;
    }

    // As far as we can go in full steps of step_size
    std::vector<T>::const_iterator end1 = begin2 + ((end2-begin2)/step_size)*step_size; 

    // Step through range in steps of step_size
    for (std::vector<T>::const_iterator begin = begin2; begin < end1; begin += step_size) {
        std::vector<T>::const_iterator end = begin + step_size;   
        if (val <= *(end - 1)) { 
            // We are in range [begin, end)  
            std::vector<T>::const_iterator ge = std::upper_bound(begin, end, val) - 1;
            return (val == *ge) ? ge : ge + 1;
        }
    }

    // Handle left-over
    if (end1 < end2) {    
        std::vector<T>::const_iterator ge = std::upper_bound(end1, end2, val) - 1;
        return (val == *ge) ? ge : ge + 1;
    }

    // No match. 
    return end2;
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

#define LOG2 0.69314718055994529

/*
 * Return smallest power of 2 >= x
 */
inline size_t
next_power2(double x) {
    return (size_t)pow(2.0, ceil(log(x)/LOG2));
}

// Functions in utils.cpp
int string_to_int(const std::string s); 
size_t get_file_size(const std::string filename);
byte *read_file(const std::string filename);

#endif // #define UTILS_H
