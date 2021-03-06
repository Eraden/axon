#include <axon/db/init.h>

const char *INIT_HEADER_CONTENT = "#pragma once\n\n"
    "#include <kore/kore.h>\n"
    "#include <kore/pgsql.h>\n"
    "#include <stdlib.h>\n"
    "#include <stdio.h>\n"
    "#include <string.h>\n"
    "#include <yaml.h>\n\n"
    "typedef struct sDatabaseConfig {\n"
    "  char *name;\n"
    "  char *host;\n"
    "  int port;\n"
    "} DatabaseConfig;\n\n"
    "int db_init(int);\n";

const char *INIT_SOURCE_CONTENT = "#include \"./init.h\"\n"
    "\n"
    "static DatabaseConfig *getConfig(char *flavor);\n"
    "\n"
    "static void fetchConfig(DatabaseConfig *c, char *name, char *value);\n"
    "\n"
    "static char *getFlavor(void);\n"
    "\n"
    "int db_init(int _) {\n"
    "  char *flavor = getFlavor();\n"
    "  DatabaseConfig *config = getConfig(flavor);\n"
    "  \n"
    "  char buffer[256];\n"
    "  sprintf(buffer, \"dbname=%s\", config->name);\n"
    "  kore_pgsql_register(\"db\", buffer);\n"
    "\n"
    "  free(flavor);\n"
    "  if (config->name) free(config->name);\n"
    "  if (config->host) free(config->host);\n"
    "  free(config);\n"
    "\n"
    "  return KORE_RESULT_OK;\n"
    "}\n"
    "\n"
    "static void fetchConfig(DatabaseConfig *c, char *name, char *value) {\n"
    "  if (c == NULL) {\n"
    "    if (name) free(name);\n"
    "    if (value) free(value);\n"
    "    return;\n"
    "  }\n"
    "  if (name == NULL) {\n"
    "    if (value) free(value);\n"
    "    return;\n"
    "  }\n"
    "  if (strcmp(name, \"name\") == 0) {\n"
    "    c->name = value;\n"
    "  } else if (strcmp(name, \"host\") == 0) {\n"
    "    c->host = value;\n"
    "  } else if (strcmp(name, \"port\") == 0) {\n"
    "    c->port = value ? atoi(value) : 5432;\n"
    "    free(value);\n"
    "  } else {\n"
    "    if (value) free(value);\n"
    "  }\n"
    "  free(name);\n"
    "}\n"
    "\n"
    "static DatabaseConfig *getConfig(char *flavor) {\n"
    "  FILE *f = fopen(\"./conf/database.yml\", \"rb\");\n"
    "  if (f == NULL) return NULL;\n"
    "  DatabaseConfig *config = calloc(sizeof(DatabaseConfig), 1);\n"
    "  int indent = 0;\n"
    "  char *buffer = NULL;\n"
    "  char *name = NULL;\n"
    "  char isCurrent = 0;\n"
    "\n"
    "  while (!feof(f)) {\n"
    "    char c = (char) fgetc(f);\n"
    "    switch (c) {\n"
    "      case '\\n':\n"
    "        if (indent > 0) {\n"
    "          if (isCurrent) fetchConfig(config, name, buffer);\n"
    "          name = NULL;\n"
    "          buffer = NULL;\n"
    "        } else {\n"
    "          isCurrent = name && strcmp(name, flavor) == 0;\n"
    "          free(name);\n"
    "          name = NULL;\n"
    "        }\n"
    "        indent = 0;\n"
    "        break;\n"
    "      case ' ':\n"
    "        if (name != NULL) indent += 1;\n"
    "        break;\n"
    "      case ':':\n"
    "        if (name) free(name);\n"
    "        name = buffer;\n"
    "        buffer = NULL;\n"
    "        break;\n"
    "      case '\"':\n"
    "        break;\n"
    "      default: {\n"
    "        const size_t len = buffer ? strlen(buffer) + 1 : 1;\n"
    "        buffer = buffer ?\n"
    "                 realloc(buffer, sizeof(char) * (len + 1)) :\n"
    "                 calloc(sizeof(char), len + 1);\n"
    "        buffer[len - 1] = c;\n"
    "        buffer[len] = 0;\n"
    "      }\n"
    "    }\n"
    "  }\n"
    "  fclose(f);\n"
    "  return config;\n"
    "}\n"
    "\n"
    "static char *getFlavor(void) {\n"
    "  // This is kore .flavor file\n"
    "  FILE *f = fopen(\".flavor\", \"r\");\n"
    "  char *flavor = NULL;\n"
    "  if (f == NULL) {\n"
    "    flavor = calloc(sizeof(char), strlen(\"dev\") + 1);\n"
    "    strcpy(flavor, \"dev\");\n"
    "    return flavor;\n"
    "  }\n"
    "  while (!feof(f)) {\n"
    "    char c = (char) fgetc(f);\n"
    "    switch (c) {\n"
    "      case ' ':\n"
    "      case '\\r':\n"
    "      case '\\n':\n"
    "        fclose(f);\n"
    "        return flavor;\n"
    "      default: {\n"
    "        const size_t len = flavor ? strlen(flavor) + 1 : 1;\n"
    "        flavor = flavor ?\n"
    "                 realloc(flavor, sizeof(char) * (len + 1)) :\n"
    "                 calloc(sizeof(char), len + 1);\n"
    "        flavor[len - 1] = c;\n"
    "        flavor[len] = 0;\n"
    "        break;\n"
    "      }\n"
    "    }\n"
    "  }\n"
    "  fclose(f);\n"
    "  return flavor;\n"
    "}\n";

int axon_initCommand() {
  int result = axon_ensureStructure();
  if (result != AXON_SUCCESS) {
    AXON_NO_SRC_DIRECTORY_MSG
    return result;
  }

  if (axon_checkIO("./src/db/init.c") == 0) {
    FILE *f = fopen("./src/db/init.c", "w+");
    fprintf(f, "%s", INIT_SOURCE_CONTENT);
    fclose(f);
  }
  if (axon_checkIO("./src/db/init.h") == 0) {
    FILE *f = fopen("./src/db/init.h", "w+");
    fprintf(f, "%s", INIT_HEADER_CONTENT);
    fclose(f);
  }

  printf(
      ""
          "Please modify your conf/project.conf\n"
          "  load    ./project.so db_init\n"
  );

  AxonGraph dbChildren[2] = {{.root="init.c",.len=0}, {.root="init.h",.len=0}};
  AxonGraph srcChildren[1] = {{.root="db", .leafs=dbChildren,.len=2}};
  AxonGraph axonGraph = {.root="src", .len=1, .leafs=srcChildren};
  axon_createInfo(&axonGraph);

  return AXON_SUCCESS;
}
