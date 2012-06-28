#ifndef MYTYPES_H
#define MYTYPES_H

// We will always work at byte granularity as it is gives complete generality
typedef unsigned char byte;

// We encode offsets as 4 byte integers so that we get at most x4 increase in 
//  size over the raw data
typedef unsigned int offset_t;
typedef offset_t *p_offset_t;

#define VERBOSITY 1

#endif // #ifndef MYTYPES_H
