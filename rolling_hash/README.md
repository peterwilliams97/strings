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

Currently getting 100 MB/sec/core on an AMD Phenom 2.2 GHz (approx 45 MB/sec/core/GHz).

The default 19 bit rolling hash gives 512k possible values. 

Therefore if there are, say, 100 regex's 
being run on the data and their static sub-strings are well spread and the input is random 
then the number of file offsets sent for sent for exact regex matching should be 
~ 100 in 512K ~ 1 in 5000  ~ 200 per MB.

Thus overall throughput is expected to be ~45 MB/sec/core/GHz - 512KB/(time to process 1 regex * num regex's).

Of course there is no reason for the regex evaluator or the action functions to run in
the same thread as the rolling hash so it is possible to implement this as a pipeline
  rolling hash -> regex evaluator -> action function

This would make use of extra cores to get 45 MB/sec/GHz on one core and produce, say, 135 MB/sec
processing speed on a 4 core 3 GHz computer. However this would add some complexity so we would need
to measure the actual gain before attempting this.

Will it work?
-------------
That depends on 
* the length of the shortest static string in all the regexes being evaluated
* the number of hash collisions in the input data


Notes
-----
Rolling hash code is copied from https://github.com/lemire/rollinghashcpp and slightly modified.
That code is under the Apache 2.0 license. 


