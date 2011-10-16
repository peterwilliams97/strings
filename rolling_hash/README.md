Rolling Hash
============
Testing using a rolling hash as a noise filter for regular expression parsing.

Rough Plan
----------
rough_plan.cpp and rough_plan.h sketch out how the rolling hash noise filter would work with regex's.
A (fast) hash lookup is made on data before a full regex is applied to it.

Performance Estimates
---------------------
Test code is in faster_regex_test.cpp

Currently getting 170 MB/sec/core on an AMD Phenom 2.2 GHz (approx 75 MB/sec/core/GHz).

The default 19 bit rolling hash gives 512k possible values. 

Therefore if there are, say, 100 regex's 
being run on the data and their static sub-strings are well spread and the input is random 
then the number of file offsets sent for sent for exact regex matching should be 
~ 100 in 512K ~ 1 in 5000  ~ 200 per MB.

Thus overall throughput is expected to be ~75 MB/sec/core/GHz - 512KB/(time to process 1 regex * num regex's).

Notes
-----
Rolling hash code is copied from https://github.com/lemire/rollinghashcpp and slightly modified.
That code is under the Apache 2.0 license. 


