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

static int allocate_str(char **pointer, size_t current_allocation) {
  current_allocation = current_allocation == 0 ? 32 : current_allocation;
  char *temp = *pointer;
  *pointer = malloc(sizeof(char) * current_allocation * 2);
  if (*pointer == NULL) {
    *pointer = temp;
    return 0;
  }

  for (size_t i = 0; temp != NULL && i < current_allocation; i++) {
    (*pointer)[i] = temp[i];
  }
  
  free(temp);
  return current_allocation * 2;
}


static void empty_string(char *str, size_t count) {
  for (size_t i = 0; i < count; i++) str[i] = '\0';
}

static void print_indent(size_t count) {
  for (size_t i = 0; i < count; i++) putc(' ', stdout);
}

static int is_keyword_char(char c) {
  return c == '.' || c == '#' || c == '-' || ((c | 32) >= 'a' && (c | 32) <= 'z');
}

static int is_whitespace(char c) {
  return c == ' ' || c == '\t' || c == '\n';
}

static int only_whitespace(const char *buffer, const size_t size) {
  for (size_t i = 0; i < size; i++) {
    char c = buffer[i];
    if (c != ' ' && c != '\t' && c != '\n') return 0;
  }
  return 1;
}

void parse(reader_context context, const size_t level) {
  #ifdef DEBUG
  printf("-- entering level %d\n", level);
  #endif
  int matched;
  char *write_buffer = NULL;
  size_t write_buffer_allocation = 0;
  write_buffer_allocation = allocate_str(&write_buffer, write_buffer_allocation);
  empty_string(write_buffer, write_buffer_allocation);

  char *previous_token = NULL;
  size_t previous_token_allocation = 0;
  previous_token_allocation = allocate_str(&previous_token, previous_token_allocation);
  empty_string(previous_token, previous_token_allocation);

  char *previous_attribute = NULL;
  size_t  previous_attribute_allocation = 0;
  previous_attribute_allocation = allocate_str(&previous_attribute, previous_attribute_allocation);
  empty_string(previous_attribute, previous_attribute_allocation);

  int print_next_indent = 2;

  reader_context *context_p = &context;
  while((matched = reader_tobuffer(context_p, 5, ' ', '\t', '\n', '[', '{')) != -2) {
    const size_t reader_len = reader_size(context_p);
    if (matched == -1 && !reader_len) goto writer_cleanup;
    char *buffer = malloc(sizeof(char)*reader_len + 1);
    for (size_t i = 0; i <= reader_len; i++) buffer[i] = '\0';
    reader_cpy(context_p, buffer);
    #ifdef DEBUG
    printf("-- parsing token: %s\n", buffer);
    #endif

    switch (matched) {
      case -1:
      case 0:
      case 1:
      case 2: {
        if (strcmp(previous_token, "")) {
          #ifdef DEBUG
          printf("-- previous token\n");
          #endif
          print_indent((!!print_next_indent) * level * 2);
          print_next_indent = 0;
          printf("%s ", previous_token);
          empty_string(previous_token, previous_token_allocation);
        } 
        
        if (matched == -1 && reader_len) {
          print_indent((!!print_next_indent) * level * 2);
          print_next_indent = 0;
          printf("%s\n", buffer);
        } else {
          if (previous_token_allocation < reader_len) previous_token_allocation = allocate_str(&previous_token, previous_token_allocation);
          strcpy(previous_token, buffer);
        }
        print_next_indent += print_next_indent == 2 ? 0 : only_whitespace(buffer, reader_len) && matched == 2;
      } break;

      case 3: {
        #ifdef DEBUG
        printf("-- entered unimplemented code: %s\n", buffer);
        #endif

        // todo: search till the end of bracket
        char in_string = 0;
        int escape_string = 0;
        int length = 0;
        char c = 0;
        char c2str[2] = { '\0', '\0' };

        char *attribute_buffer = NULL;
        size_t attribute_allocation = 0;
        attribute_allocation = allocate_str(&attribute_buffer, attribute_allocation);
        empty_string(attribute_buffer, attribute_allocation);

        while((c = reader_char(context_p)) != EOF) {
          c2str[0] = c;
          if ((length + 1.0) / attribute_allocation > 0.75) attribute_allocation = allocate_str(&attribute_buffer, attribute_allocation);
          switch (c) {
            case '"':
            case '\'': {
              if (in_string == c) {
                if (!escape_string) in_string = 0;
              } else {
                in_string = in_string == 0 ? c : in_string;
              }
              strcat(attribute_buffer, c2str);
            } break;
            case '\\': {
              escape_string = !!in_string;
              strcat(attribute_buffer, c2str);
              break;
            }
            case '[': {
              if (in_string) {
                strcat(attribute_buffer, c2str);
                break;
              }
              
              // todo: discard previous string
              // and redo the whole thing
              in_string = 0;
              puts(previous_token);
              puts(c2str);
              // backtrack to the latest non
              if (previous_token_allocation < attribute_allocation) previous_token_allocation = allocate_str(&previous_token, previous_token_allocation);
              empty_string(previous_token, previous_token_allocation);

              size_t backtrack = strlen(attribute_buffer); 
              {
                size_t i = strlen(attribute_buffer);
                do {
                  if (!is_keyword_char(attribute_buffer[i])) break;
                  backtrack = i--;
                  if (i == 0) break;
                } while (1);
              }

              print_indent(level * 2);
              printf("%.*s%c", (int)backtrack, attribute_buffer, backtrack ? '\n' : '\0');
              strcpy(previous_token, attribute_buffer + backtrack);
              empty_string(attribute_buffer, attribute_allocation);
              break;
            }
            case ']':
              if (in_string) {
                strcat(attribute_buffer, c2str);
                break;
              }
              goto finish_attribute;
          }
        }

        finish_attribute:
        if (previous_attribute_allocation < strlen(attribute_buffer) + 1) previous_attribute_allocation = allocate_str(&previous_attribute, previous_attribute_allocation);
        strcpy(previous_attribute, attribute_buffer);
      } break;

      case 4: {
        if (reader_size(context_p)) {
            if (strlen(previous_token)) {
            #ifdef DEBUG
            printf("-- previous token - entering recursive\n");
            #endif

            print_indent((!print_next_indent) * level * 2);
            print_next_indent = 0;
            printf("%s\n", previous_token);
          }
          if (previous_token_allocation < reader_size(context_p)) previous_token_allocation = allocate_str(&previous_token, previous_token_allocation);
          strcpy(previous_token, buffer);
        }

        int recursive = strlen(previous_token);

        if (recursive) {
          int length = 0;
          char c = 0;
          char c2str[2] = { '\0', '\0' };
          int blocks = 0;
          char *recursive_buffer = NULL;
          size_t recursive_allocation = 0;
          recursive_allocation = allocate_str(&recursive_buffer, recursive_allocation);
          empty_string(recursive_buffer, recursive_allocation);
          while((c = reader_char(context_p)) != EOF) {
            c2str[0] = c;
            if ((length + 1.0) / recursive_allocation > 0.75)recursive_allocation = allocate_str(&recursive_buffer, recursive_allocation);
            switch (c) {
              case '{':
                blocks++;
                strcat(recursive_buffer, c2str);
                break;
              case '}':
                if (!blocks) goto startlex;
                blocks--;
                __attribute__ ((fallthrough));
              default:
                strcat(recursive_buffer, c2str);
                break;
            }
          }
          startlex: {
            reader_context next;
            size_t nonnewline = 0;
            for (size_t i = 0; i < strlen(recursive_buffer); i++) {
              if (!is_whitespace(recursive_buffer[i])) break;
              nonnewline++;
            }
            
            reader_buffer_init(&next, recursive_buffer + nonnewline);
            #ifdef DEBUG
            printf("-- init buffer %s\n", recursive_buffer + nonnewline);
            #endif
            free(recursive_buffer);
            recursive_allocation = 0;
            // todo: print opening tag
            #ifdef DEBUG
            printf("-- opening\n");
            #endif
            puts("");
            print_indent(level * 2);
            printf("<%s>\n", previous_token);
            parse(next, level + 1);
            reader_close(&next);
            // todo: print closing tag
            #ifdef DEBUG
            printf("-- closing\n");
            #endif
            puts("");
            print_indent(level * 2);
            printf("</%s>", previous_token);
            empty_string(previous_token, previous_token_allocation);
          }
        } else {
          print_indent(level * 2);
          printf("%s{\n", buffer);
        }
      } break;
      default:
        break;
    }
    free(buffer);
  }
  writer_cleanup:
  free(write_buffer);
  free(previous_token);
  free(previous_attribute);
}

int main(int argc, char **argv) {
  reader_context context;
  if (argc < 2) {
    reader_stdin_init(&context);
    parse(context, 0);
    reader_close(&context);
    return 0;
  }
  int as_files = 0;
  for (int i = 1; i < argc; i++) {
    as_files |= !strcmp(argv[i], "-f") || !strcmp(argv[i], "--files");
  }
  

  for (int i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "-f") || !strcmp(argv[i], "--files")) continue;
    if (as_files) {
      if (reader_file_init(&context, argv[i])) {
        fprintf(stderr, "Cannot read file: %s\n", argv[i]);
        reader_close(&context);
        continue;
      }
    } else {
      reader_buffer_init(&context, argv[i]);
    }
    parse(context, 0);
    reader_close(&context);
  }
  return 0;
}