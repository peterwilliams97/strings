
#define ALPHA_UNKNOWN 0
#define ALPHA_DNA 1
#define ALPHA_RNA 2
#define ALPHA_PROTEIN 3
#define ALPHA_ASCII 4
#define NUM_PREDEF_ALPHA 4

int alpha_init(int), add_alphabet(char *);
char mapchar(int, char), rawmapchar(int, char);
int map_sequences(STRING *, STRING *, STRING **, int);
void unmap_sequences(STRING *, STRING *, STRING **, int);
void alpha_free(int);
int get_alpha_size(int alpha); 
