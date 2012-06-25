Find Repeated Patterns in Documents
===================================

Finding Longest Repeated Substring
----------------------------------
In this sub-project we solve the sub-problem of finding the longest string
that is repeated n[i] times in documents i = 1..N

Find longest substring that occurs >= num[i] times in documents doc[i] for i = 1..N 
Each doc[i] is size[i] bytes long.

Examples
--------
Make documents with repeated strings with make_repeats.py in this directory 

    python make_repeats.py

The Basic Idea
--------------
The naive solution for finding the longest substring that occurs >= num[i] times in documents doc[i] for i = 1..N
will be hopelessly inefficient.

This would be to search for all substrings of 1..size[i]/num[i] for i = 1..N, and find the longest one 
that occurs >= num[i] times in doc[i] for all i

This would take approximately size[i] * (size[i]/num[i])^2 * N which is N * size ^ 3.
For 10 x 10 MB documents this would be 10 ^ 22 * some constant factor. Way too slow

We can do much better than this by observing that for each string s of length n
that occurs >= num[i] times, all substrings of s must also occur at least n times. 

Therefore we can build a list of strings recursively   
>
>   find all characters that occur 
>
>
>
>
 
These programs find the longest substring that is repeated a specified minimum number of times a 
list of strings. The number of repeats may different for each string.

In the sample code, the strings are files with the minimum number of occcurrences of the substring 
encoded in their names.

Sketch of Algorithm
-------------------   
>   for k = 4 to K

>       allowed_substrings = None
>       for s = shortest to longest string
>            r = number of repeats required for s
>            all_substrings = all substrings of length k in s that occur >= r times
>            if allowed_substrings
>                allowed_substrings = intersection(all_substrings, allowed_substrings)
>            else    
>                allowed_substrings = all_substrings
>            offsets[s] = offsets of allowed_substrings in s

Usage: 
    python find_repeated.py <file mask>
    python fr.py <file mask>
    repeats 

Performance
-----------
There are several aspects of the code that give good typical runtimes:

* Using [rolling hashes](https://github.com/lemire/rollinghashjava) instead of python dicts/sets allow 
    the substrings to be matched in O(sum(length of strings)) See [Cython](http://cython.org/) 
    implementation in rolling_hash.pyx
* len(allowed_substrings) tends not to increase much. If the first string searched is short enough 
    then len(allowed_substrings) can start at around 100-200
* for k > 4 the length k+1 substrings are generated from the length k stings by searching 1
    character forward and back. This is 
    running_time <= 2*len(allowed_substrings)*number of strings*(K-4)*string_match(K)

For typical values of: 

* starting len(allowed_substrings) 100
* number of strings 60
* K 40

this gives:
* running_time <= 2 * 100 * 60 * 40 * 40 = 19,200,000 

### Accuracy
The -t command line option finds the longest substring from several subsets of input texts. 
       
Plans for Improvement
---------------------
* Increase speed of _get_pattern_offsets()
* Pass longer substrings in rolling hash as speed independent of substring length  
* Possibly convert the whole thing to Suffix Arrays
* Do approximate matching

TODO
----
Try http://dlib.net/
