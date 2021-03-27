#include "reader.h"

typedef enum _assign_as {
  ASSIGN_CHAR,
  ASSIGN_INT
} assign_as;

static int allocate_more(void **pointer, size_t current_allocation, size_t pointer_size, int assign_as) {
  current_allocation = current_allocation == 0 ? 32 : current_allocation;
  void *temp = *pointer;
  *pointer = malloc(pointer_size * current_allocation * 2);
  if (*pointer == NULL) {
    *pointer = temp;
    return 0;
  }

  for (size_t i = 0; temp != NULL && i < current_allocation; i++) {
    switch (assign_as) {
      case ASSIGN_CHAR: {
        char *p = *pointer;
        char *f = temp;
        p[i] = f[i];
      } break;
      default:
        break;
    }
  }
  
  free(temp);
  return current_allocation * 2;
}

int parse(reader_context context, const int level) {
  printf("Entering level %d\n", level);
  int matched;
  int is_parsing_mode = 0;
  char *write_buffer = NULL;
  int write_buffer_allocation = 0;
  write_buffer_allocation = allocate_more((void**)&write_buffer, write_buffer_allocation, sizeof(char), ASSIGN_CHAR);
  reader_context *context_p;
  while((matched = reader_tobuffer(context_p, 5, ' ', '\t', '\n', '{', '[')) != -2) {
    char *buffer = malloc(sizeof(char)*reader_size(context_p) + 1);
    for (size_t i = 0; i <= reader_size(context_p); i++) buffer[i] = '\0';
    reader_cpy(context_p, buffer);

    switch (matched) {
      case -1: {
        printf("%s\n", buffer);
      } break;

      case 0:
      case 1:
      case 2: {
        if (is_parsing_mode) {
          printf("In attribute parsing mode: %s\n", buffer);
        } else printf("%s ", buffer);
      } break;

      case 3: {
        is_parsing_mode = !!reader_size(context_p);
        if (is_parsing_mode) {
          printf("Entering parsing mode:\n");
        }
      } break;

      case 4: {
        int recursive = !!reader_size(context_p);
        if (recursive) {
          char *recursive_buffer = NULL;
          int recursive_allocation = 0;
          int length = 0;
          char c = 0;
          char c2str[2] = { '\0', '\0' };
          int blocks = 0;
          recursive_allocation = allocate_more((void**)&recursive_buffer, recursive_allocation, sizeof(char), ASSIGN_CHAR);
          while((c = reader_char(context_p)) != EOF) {
            c2str[0] = c;
            if ((length + 10) / recursive_allocation > 0.75)recursive_allocation = allocate_more((void**)&recursive_buffer, recursive_allocation, sizeof(char), ASSIGN_CHAR);
            switch (c) {
              case '{':
                blocks++;
                strcat(recursive_buffer, c2str);
                break;
              case '}':
                if (!blocks) goto startlex;
                blocks--;
                strcat(recursive_buffer, c2str);
              default:
                strcat(recursive_buffer, c2str);
                break;
            }
            startlex: {
              reader_context next;
              reader_buffer_init(&next, recursive_buffer);
              free(recursive_buffer);
              recursive_allocation = 0;
              // todo: print opening tag
              parse(next, level + 1);
              // todo: print closing tag
            }
          }
        } else {
          printf("%s{\n", buffer);
        }
      } break;
      default:
        break;
    }
    cleanup:
    free(buffer);
  }
  free(write_buffer);
}

int main() {
  reader_context context;
  reader_init(&context);
  parse(context, 0);
  return 0;
}