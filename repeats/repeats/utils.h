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
