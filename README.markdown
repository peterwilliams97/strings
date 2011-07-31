# Set of tools to perform text analysis

## Finding repeated substrings in a set of strings
These programs find the longest substring that is repeated a specified minimum number of times a 
list of  strings. The number of repeats may different for each string.

In the sample code, the strings are files with the minimum number of occcurrences of the substring 
encoded in their names.

## find_repeated_substrings_offset.py    
### Sketch of Algorithm
>    for k = 4 to K
>        allowed_substrings = None
>        for s = shortest to longest string
>            r = number of repeats required for s
>            all_substrings = all substrings of length k in s that occur >= r times
>            if allowed_substrings
>                allowed_substrings = intersection(all_substrings, allowed_substrings)
>            else    
>                allowed_substrings = all_substrings
>            offsets[s] = offsets of allowed_substrings in s

### Peformance
There are several aspects of the code that give good typical runtimes
len(allowed_substrings) tends not to increase much. If the first string searched is short enough 
        then len(allowed_substrings) can start at around 100-200
for k > 4 the length k+1 substrings are generated from the length k stings by searching 1
        character forward and back. This is 
        running_time <= 2*len(allowed_substrings)*number of strings*(K-4)*string_match(K)

For typical values of 
       starting len(allowed_substrings) 100
       number of strings 60
       K 40
this gives       
       running_time <= 2 * 100 * 60 * 40 * 40 = 19,200,000 

### Accuracy
The -t command line option finds the longest substring from several subsets of input texts. 
       
##Plans for Improvement
+Using [rolling hashes](https://github.com/lemire/rollinghashjava) instead of python dicts/sets
+Possibly port to [Cython](http://cython.org/)
+Possibly convert the whole thing to Suffix Arrays
+Do approximate matching.