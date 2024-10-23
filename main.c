#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GREEN "\033[0;32m"
#define RED "\033[0;31m"
#define BOLD "\033[1m"
#define ITALIC "\033[3m"
#define RESET "\033[0m"

typedef enum JsonType {
  JsonBoolean = 0,
  JsonObject,
  JsonArray,
  JsonNumber,
  JsonString,
  JsonNull,
} JsonType;

typedef struct Token Token;
struct Token {
  char *value;
  JsonType type;
};

char *get_type_name(JsonType type) {
  switch (type) {
  case JsonBoolean:
    return "JsonBoolean";
  case JsonObject:
    return "JsonObject";
  case JsonArray:
    return "JsonArray";
  case JsonNumber:
    return "JsonNumber";
  case JsonString:
    return "JsonString";
  case JsonNull:
    return "JsonNull";
  default:
    return NULL;
  }
}

void tokenize_arrays(size_t *i, char *contents, char *window, Token **tokens,
                     size_t *amount) {
  size_t length = strlen(contents);
  size_t lptr, rptr, len;

  if (contents[*i] == '[') {
    len = 0;
    lptr = *i + 1;
    rptr = lptr;

    memset(window, 0, length);

    while (rptr < length && contents[rptr] != ']') {
      strncat(window, &contents[rptr], 1);
      rptr++;
    }

    len = rptr - lptr;

    tokens[*amount] = calloc(1, sizeof(Token));
    if (tokens[*amount] == NULL) {
      fprintf(stderr, "ERROR: Failed to allocate memory for token\n");
      exit(EXIT_FAILURE);
    }

    tokens[*amount]->value = calloc(len + 3, sizeof(char));
    if (tokens[*amount]->value == NULL) {
      fprintf(stderr, "ERROR: Failed to allocate memory for token value\n");
      exit(EXIT_FAILURE);
    }
    tokens[*amount]->value[0] = '[';
    strcpy(tokens[*amount]->value + 1, window);
    tokens[*amount]->value[len + 1] = ']';
    tokens[*amount]->value[len + 2] = '\0';
    tokens[*amount]->type = JsonArray;

    (*amount)++;
    *i = rptr;
  }
}

void tokenize_integers(size_t *i, char *contents, char *window, Token **tokens,
                       size_t *amount) {
  size_t length = strlen(contents);
  size_t lptr, rptr, len;

  if (isdigit(contents[*i])) {
    len = 0;
    lptr = *i + 1;
    rptr = lptr;

    memset(window, 0, length);

    while (rptr < length && isdigit(contents[rptr])) {
      snprintf(&contents[rptr], sizeof(int)+1, "%d", contents[rptr]);
      strncat(window, &contents[rptr], 1);
      rptr++;
    }

    len = rptr - lptr;

    tokens[*amount] = calloc(1, sizeof(Token));
    if (tokens[*amount] == NULL) {
      fprintf(stderr, "ERROR: Failed to allocate memory for token\n");
      exit(EXIT_FAILURE);
    }

    tokens[*amount]->value = calloc(len + 1, sizeof(char));
    if (tokens[*amount]->value == NULL) {
      fprintf(stderr, "ERROR: Failed to allocate memory for token value\n");
      exit(EXIT_FAILURE);
    }
    strcpy(tokens[*amount]->value + 1, window);
    tokens[*amount]->value[len + 1] = '\0';
    tokens[*amount]->type = JsonNumber;

    (*amount)++;
    *i = rptr;
  }
}

void tokenize_strings(size_t *i, char *contents, char *window, Token **tokens,
                      size_t *amount) {
  size_t length = strlen(contents);
  size_t lptr, rptr, len;

  if (contents[*i] == '"') {
    len = 0;
    lptr = *i + 1;
    rptr = lptr;

    memset(window, 0, length);

    while (rptr < length && contents[rptr] != '"') {
      strncat(window, &contents[rptr], 1);
      rptr++;
    }

    len = rptr - lptr;

    tokens[*amount] = calloc(1, sizeof(Token));
    if (tokens[*amount] == NULL) {
      fprintf(stderr, "ERROR: Failed to allocate memory for token\n");
      exit(EXIT_FAILURE);
    }

    tokens[*amount]->value = calloc(len + 3, sizeof(char));
    if (tokens[*amount]->value == NULL) {
      fprintf(stderr, "ERROR: Failed to allocate memory for token value\n");
      exit(EXIT_FAILURE);
    }
    tokens[*amount]->value[0] = '"';
    strcpy(tokens[*amount]->value + 1, window);
    tokens[*amount]->value[len + 1] = '"';
    tokens[*amount]->value[len + 2] = '\0';
    tokens[*amount]->type = JsonString;

    (*amount)++;
    *i = rptr;
  }
}

void tokenize_objects(size_t *i, char *contents, char *window, Token **tokens,
                      size_t *amount) {
  size_t length = strlen(contents);
  size_t lptr, rptr, len;

  if (contents[*i] == '{') {
    if (*i <= 0)
      return;

    len = 0;
    lptr = *i + 1;
    rptr = lptr;

    memset(window, 0, length);

    while (rptr < length) {
      bool is_obj_bracket = (contents[rptr] == '}' && rptr + 1 < length &&
                             contents[rptr + 1] == ',');
      if (is_obj_bracket)
        break;

      // tokenize_strings(i, contents, window, tokens, amount);
      // tokenize_arrays(i, contents, window, tokens, amount);
      // tokenize_integers(i, contents, window, tokens, amount);

      strncat(window, &contents[rptr], 1);
      rptr++;
    }

    len = rptr - lptr;

    tokens[*amount] = calloc(1, sizeof(Token));
    if (tokens[*amount] == NULL) {
      fprintf(stderr, "ERROR: Failed to allocate memory for token\n");
      exit(EXIT_FAILURE);
    }

    tokens[*amount]->value = calloc(len + 3, sizeof(char));
    if (tokens[*amount]->value == NULL) {
      fprintf(stderr, "ERROR: Failed to allocate memory for token value\n");
      exit(EXIT_FAILURE);
    }
    tokens[*amount]->value[0] = '{';
    strcpy(tokens[*amount]->value + 1, window);
    tokens[*amount]->value[len + 1] = '}';
    tokens[*amount]->value[len + 2] = '\0';
    tokens[*amount]->type = JsonObject;

    (*amount)++;
    *i = rptr;
  }
}

Token **tokenizer(char *contents, size_t *amount) {
  size_t length = strlen(contents);
  assert(length > 0);

  char *window = calloc(length + 1, sizeof(char));
  if (window == NULL) {
    fprintf(stderr, "ERROR: Failed to allocate enough memory\n");
    exit(EXIT_FAILURE);
  }

  Token **tokens = calloc(length, sizeof(Token *));
  if (tokens == NULL) {
    fprintf(stderr, "ERROR: Failed to allocate enough memory\n");
    exit(EXIT_FAILURE);
  }

  *amount = 0;

  for (size_t i = 0; i < length; ++i) {
    tokenize_strings(&i, contents, window, tokens, amount);
    tokenize_integers(&i, contents, window, tokens, amount);
    tokenize_arrays(&i, contents, window, tokens, amount);
    tokenize_objects(&i, contents, window, tokens, amount);
  }

  free(window);

  return tokens;
}

char *read_file(char *file_name) {
  FILE *fp;
  char *buffer;
  size_t items, length;

  fp = fopen(file_name, "r");
  if (fp == NULL) {
    fprintf(stderr, "ERROR: You need to specify a json file\n");
    exit(EXIT_FAILURE);
  }

  // Set the file position indicator for the stream to the end.
  fseek(fp, 0L, SEEK_END);

  // Obtain the current value of the file position indicator.
  length = ftell(fp);
  if (length == 0) {
    fprintf(stderr, "ERROR: ftell() failed: %zu\n", length);
    exit(EXIT_FAILURE);
  }

  // Set the file position indicator to the beginning of the file.
  fseek(fp, 0L, SEEK_SET);

  buffer = malloc(length * sizeof(char) + 1);
  if (buffer == NULL) {
    fprintf(stderr, "ERROR: Failed to allocate enough memory: %s\n", buffer);
    exit(EXIT_FAILURE);
  }

  items = fread(buffer, sizeof(*buffer), length, fp);
  if (items != length) {
    fprintf(stderr, "ERROR: fread() failed: %zu\n", items);
    exit(EXIT_FAILURE);
  }

  fclose(fp);

  buffer[length] = '\0';

  return buffer;
}

void pretty_print(size_t i, char *type, char *value) {
  printf(GREEN "[%2zu]" RESET, i);
  printf(BOLD " %s " RESET, type);
  printf(RED "%s\n" RESET, value);
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "ERROR: You need to specify a json file\n");
    return -1;
  }

  char *contents = read_file(argv[1]);
  size_t amount = 0;
  Token **tokens = tokenizer(contents, &amount);

  for (size_t i = 0; i < amount; ++i) {
    pretty_print(i, get_type_name(tokens[i]->type), tokens[i]->value);
  }

  for (size_t i = 0; i < amount; ++i) {
    free(tokens[i]->value);
    free(tokens[i]);
  }

  free(tokens);
  free(contents);

  return 0;
}
