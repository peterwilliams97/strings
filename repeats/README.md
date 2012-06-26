Finding Longest Repeated Substring in Documents
===============================================
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
For 10 x 10 MB documents this would be 10 ^ 22 * some constant factor. That is way too slow.

We can do much better than this by observing that for each string s of length n
that occurs >= num[i] times, all substrings of s must also occur at least n times. 

Therefore we can build a list of strings recursively as follows
  
    Compute allowed_bytes = all strings of length 1 that occur required number of time in documents by scanning all documents once
    Compute allowed_strings[n] = strings of length i that occur required number of times, as follows
    allowed_strings[1] = allowed_bytes 
    for n = 1 .. 
       construct allowed_strings[n+1] as s + b for all strings s in allowed_strings[n] and all strings b in allowed_bytes
       remove from allowed_strings[n+1] all strings that do not end in a string from allowed_strings[n] 
       remove from allowed_strings[n+1] all strings that do not occur required number of time 
       if len(allowed_strings[n+1]) == 0 
           return allowed_strings[n]          

Implementation
--------------
This is implemented in the python files in this directory. fr.py is a terse implementation of the above pseudo-code while
find_repeated.py is more verbose but otherwise works exactly the same 

Usage: 
    python fr.py <file mask>
    python find_repeated.py <file mask>

Performance
-----------
The above code typical runtimes with the documents I work on because I choose documents with the following properties

* small repeat size (size of document / number of repeats)
* great variety, which tend to make it hard for a substring to occur the allowed number of times by accident

Typical behavior is

* len(allowed_bytes) tends to be 50 - 100 (compared to a possible maxium of 256)
* len(allowed_substrings) tends not to increase much. It tends to stay < 1000 for n = 2..5 then decrease
* once len(allowed_substrings) starts to decrease with increasing n, convergence follows fast
* for k > 4 the length k+1 substrings are generated from the length k stings by searching 1
    character forward and back. This is 
    running_time <= 2*len(allowed_substrings)*number of strings*(K-4)*string_match(K)

This gives running times of :

* typical worst case <= 5 * 1000 * number of bytes in all documents 
* typical  <= 3 * 200 * number of bytes in all documents 

Running times like this turn out to be from under a minute to up a minute for well-chosen documents of a few MBytes each 


TODO
----
Try http://dlib.net/
