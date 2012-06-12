#define NUMELEMS(a) (sizeof(a)/sizeof(a[0]))

template <class K, class V>
std::list<K> get_keys(const std::map<K,V> mp) {
    std::list<K> keys;
    for (std::map<K,V>::const_iterator it = mp.begin(); it != mp.end(); it++) {
        keys.push_back(it->first);
    }
    return keys;
}

template <class K, class V>
std::map<K,V> copy_map(const std::map<K,V> &mp) {
    map<K,V> result;
    for (std::map<K,V>::const_iterator it = mp.begin(); it != mp.end(); it++) {
        K key = it->first;
        V val = it->second; 
        result[key] = val;
    }
    return result;
}


template <class T>
void print_list(const std::string &name, const std::list<T> &lst) {
    cout << name << ": " << lst.size() << " [";
    for (std::list<T>::const_iterator it = lst.begin(); it != lst.end(); ++it) {
	cout << *it << ", ";
    }  
    cout << "]" << endl;
}

template <class T>
void print_vector(const std::string &name, const std::vector<T> &lst) {
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