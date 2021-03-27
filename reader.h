#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

typedef enum _readtype {
  READER_FILE,
  READER_STDIN,
  READER_BUFFER
} reader_readtype;

typedef struct _buffercontext {
  FILE *file;
  reader_readtype type;
  char *read_buffer;
  int read_pointer;
  char *buffer;
  int allocated;
  int length;
} reader_context;


// >>> INITIALIZATION >>>

/* 
  initializes a context to read from standard input

  the return value is an error of a bit field with the following specification:
    0b01 - output buffer has been initialized
    0b10 - output buffer allocation error
*/
extern int reader_init(reader_context* context);

/* 
  a context to read from a specified file

  the return value is an error of a bit field with the following specification:
    0b001 - file pointer error
    0b010 - output buffer has been initialized
    0b100 - output buffer allocation error
*/
extern int reader_file_init(reader_context* context, const char *filename);

/* 
  a context to read from a specified string buffer

  the error is a bit field with the following specification:
    0b0001 - the current read buffer has been initialized
    0b0010 - read buffer allocation error
    0b0100 - output buffer has been initialized
    0b1000 - output buffer allocation error
*/
extern int reader_buffer_init(reader_context* context, const char *buffer);

// <<< INITIALIZATION <<<

/* copies current read result into the destination */
extern int reader_cpy(reader_context* context, char *dest);
/* reads context by character */
extern char reader_char(reader_context* context);

/* Returns -2 on error, -1 on EOF, and >= 0 on matched delimiter*/
extern int reader_tobuffer(reader_context* context, size_t count, ...);
extern int reader_line(reader_context* context);
extern int reader_word(reader_context* context);
extern int reader_size(reader_context* context);
extern void reader_close(reader_context* context);
