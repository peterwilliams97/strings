

/* shared return values */
#define FORMAT_ERROR -1

/* return values for read_sequence */
#define END 0
#define MISMATCH -2
#define PREMATURE_EOF -3

/* file I/O function prototypes */
int find_next_sequence(FILE *);
int read_sequence(FILE *, STRING *);
int write_sequence(FILE *, STRING *);
















