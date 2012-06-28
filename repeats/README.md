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
  
    Compute allowed_bytes = all strings of length 1 that occur required number of time in 
        documents by scanning all documents once
    Compute allowed_strings[n] = strings of length i that occur required number of times, 
        as follows
    allowed_strings[1] = allowed_bytes 
    for n = 1 .. 
       construct allowed_strings[n+1] as s + b for all strings s in allowed_strings[n] 
        and all strings b in allowed_bytes
       remove from allowed_strings[n+1] all strings that do not end in a string from allowed_strings[n] 
       remove from allowed_strings[n+1] all strings that do not occur required number of time 
       if len(allowed_strings[n+1]) == 0 
           return allowed_strings[n]          

Implementation
--------------
This is implemented in the python files in this directory. [fr.py](https://github.com/peterwilliams97/strings/blob/master/repeats/fr.py) 
is a terse implementation of the above pseudo-code while
[find_repeated.py](https://github.com/peterwilliams97/strings/blob/master/repeats/find_repeats.py) is more verbose but otherwise works exactly the same. 

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

* typical bad case <= 5 * 1000 * number of bytes in all documents 
* typical good case <= 3 * 200 * number of bytes in all documents 

Running times like this turn out to be from under a minute to up a minute for well-chosen documents of a few MBytes each 

Problems with the Basic Solution
--------------------------------
While [fr.py](https://github.com/peterwilliams97/strings/blob/master/repeats/fr.py) performs well for well-chosen
documents, its worst case performance is bad.

Worst case performance happens on large documents with little structure outside the repeated patterns. In this
case  

* len(allowed_bytes) is ~ 256
* len(allowed_substrings[n]) to grow at 256^n to its limit of doc.size / n / num.repeats, e.g 30^7/3/10 = 1,000,000
  for a 30 MByte document with 10 repeats where it maximizes at n = 3  
* for a corpus of 10 x 30 MByte documents, 300 MBytes of data would need to be seached 1,000,000 times which is
  approx 3*10^14 bytes of searching which is going take months on today's computers.

A Solution with Better Worst-Case Performance
---------------------------------------------
The big problem with the basic solution is that the number of substring searches grows too fast. This doesn't have to 
happen. 

After all, each of occurrences of the strings in allowed_substrings[n] in each document start with a string from 
allowed_substrings[n] by construction. Therefore the total number of occurrences of the of the strings in 
allowed_substrings[n+1] in each document must be less than or equal to the total number of occurrences of the of 
the strings in allowed_substrings[n].

The problem seems to be that our basic solution searches all of each document for each substring. We should be able
to achieve search times proportional to document size if we stored the offsets of all occurrences of each substring in
in each document, then searched only from there.  

A well-known way of storing documents in this way is an [inverted index](http://en.wikipedia.org/wiki/Inverted_index). 
There is a c++ implementation of an inverted index specialized for matching repeated substrings in
[inverted_index.cpp](https://github.com/peterwilliams97/strings/blob/master/repeats/repeats/inverted_index.cpp)

In inverted_index.cpp, 

*    inverted_index._postings_map[s]_offsets_map[i]
*    is a vector of offsets of string s in document number i

Thus the inverted_index of strings of length 1, or bytes, stores the entire contents of the corpus of documents. Each 
offset is 4 bytes long so the inverted index takes 4 bytes to store every byte in the corpus.    

This 4-fold increase in storage size gives us a big advantage. As we run our algorithm of constructing
allowed_substrings[n+1] from allowed_substrings[n], the worst case amount of searching does not increase 
exponentially with n as before. In fact it does not increase much at all.

To go from allowed_substrings[n] to allowed_substrings[n+1], we 

    construct
    inverted_index._postings_map[s]_offsets_map[i] for all s in allowed_substrings[n+1]
    from
    inverted_index._postings_map[s1]_offsets_map[i] for all s1 in allowed_substrings[n]
    by appending 
    inverted_index._postings_map[b]_offsets_map[i] for all b in allowed_substrings[1]
    
This is the same algorithm as before converted to inverted indexes. In this case we also need to update the inverted
index as we go. As the string lengths increase the number of allowed substrings increase and the
elements inverted_index._postings_map[s] increase, but the length of each vector of offsets
inverted_index._postings_map[s]_offsets_map[i] descreases as well so the total number of offsets stored does not 
increase.

The growth in processing time with the length of substrings being searched for depends 
on long it takes to contstuct the length n+1 inverted index from the length n inverted index. This is 
determined by how we construct the inverted index.

We do this by "[merging](http://www.sorting-algorithms.com/merge-sort)" 
([German version](http://bit.ly/MZaHJ0)) the sorted 
inverted_index._postings_map[s]_offsets_map[i] with the sorted 
inverted_index._postings_map[1]_offsets_map[i]. These vectors are sorted because 
inverted_index._postings_map[1]_offsets_map[i] is constructed sorted as it is built by scanning 
document i and merging, as we will see, preserves order.  

Worst-Case Performance of the Merge Solution
--------------------------------------------
    Assume a document has length m with r repeats
    It will take O(m) time to read the document and construct the inverted index.
    The nth merges m/256 bytes with m/(256^n) strings x 256^n times 
        ==> O(m*(1/256+1/(256^n))*256^n)  
        ==> O(m*(256^(n-1)))  

This is still exponential in n, the length of the substrings being tested for repeats! 

Does this matter?
    
Clearly it matters for if n is large. Therefore we must estimate the biggest m we will see in
real problems. 

This is straightforward to calculate for our worst case which is the maximum number of unique
substrings n: 256^n == size of repeat == m/r or n = log256(m/r) 

** 10 MByte document with 10 repeats ==> n=3
** 100 MByte document with 100 repeats ==> n=3.2

The documents that I test are usually less than 10 MByte so m*(256^(n-1)) is m*(256^2) which 
is not too bad. Therefore simple merging should work reasonably well. This is marked as 
INNER_LOOP==1 in [inverted_index.cpp](https://github.com/peterwilliams97/strings/blob/master/repeats/repeats/inverted_index.cpp)  

However we can do better.

Recall that we are merging m/256 bytes with m/(256^n) strings x 256^n times. The stepping through
the m/(256^n) strings x 256^n times takes constant time with respect to n. The problem is the
stepping through the constant number of bytes x 256^n. 

The byte offsets are sorted so we don't need to step linearly. We can step in a 
the bytes offsets in INNER_LOOP==1. We can divide the bytes offsets into a number equal regions of
of the length of the number of strings, then linearly step through the string offsets and after each 
one, binary searc the bytes offset region. The length of each byte offset region is 
(1/256) / (1/(256^n)) = 256^(n-1), so the binary search time is O(log(256^(n-1))) = O(n)

The total search time is  
    ==> O(m*(S+Bn)) where S and B are some constants. S for stepping through the substring 
            offsets and B for stepping through the byte offset
    ==> O(mn)

This growth stops when the maximum number of valid unique substrings is reached at 
 
    

TODO
----
Binary search on strings, linear search on bytes
Raw binary search
Multi-thread
Try http://dlib.net/
Test for substrings that are repeated more times that required
Construct worst case of the maximum number of unique _valid_ substrings