#include <axon/config.h>

static void
axon_fetchOrder(AxonOrder *axonOrder, AxonOrderTarget target, char *name, char *value) {
  if (value == NULL) return;
  if (name != NULL) return;
  char *ptr = calloc(sizeof(char), strlen(value) + 1);
  strcpy(ptr, value);
  switch (target) {
    case AXON_ORDER_TARGET_NONE:
      /* LCOV_EXCL_START */
      free(ptr);
      break;
      /* LCOV_EXCL_STOP  */
    case AXON_ORDER_TARGET_SEED: {
      axonOrder->seedLen += 1;
      axonOrder->seedFiles = axonOrder->seedFiles ?
                             realloc(axonOrder->seedFiles, sizeof(char **) * (axonOrder->seedLen + 1)) :
                             calloc(sizeof(char **), axonOrder->seedLen + 1);
      axonOrder->seedFiles[axonOrder->seedLen - 1] = ptr;
      axonOrder->seedFiles[axonOrder->seedLen] = 0;
      break;
    }
    case AXON_ORDER_TARGET_SETUP: {
      axonOrder->setupLen += 1;
      axonOrder->setupFiles = axonOrder->setupFiles ?
                              realloc(axonOrder->setupFiles, sizeof(char **) * (axonOrder->setupLen + 1)) :
                              calloc(sizeof(char **), axonOrder->setupLen + 1);
      axonOrder->setupFiles[axonOrder->setupLen - 1] = ptr;
      axonOrder->setupFiles[axonOrder->setupLen] = 0;
      break;
    }
  }
}

static void
axon_readOrderFile(AxonOrder *axonOrder) {
  AxonOrderTarget target = AXON_ORDER_TARGET_NONE;
  FILE *f = fopen(AXON_ORDER_CONFIG_FILE, "rb");
  if (f == NULL) return;
  yaml_parser_t parser;
  yaml_parser_initialize(&parser);
  yaml_parser_set_input_file(&parser, f);
  yaml_token_t token;

  char *key = NULL;
  char *value = NULL;
  enum yaml_token_type_e type = YAML_NO_TOKEN;
  unsigned breakCounter = 0; /* LCOV_EXCL_LINE */

  do {
    if (breakCounter++ > 200) break; /* LCOV_EXCL_LINE */
    yaml_parser_scan(&parser, &token);

    switch (token.type) {
      case YAML_KEY_TOKEN:
        if (token.start_mark.column != 0)
          type = token.type;
        else
          type = YAML_BLOCK_MAPPING_START_TOKEN;
        break;
      case YAML_VALUE_TOKEN:
        type = token.type;
        break;
      case YAML_BLOCK_ENTRY_TOKEN:
        if (token.start_mark.column != 0)
          type = YAML_VALUE_TOKEN;
        break;
      case YAML_BLOCK_MAPPING_START_TOKEN:
      case YAML_BLOCK_END_TOKEN:
        type = YAML_BLOCK_MAPPING_START_TOKEN;
        break;
      case YAML_SCALAR_TOKEN:
        switch (type) {
          case YAML_VALUE_TOKEN: {
            value = calloc(sizeof(char), strlen((char *) token.data.scalar.value) + 1);
            strcpy(value, (char *) token.data.scalar.value);
            type = YAML_NO_TOKEN;
            axon_fetchOrder(axonOrder, target, key, value);
            if (key) free(key);
            free(value);
            key = NULL;
            value = NULL;
            break;
          }
          case YAML_KEY_TOKEN: {
            /* LCOV_EXCL_START */
            if (key != NULL) {
              axon_fetchOrder(axonOrder, target, key, value);
              free(key);
              value = NULL;
            }
            /* LCOV_EXCL_STOP */
            key = calloc(sizeof(char), strlen((char *) token.data.scalar.value) + 1);
            strcpy(key, (char *) token.data.scalar.value);
            break;
          }
          case YAML_BLOCK_MAPPING_START_TOKEN: {
            type = YAML_NO_TOKEN;
            if (strcmp((char *) token.data.scalar.value, "seed") == 0) {
              target = AXON_ORDER_TARGET_SEED;
            } else if (strcmp((char *) token.data.scalar.value, "setup") == 0) {
              target = AXON_ORDER_TARGET_SETUP;
            } else {
              target = AXON_ORDER_TARGET_NONE;
            }
            break;
          }
          default:
            break; /* LCOV_EXCL_LINE */
        }
        break;
      case YAML_BLOCK_SEQUENCE_START_TOKEN:
      case YAML_NO_TOKEN:
      case YAML_STREAM_START_TOKEN:
      case YAML_STREAM_END_TOKEN:
      default:
        break; /* LCOV_EXCL_LINE */
    }
    if (token.type == YAML_NO_TOKEN) {
      if (key) free(key);
      yaml_token_delete(&token);
      break;
    }
    if (token.type != YAML_STREAM_END_TOKEN)
      yaml_token_delete(&token);
  } while (token.type != YAML_STREAM_END_TOKEN);

  yaml_token_delete(&token);
  yaml_parser_delete(&parser);
  fclose(f);
}

AxonOrder __attribute__((__malloc__)) *
axon_readOrderConfig(void) {
  axon_createConfig();

  AxonOrder *order = calloc(sizeof(AxonOrder), 1);
  axon_readOrderFile(order);
  return order;
}

void axon_freeOrderConfig(AxonOrder *order) {
  char **files = NULL;

  files = order->seedFiles;
  while (files && *files) {
    free(*files);
    files += 1;
  }
  if (order->seedFiles) free(order->seedFiles);

  files = order->setupFiles;
  while (files && *files) {
    free(*files);
    files += 1;
  }
  if (order->setupFiles) free(order->setupFiles);

  free(order);
}

