#include <yaml.h>
#include <axon/config.h>

void axon_createConfig(void) {
  axon_ensureStructure();
  if (axon_configExists()) return;
  FILE *config = fopen("./conf/database.yml", "w+");
  if (config == NULL) {
    /* LCOV_EXCL_START */
    fprintf(stderr, "Error: %s\n", strerror(errno));
    return;
    /* LCOV_EXCL_STOP */
  }
  fprintf(
      config,
      ""
          "dev:\n"
          "  name: \"kore_dev\"\n"
          "  port: 5432\n"
          "  host: \"localhost\"\n"
          "prod:\n"
          "  name: \"kore_prod\"\n"
          "  port: 5432\n"
          "  host: \"localhost\"\n"
          "test:\n"
          "  name: \"kore_test\"\n"
          "  port: 5432\n"
          "  host: \"localhost\"\n"
  );
  fclose(config);
}

char axon_configExists(void) {
  return axon_checkIO("./conf/database.yml");
}

void axon_createOrder(void) {
  axon_ensureStructure();
  if (axon_orderExists()) return;
  if (axon_touch("./db/order.yml") != 1) return;

  FILE *f = fopen("./db/order.yml", "w+");
  if (f == NULL) return;
  fprintf(
      f,
      ""
          "setup:\n"
          "  - first.sql\n"
          "seed:\n"
          "  - first.sql\n"
  );
  fclose(f);
}

char axon_orderExists(void) {
  return axon_checkIO("./db/order.yml");
}

static void axon_fetchConfig(AxonEnvironmentConfig *c, char *name, char *value) {
  if (c == NULL) {
    /* LCOV_EXCL_START */
    if (name) free(name);
    if (value) free(value);
    return;
    /* LCOV_EXCL_STOP */
  }
  if (name == NULL) {
    /* LCOV_EXCL_START */
    if (value) free(value);
    return;
    /* LCOV_EXCL_STOP */
  }
  if (strcmp(name, "name") == 0) {
    c->name = value;
  } else if (strcmp(name, "host") == 0) {
    c->host = value;
  } else if (strcmp(name, "port") == 0) {
    c->port = value ? atoi(value) : 5432;
    free(value);
  } else {
    if (value) free(value); /* LCOV_EXCL_LINE */
  }
  free(name);
}

static void axon_readConfigFile(AxonConfig *config) {
  FILE *f = fopen(AXON_DATABASE_CONFIG_FILE, "rb");
  if (f == NULL) return;
  yaml_parser_t parser;
  yaml_parser_initialize(&parser);
  yaml_parser_set_input_file(&parser, f);
  yaml_token_t token;
  AxonEnvironmentConfig *c = NULL;

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
        break;
      case YAML_VALUE_TOKEN:
        type = token.type;
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
            axon_fetchConfig(c, key, value);
            key = NULL;
            value = NULL;
            break;
          }
          case YAML_KEY_TOKEN: {
            if (key != NULL) axon_fetchConfig(c, key, value); /* LCOV_EXCL_LINE */
            key = calloc(sizeof(char), strlen((char *) token.data.scalar.value) + 1);
            strcpy(key, (char *) token.data.scalar.value);
            break;
          }
          case YAML_BLOCK_MAPPING_START_TOKEN: {
            type = YAML_NO_TOKEN;
            char *env = calloc(sizeof(char), strlen((char *) token.data.scalar.value) + 1);
            strcpy(env, (char *) token.data.scalar.value);
            c = calloc(sizeof(AxonEnvironmentConfig), 1);
            config->len += 1;
            config->configs = config->configs ?
                              realloc(config->configs, sizeof(AxonEnvironmentConfig *) * (config->len + 1)) :
                              calloc(sizeof(AxonEnvironmentConfig *), config->len + 1);
            config->configs[config->len - 1] = c;
            config->configs[config->len] = 0;
            config->environments = config->environments ?
                                   realloc(config->environments, sizeof(char *) * (config->len + 1)) :
                                   calloc(sizeof(char *), config->len + 1);
            config->environments[config->len - 1] = env;
            config->environments[config->len] = 0;
            break;
          }
          default:
            break; /* LCOV_EXCL_LINE */
        }
        break;
      case YAML_STREAM_START_TOKEN:
      case YAML_STREAM_END_TOKEN:
      case YAML_BLOCK_SEQUENCE_START_TOKEN:
      case YAML_BLOCK_ENTRY_TOKEN:
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

AxonConfig *axon_readConfig() {
  AxonConfig *config = calloc(sizeof(AxonConfig), 1);
  config->configs = NULL;
  config->environments = NULL;
  config->len = 0;

  if (axon_configExists() == 0) {
    fprintf(stderr, "No database config found, creating from template...\n");
    axon_createConfig();
  }
  axon_readConfigFile(config);

  return config;
}

AxonEnvironmentConfig *axon_findEnvConfig(AxonConfig *config, const char *env) {
  if (config == NULL) return NULL;
  if (config->environments == NULL) return NULL;
  AxonEnvironmentConfig *c = NULL;
  char **keys = config->environments;
  int offset = 0;

  while (keys && *(keys + offset)) {
    if (strcmp(*(keys + offset), env) == 0)
      return *(config->configs + offset);
    offset += 1;
  }

  return c;
}

void axon_freeConfig(AxonConfig *config) {
  char **keys = config->environments;
  AxonEnvironmentConfig **configs = config->configs;
  while (keys && *keys) {
    free(*keys);
    if ((*configs)->name) free((*configs)->name);
    if ((*configs)->connectionName) free((*configs)->connectionName);
    if ((*configs)->host) free((*configs)->host);
    free(*configs);
    keys += 1;
    configs += 1;
  }
  if (config->configs) free(config->configs);
  if (config->environments) free(config->environments);
  free(config);
}

char *axon_getFlavor(void) {
  char *flavor = NULL;
  if (getenv("KORE_ENV")) {
    const char *env = getenv("KORE_ENV");
    flavor = calloc(sizeof(char), strlen(env) + 1);
    strcpy(flavor, env);
    return flavor;
  }
  // This is kore .flavor file
  FILE *f = fopen(".flavor", "r");
  if (f == NULL) {
    flavor = calloc(sizeof(char), strlen("dev") + 1);
    strcpy(flavor, "dev");
    return flavor;
  }
  while (!feof(f)) {
    char c = (char) fgetc(f);
    switch (c) {
      case ' ':
      case '\r':
      case '\n':
        fclose(f);
        return flavor;
      default: {
        if (isascii(c)) {
          const size_t len = flavor ? strlen(flavor) + 1 : 1;
          flavor = flavor ?
                   realloc(flavor, sizeof(char) * (len + 1)) :
                   calloc(sizeof(char), len + 1);
          flavor[len - 1] = c;
          flavor[len] = 0;
        }
        break;
      }
    }
  }
  setenv("KORE_ENV", flavor, 0);
  fclose(f);
  return flavor;
}

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
axon_readOrder(void) {
  if (!axon_orderExists()) {
    fprintf(stderr, "%s does not exists!\n", AXON_ORDER_CONFIG_FILE);
    return NULL;
  }

  AxonOrder *order = calloc(sizeof(AxonOrder), 1);
  axon_readOrderFile(order);
  return order;
}

void axon_freeOrder(AxonOrder *order) {
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

char axon_isDatabaseInitExists(void) {
  if (!axon_checkIO("src/db/init.h")) return 0;
  if (!axon_checkIO("src/db/init.c")) return 0;
  return 1;
}
