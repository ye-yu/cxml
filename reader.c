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
#include "reader.h"

static void _init(reader_context* context) {
  context->buffer=NULL;
  context->read_buffer=NULL;
  context->file=NULL;
}

static int _output_buffer_init(reader_context* context) {
  context->allocated = 64;
  context->length = 0;
  context->buffer = malloc(sizeof(char) * context->allocated);
  context->type = READER_STDIN;
  if (context->buffer == NULL) return 1 << 1;
  return 0;
}

int reader_stdin_init(reader_context* context) {
  _init(context);
  return _output_buffer_init(context);
}

int reader_file_init(reader_context* context, const char *filename) {
  _init(context);
  context->file = fopen(filename, "r");
  int error = _output_buffer_init(context) | ((context->file == NULL) << 1);
  context->type = READER_FILE;
  return error;
}

int reader_buffer_init(reader_context* context, const char *buffer) {
  _init(context);
  int error = 0;
  const size_t len = strlen(buffer) + 1;
  context->read_buffer = malloc(sizeof(char)*len);
  if (context->read_buffer == NULL) error |= 0b1;
  else {
    for (size_t i = 0; i <= len; i++) context->read_buffer[i] = '\0';
    strcpy(context->read_buffer, buffer);
  }
  context->read_pointer=0;
  error |= (_output_buffer_init(context) << 1);
  context->type = READER_BUFFER;
  return error;
}

static char _read_char(reader_context* context) {
  return context->read_buffer[context->read_pointer] == '\0'
    ? EOF
    : context->read_buffer[context->read_pointer++];
}

char reader_char(reader_context* context) {
  return context->type == READER_STDIN 
    ? getc(stdin)
    : context->type == READER_FILE
      ? getc(context->file)
      : _read_char(context);
}

int reader_cpy(reader_context* context, char *dest) {
  strcpy(dest, context->buffer);
}

static int _clear_buffer(reader_context* context) {
  if (context->buffer == NULL) return 1;
  for (size_t i = 0; i < context->allocated; i++) context->buffer[i] = '\0';
  context->length = 0;
  return 0;
}

int allocate_more(reader_context* context) {
  char *temp = context->buffer;
  context->allocated *= 2;
  context->buffer = malloc(sizeof(char) * context->allocated);
  if (context->buffer == NULL) {
    context->buffer = temp;
    return 1;
  }
  strcpy(context->buffer, temp);
  free(temp);
  #ifdef DEBUG
  printf("Allocated new buffer: $d\n bytes", context->allocated);
  #endif
  return 0;
}

static int _append(reader_context* context, char c) {
  if ((context->length + 1.0) / context->allocated > 0.75 && allocate_more(context)) return 1;
  char c2str[2] = { c, '\0'};
  strcat(context->buffer, c2str);
  context->length++;
  return 0;
}

static int _char_is_in(char c, char*buf, size_t count) {
  for (size_t i = 0; i < count; i++) if (c == buf[i]) return i;
  return -1;
}

int reader_tobuffer(reader_context* context, size_t count, ...) {
  // an error has occured
  if (_clear_buffer(context)) {
    fprintf(stderr, "Cannot clear buffer before reading stdin.");
    return -2;
  }

  va_list ap;
  va_start(ap, count);
  char *delims = malloc(sizeof(char)*count);
  for (size_t i = 0; i < count; i++) delims[i] = va_arg(ap, int);  

  char c = 0;
  int matched_position = -1;
  while((c = reader_char(context)) != EOF) {
    if ((matched_position = _char_is_in(c, delims, count)) != -1) goto cleanup;
    _append(context, c);
  }

  cleanup:
  va_end(ap);
  free(delims);
  return matched_position;
}

int reader_line(reader_context* context) {
  return reader_tobuffer(context, 1, '\n');
}

int reader_word(reader_context* context) {
  return reader_tobuffer(context, 3, ' ', '\t', '\n');
}

int reader_size(reader_context* context) {
  return context->length;
}

void reader_close(reader_context* context) {
  if (context->file != NULL) fclose(context->file);
  if (context->buffer != NULL) free(context->buffer);
  if (context->read_buffer != NULL) free(context->read_buffer);
}

