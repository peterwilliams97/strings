void type_in_seq(void), delete_seq(void);
void list_sequences(int), print_seq(int);
void print_string(STRING *), terse_print_string(STRING *);
void fread_formatted(void), fread_unformatted(void);
void fwrite_formatted(void);
STRING *get_string(char *), **get_string_ary(char *, int *);
int get_bounded(char *, int, int, int);
char *get_seq_numbers(char *, int *);

STRING *make_seq(const char *title, const char *cstring);




