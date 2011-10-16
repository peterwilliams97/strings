Rolling Hash
============
Testing using a rolling hash as a noise filter of regular expression parsing.

Rough Plan
----------
rough_plan.cpp and rough_plan.h scetch out how the rolling hash noise filter would work with regex's.

Performance Estimates
---------------------
Test code is in faster_regex_test.cpp

Getting 170 MB/sec/core on an AMD Phenom 2.2 GHz (Approx 75 MB/sec/core/GHz)

The default 19 bit rolling hash gives a 512k possible values. 

Therefore if there are, say, 100 regex's 
being run on the data and their static sub-strings are well spread and the input is random 
then the number of file offsets sent for sent for exact regex matching should be 
~ 100 in 512K ~ 1 in 5000  ~ 200 per MB.

Overall throughput is ~75 MB/sec/core/GHz - (cost of processing 1 regex * num regex's / 512K)
