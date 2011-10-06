///*
// * more.c
// *
// * A set of procedures which provides a 'more'-like interface for the 
// * output passed through it.
// *
// * USE INSTRUCTIONS
// *
// *     This package is made of five main procedures: mstart, mputc,
// *     mputs, mprintf, mend.  Mputc, mputs and mprintf are the procedures
// *     used to pass output through the more interface. Procedures mstart
// *     and mend signal the beginning and end of the output printing (or that
// *     round of output printing).  When the more interface is running, 
// *     the output is sent in pages to the tty and the user is queried after
// *     each page.  The commands the users can give are the following:
// *         
// *         <space>              Display next screenful
// *         <return>             Display next line
// *         d or D or ctrl-D     Display next half screenful
// *         h or H or ?          Display this message
// *         q or Q               End text display
// *
// *
// *     The procedures mputc, mputs and mprintf have the same arguments and
// *     and functionality as putchar, puts and printf, except that
// *        1) mputs does NOT append a newline character to the end of the
// *           output stream (acting, instead, the way fputs does).
// *        2) the return values for the functions are different.  The three
// *           procedures here all return a 1 for a successful write, the value
// *           EOF for an error during the write, and a 0 if, while running the
// *           'more' interface, the user quits the printing.  Once the user
// *           'quits' the printing, nothing is written to the output and
// *           zeroes are returned (until mend and then mstart is used to start
// *           a new printing).
// *
// *    For completeness, here are the prototypes:
// *
// *            int mputc(char c)
// *
// *            int mputs(char *s)
// *
// *            int mprintf(char *format [ , args ] ...)
// *
// *
// *     Mstart must be called before any calls to the printing procedures in
// *     order to use the more-like interface (mputc, puts and mprintf will
// *     print to stdout if called before mstart).  The cycle of 1) mstart, 
// *     2) any number of mputc, mputs and mprintf calls, and 3) mend, however,
// *     can occur multiple times in the program.
// *
// *     The prototypes for mstart and mend are as follows:
// *
// *         void mstart(FILE *infp, FILE *outfp, int go_flag, int tty_flag,
// *                     int init_lines, void (*quitfn)())
// *
// *         void mend(int follow_lines)
// *
// *     infp and outfp are the input and output streams used during calls
// *     to mputc, mputs and mprintf.  The more interface is only run when
// *     both of these streams are connected to tty's.  If either stream
// *     is connected to a pipe, file or something other than a tty, the 
// *     print procedures simply call fputc, fputs and fprintf using the
// *     output stream and never use the input stream.
// *
// *     The go_flag signals whether or not to run the more interface.  If
// *     its value is non-zero, the interface runs or not as described in
// *     the previous paragraph.  If its value is zero, the print procedures
// *     just call fputc, fputs, and fprintf and don't use the input stream.
// *
// *     The tty_flag tells whether or not the terminal settings should be
// *     handled here.  The more interface requires that the terminal be set
// *     with either the RAW or CBREAK flags on and the ECHO flag off.  
// *     Unexpected results will occur if these terminal settings are not made.
// *     If the tty_flag value is non-zero, the terminal is reset with those
// *     settings, and signal handlers are installed to restore the terminal
// *     when the common Unix signals are generated.  When mend is called,
// *     the old terminal settings and signal handlers are restored.  If the
// *     value is zero, the procedures here assume that the rest of the program
// *     will deal with the terminal settings.  
// *     NOTE1:  If the tty_flag is non-zero, this means that mend must be
// *             called before returning from the main procedure or before
// *             calling 'exit'.  If mend is not called, the terminal will
// *             not be restored.
// *     NOTE2:  All the signal handler routines here do is first restore
// *             the terminal, reset that signal's handler to the procedure
// *             installed when mstart was called, and regenerate the signal
// *             (to let the original handler deal with it).  If that signal 
// *             handler returns, then the handler here backs out (reinstalling
// *             the signal handler here and resetting the terminal setting
// *             for the more interface).
// *             What this means is that the program can change the signal
// *             handlers, as long as those changes are done before the call
// *             to mstart.
// *     NOTE3:  The signals handled here are SIGHUP, SIGINT, SIGQUIT,
// *             SIGILL, SIGBUS, SIGSEGV, SIGSYS, SIGPIPE, SIGALRM,
// *             SIGTERM and SIGTSTP.
// *
// *     The init_lines value gives the number of lines which the more
// *     interface should consider as having been written to the screen
// *     already.  The first page size is reduced by that amount so as not
// *     to push those lines past the top of the screen.
// *
// *     Similarly, the follow_lines argument to mend gives the number of
// *     lines of output that will follow what has been shown.  If that number
// *     plus the number of lines of the current page is greater than the
// *     screen size, the interface will ask the user to continue one more
// *     time.  This ensures that the last page of the output does not push
// *     past the top of the screen because of the lines output after mend
// *     is called.
// *
// *     If the quitfn value is not NULL, the quitfn function is called
// *     when the user 'quits' the printing.  This function can be used
// *     to provide an quick way to exit the program when the user no
// *     longer wants to see the output (easier than checking every
// *     call to the print procedures for a 0 return value).
// *
// *     This function, however, does not need to exit and can return normally.
// *     If the function does return, then a 0 value is return by that and 
// *     all future calls to mputc, mputs and mprintf (until mend is called).
// *
// *
// * NOTES
// *    8/94  -  Original Implementation.  (James Knight)
// *    9/94  -  Converted it to ANSI format.  (James Knight)
// *    8/95  -  Changed to using stdarg from varargs and 
// *             added the follow_lines argument to mend.  (James Knight)
// *   10/97  -  Quick (and dirty?) conversion to sun gcc 2.7.2.3 (Jens Stoye)
// *    6/98  -  Added POSIX terminal handling for Linux (glibc). Tested on a 
// *             RedHat 5.1 system. Added conditionals for compilation under 
// *             ultrix, sun and linux (Christian N.S Pedersen)
// */
//  
//#include <stdio.h>
//#include <errno.h>
//#include <signal.h>
//#include <stdarg.h>
//
///*
// * Check if we compile on a known architecture
// */
//#if !defined(__ultrix) && !defined(__sun) && !defined(__linux)
//#define __unknown
//#endif
//
//#if defined(__ultrix) || defined(__unknown)
////#include <sgtty.h> !@#$
////#include <sys/ioctl.h>
//#elif defined(__sun)
//#include <sgtty.h>
//#include <unistd.h>
//#include <sys/ioctl.h>
//#include <sys/ttold.h>
//#elif defined(__linux)
//#include <termios.h>
//#include <unistd.h>
//#include <sys/ioctl.h>
//#endif
//
//#include <stdlib.h>
//#include "more.h"
//
///*
// * External Functions (not declared in .h files).
// */
//int isatty(int);
//
//#if defined(__ultrix) || defined(__unknown)
//int sigsetmask(int);
//int ioctl(int, int, void *);
//#elif defined(__sun)
//#define CBREAK 0x02 
//#endif
//
///*
// * Constants and variables defining terminal values.
// */
//#define DEFAULT_NUM_ROWS 24
//#define DEFAULT_NUM_COLS 80
//#define PAGE_OVERLAP 2
//
//#if defined(__ultrix) || defined(__sun) || defined(__ultrix)
//struct sgttyb term, oldterm;
//#elif defined(__linux)
//struct termios term, oldterm;
//#endif
//
////static struct winsize window;
//static int num_rows, num_cols, tty_flag, term_set;
//
//static void ((*handlers[50])());
//
///*
// * Other miscellaneous constants and variables.
// */
//typedef enum { NOTSTARTED, SIMPLEOUTPUT, MOREOUTPUT, SKIPOUTPUT } MODE;
//
//static MODE more_mode = NOTSTARTED;
//static int lines_to_go, col_pos;
//static char spf_buffer[5000];
//
//static int infd, outfd;
//static FILE *infp, *outfp;
//static void (*quitfn)();
//
//static char *regprompt = "--More--";
//static char *regerase = "\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b";
//static char *endprompt = "--End--";
//static char *enderase = "\b \b\b \b\b \b\b \b\b \b\b \b\b \b";
//
//static char *prompt_string, *erase_string;
//
//
///*
// * Internal prototypes.
// */
//static int prompt_for_more(void);
//static void get_term_info(void), set_tty(void), restore_tty(void);
//static void restore_handler(int, int);
//
///*
// * 
// * mstart, mputc, mputs, mprintf, mend
// *
// * The procedures implementing the more-like output.
// *
// *
// */
//
//void mstart(FILE *fp_in, FILE *fp_out, int go_flag, int setup_tty_flag,
//            int init_lines, void (*quit_fn)())
//{
//  infp = fp_in;
//  outfp = fp_out;
//  tty_flag = setup_tty_flag;
//  quitfn = quit_fn;
//
//  if (!go_flag)
//    more_mode = SIMPLEOUTPUT;
//  else {
//    if (infp == NULL) {
//      fprintf(stderr, "MORE:  Input file pointer argument is NULL.\n");
//      exit(1);
//    }
//    if (outfp == NULL) {
//      fprintf(stderr, "MORE:  Output file pointer argument is NULL.\n");
//      exit(1);
//    }
//    
//    infd = fileno(infp);
//    outfd = fileno(outfp);
//
//    if (!isatty(infd) || !isatty(outfd))
//      more_mode = SIMPLEOUTPUT;
//    else {
//      more_mode = MOREOUTPUT;
//
//      clearerr(infp);
//
//      get_term_info();
//      if (tty_flag)
//        set_tty();
//
//      num_rows -= PAGE_OVERLAP;
//      lines_to_go = num_rows;
//      if (init_lines > 0)
//        lines_to_go = (init_lines < num_rows ? num_rows - init_lines : 1);
//      
//      col_pos = 1;
//      prompt_string = regprompt;
//      erase_string = regerase;
//    }
//  }
//}
//
//
//int mputc(char c)
//{
//  switch (more_mode) {
//  case NOTSTARTED:
//    if (fputc(c, stdout) == EOF)
//      return EOF;
//    break;
//
//  case SIMPLEOUTPUT:
//    if (fputc(c, outfp) == EOF)
//      return EOF;
//    break;
//
//  case MOREOUTPUT:
//    if (fputc(c, outfp) == EOF)
//      return EOF;
//
//    if (c == '\n' && --lines_to_go == 0 && prompt_for_more() == 0)
//      return 0;
//    break;
//
//  case SKIPOUTPUT:
//    return 0;
//  }
//
//  return 1;
//}
//
//
//int mputs(char *s)
//{
//  char ch, *t;
//
//  switch (more_mode) {
//  case NOTSTARTED:
//    if (fputs(s, stdout) == EOF)
//      return EOF;
//    break;
//
//  case SIMPLEOUTPUT:
//    if (fputs(s, outfp) == EOF)
//      return EOF;
//    break;
//
//  case MOREOUTPUT:
//    for (t=s; *t; t++) {
//      if (*t == '\n' && --lines_to_go == 0) {
//        ch = *(t+1);
//        *(t+1) = '\0';
//        if (fputs(s, outfp) == EOF)
//          return EOF;
//        *(t+1) = ch;
//
//        if (!prompt_for_more())
//          return 0;
//
//        s = t + 1;
//      }
//    }
//    if (*s && fputs(s, outfp) == EOF)
//      return EOF;
//    break;
//
//  case SKIPOUTPUT:
//    return 0;
//  }
//
//  return 1;
//}
//
//
//int mprintf(char *format, ...)
//{
//  va_list ap;
//  char ch, *s, *t;
//
//  switch (more_mode) {
//  case NOTSTARTED:
//    va_start(ap, format);
//    vfprintf(stdout, format, ap);
//    break;
//
//  case SIMPLEOUTPUT:
//    va_start(ap, format);
//    vfprintf(outfp, format, ap);
//    break;
//
//  case MOREOUTPUT:
//    va_start(ap, format);
//    if (vsprintf(spf_buffer, format, ap) < 0)
//      return EOF;
//
//    s = spf_buffer;
//    for (t=s; *t; t++) {
//      if (*t == '\n' && --lines_to_go == 0) {
//        ch = *(t+1);
//        *(t+1) = '\0';
//        if (fputs(s, outfp) == EOF)
//          return EOF;
//        *(t+1) = ch;
//
//        if (!prompt_for_more())
//          return 0;
//
//        s = t + 1;
//      }
//    }
//    if (*s && fputs(s, outfp) == EOF)
//      return EOF;
//    break;
//
//  case SKIPOUTPUT:
//    return 0;
//  }
//
//  return 1;
//}
//
//
//void mend(int follow_lines)
//{
//  if (more_mode == NOTSTARTED)
//    return;
//
//  if (more_mode == MOREOUTPUT && follow_lines > 0 &&
//      follow_lines >= lines_to_go + PAGE_OVERLAP) {
//    prompt_string = endprompt;
//    erase_string = enderase;
//    prompt_for_more();
//    prompt_string = regprompt;
//    erase_string = regerase;
//  }
//
//  if (tty_flag && (more_mode == MOREOUTPUT || more_mode == SKIPOUTPUT))
//    restore_tty();
//
//  more_mode = NOTSTARTED;
//}
//
//
//
///*
// *
// * print_prompt, erase_prompt, prompt_for_more
// *
// * The interface procedures which prompt the user for more information.
// *
// * It returns either 1 if the user wants more output or 0 if the user
// * wants no more output.
// *
// *
// */
//
//static void print_prompt(void)
//{
//  fputs(prompt_string, outfp);
//  fflush(outfp);
//}
//
//
//static void erase_prompt(void)
//{
//  fputs(erase_string, outfp);
//  fflush(outfp);
//}
//
//
//static int prompt_for_more(void)
//{
//  char ch;
//
//  print_prompt();
//  while (1) {
//    if ((ch = fgetc(infp)) == EOF) {
//      if (ferror(infp) && errno == EINTR) {
//        clearerr(infp);
//        continue;
//      }
//      else {
//        lines_to_go = -1;     /*  This ensures that prompt_for_more is     *
//                               *  not called again during the current      *
//                               *  print procedure.  (After that the        *
//                               *  more_mode == SIMPLEOUTPUT takes effect.) */
//        more_mode = SIMPLEOUTPUT;
//        if (tty_flag)
//          restore_tty();
//        break;
//      }
//    }
//
//    switch (ch) {
//    case ' ':
//      lines_to_go = num_rows;
//      goto LOOP_END;
//
//    case 'h':
//    case 'H':
//    case '?':
//      fputc('\n', outfp);
//      fputs("<space>              Display next screenful\n", outfp);
//      fputs("<return>             Display next line\n", outfp);
//      fputs("d or D or ctrl-D     Display next half screenful\n", outfp);
//      fputs("h or H or ?          Display this message\n", outfp);
//      fputs("q or Q               End text display\n", outfp);
//      print_prompt();
//      break;
//
//    case '\n':
//      lines_to_go = 1;
//      goto LOOP_END;
//
//    case 'd':
//    case 'D':
//    case '\004':
//      lines_to_go = num_rows / 2;
//      goto LOOP_END;
//
//    case 'q':
//    case 'Q':
//      erase_prompt();
//      if (quitfn != NULL)
//        (*quitfn)();
//      more_mode = SKIPOUTPUT;
//      return 0;
//    }
//  }
//LOOP_END:
//  erase_prompt();
//  return 1;
//}
//
//
//
///*
// *
// * get_term_info, set_tty, restore_tty, restore_handler
// *
// * Procedures for getting and setting the terminal information necessary
// * to implement the more-like interface, and the signal handler routine
// * used to catch interrupts.
// *
// *
// */
//
//static void get_term_info(void)
//{
////  ioctl(infd, TIOCGWINSZ, &window);
////  num_rows = DEFAULT_NUM_ROWS;
////  if (window.ws_row > 0)
////    num_rows = window.ws_row;
////
////  num_cols = DEFAULT_NUM_COLS;
////  if (window.ws_col > 0)
////    num_cols = window.ws_col;
////
////#if defined(__ultrix) || defined(__sun) || defined(__unknown)
////  ioctl(infd, TIOCGETP, &oldterm);
////  term = oldterm;
////  term.sg_flags |= CBREAK;
////  term.sg_flags &= ~ECHO; 
////#elif defined(__linux)
////  tcgetattr(infd, &oldterm);
////  term = oldterm;
////  term.c_lflag &= ~ECHO;
////  term.c_lflag &= ~ICANON;
////#endif
//}
//
//static void set_tty(void)
//{
////  term_set = 1;
////#if defined(__ultrix) || defined(__sun) || defined(__unknown)
////  ioctl(infd, TIOCSETP, &term);
////#elif defined(__linux)
////  tcsetattr(infd, TCSANOW, &term);
////#endif
////  handlers[SIGHUP] = signal(SIGHUP, (void (*)()) restore_handler);
////  handlers[SIGINT] = signal(SIGINT, (void (*)()) restore_handler);
////  handlers[SIGQUIT] = signal(SIGQUIT, (void (*)()) restore_handler);
////  handlers[SIGILL] = signal(SIGILL, (void (*)()) restore_handler);
////  handlers[SIGBUS] = signal(SIGBUS, (void (*)()) restore_handler);
////  handlers[SIGSEGV] = signal(SIGSEGV, (void (*)()) restore_handler);
////#if defined(__ultrix) || defined(__sun) || defined(__unknown)
////  handlers[SIGSYS] = signal(SIGSYS, (void (*)()) restore_handler);
////#endif
////  handlers[SIGPIPE] = signal(SIGPIPE, (void (*)()) restore_handler);
////  handlers[SIGALRM] = signal(SIGALRM, (void (*)()) restore_handler);
////  handlers[SIGTERM] = signal(SIGTERM, (void (*)()) restore_handler);
////  handlers[SIGTSTP] = signal(SIGTSTP, (void (*)()) restore_handler);
//}
//
//
//static void restore_tty(void)
//{
////  signal(SIGHUP, handlers[SIGHUP]);
////  signal(SIGINT, handlers[SIGINT]);
////  signal(SIGQUIT, handlers[SIGQUIT]);
////  signal(SIGILL, handlers[SIGILL]);
////  signal(SIGBUS, handlers[SIGBUS]);
////  signal(SIGSEGV, handlers[SIGSEGV]);
////#if defined(__ultrix) || defined(__sun) || defined(__unknown)
////  signal(SIGSYS, handlers[SIGSYS]);
////#endif
////  signal(SIGPIPE, handlers[SIGPIPE]);
////  signal(SIGALRM, handlers[SIGALRM]);
////  signal(SIGTERM, handlers[SIGTERM]);
////  signal(SIGTSTP, handlers[SIGTSTP]);
////  if (term_set) {
////#if defined(__ultrix) || defined(__sun) || defined(__unknown)
////    ioctl(infd, TIOCSETP, &oldterm);
////#elif defined(__linux)
////    tcsetattr(infd, TCSANOW, &oldterm);
////#endif    
////    term_set = 0;
////  }
//}
//
//
//static void restore_handler(int sig, int code)
//{
////  if (more_mode == MOREOUTPUT && lines_to_go == 0)
////    erase_prompt();
////
////  if (term_set) {
////#if defined(__ultrix) || defined(__sun) || defined(__unknown)
////    ioctl(infd, TIOCSETP, &oldterm);
////#elif defined(__linux)
////    tcsetattr(infd, TCSANOW, &oldterm);
////#endif    
////    term_set = 0;
////  }
////
////  signal(sig, handlers[sig]);
////#if defined(__ultrix) || defined(__unknown)
////  sigsetmask(0);
////#endif
////  kill (0, sig);
////  /* Pause for forever or just a station break */
////
////  /* If we're back, the process was just suspended and restarted */
////  signal(sig, (void (*)()) restore_handler);
////
////  if (tty_flag && more_mode != NOTSTARTED) {
////    term_set = 1;
////#if defined(__ultrix) || defined(__sun) || defined(__unknown)
////    ioctl(infd, TIOCSETP, &term);
////#elif defined(__linux)
////    tcsetattr(infd, TCSANOW, &term);
////#endif    
////    if (more_mode == MOREOUTPUT && lines_to_go == 0) {
////      print_prompt();
////    }
////  }
//}
