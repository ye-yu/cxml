/**
  * CXML - CSS-like XML transpiler
  * Copyright (C) 2021   Ye-Yu (rafolwen98@gmail.com)
  *
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
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
  size_t read_pointer;
  char *buffer;
  size_t allocated;
  size_t length;
} reader_context;


// >>> INITIALIZATION >>>

/* 
  initializes a context to read from standard input

  the returns 1 if there is an output buffer allocation error
  otherwise 0
*/
extern int reader_stdin_init(reader_context* context);

/* 
  a context to read from a specified file

  the returns 1 if there is an output buffer allocation error
  otherwise 0
*/
extern int reader_file_init(reader_context* context, const char *filename);

/* 
  a context to read from a specified string buffer

  the error is a bit field with the following specification:
    0b001 - read buffer allocation error
    0b010 - output buffer has been initialized
    0b100 - output buffer allocation error
*/
extern int reader_buffer_init(reader_context* context, char *buffer);

// <<< INITIALIZATION <<<

/* copies current read result into the destination */
extern void reader_cpy(reader_context* context, char *dest);
/* reads context by character */
extern char reader_char(reader_context* context);

/* Returns -2 on error, -1 on EOF, and >= 0 on matched delimiter*/
extern int reader_tobuffer(reader_context* context, size_t count, ...);
extern int reader_line(reader_context* context);
extern int reader_word(reader_context* context);
extern size_t reader_size(reader_context* context);
extern void reader_close(reader_context* context);
