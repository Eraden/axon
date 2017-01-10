#include "compiler.h"

static int axon_resolveCompiler(AxonCompileTriggersData *data);

static int axon_collectSourceFiles(AxonCompileTriggersData *data);

static void axon_freeAxonCompileTriggersData(AxonCompileTriggersData *data);

static char *axon_appendLibsToCommand(char *command, char *libs) {
  if (libs == NULL) return command;
  const size_t len = strlen(libs);
  size_t tmpLen = 0;
  char *tmp = NULL;
  char prev = 0;
  for (size_t i = 0; i < len; i++) {
    char c = libs[i];
    if ((prev == ' ' || prev == 0) && c != ' ') {
      tmpLen = tmpLen + strlen(" -l") + 1;
      tmp = tmp ?
            realloc(tmp, sizeof(char) * (tmpLen + 1)) :
            calloc(sizeof(char), tmpLen + 1);
      strcat(tmp, " -l");
      tmp[tmpLen - 1] = c;
      tmp[tmpLen] = 0;
    } else {
      tmpLen += 1;
      tmp = tmp ?
            realloc(tmp, sizeof(char) * (tmpLen + 1)) :
            calloc(sizeof(char), tmpLen + 1);
      tmp[tmpLen - 1] = c;
      tmp[tmpLen] = 0;
    }
    prev = c;
  }
  const size_t oldLen = strlen(command);
  const size_t newLen = oldLen + strlen(" ") + strlen(tmp);
  command = realloc(command, sizeof(char) * (newLen + 1));
  char *ptr = command + oldLen;
  for (size_t i = 0; i < newLen - oldLen; i++) ptr[i] = 0;
  strcat(command, " ");
  strcat(command, tmp);
  command[newLen] = 0;
  free(tmp);
  return command;
}

static char *axon_getObjectFilePath(const char *sourceFilePath) {
  char *pwd = realpath("./", NULL);
  char *ptr = realloc(pwd, sizeof(char) * (strlen(pwd) + strlen("/.axon") + 1));
  if (ptr == NULL) {
    /* LCOV_EXCL_START */
    free(pwd);
    return NULL;
    /* LCOV_EXCL_STOP */
  } else {
    pwd = ptr;
    strcat(pwd, "/.axon");
  }
  char *basename = strrchr(sourceFilePath, '/');
  size_t pathLen = strlen(pwd) + strlen(basename);
  char *objectPath = calloc(sizeof(char), pathLen + 1);
  strcat(objectPath, pwd);
  strcat(objectPath, basename);
  objectPath[pathLen - 1] = 'o';
  free(pwd);
  return objectPath;
}

static int axon_resolveCompiler(AxonCompileTriggersData *data) {
  char *compiler = getenv("CC");
  if (compiler == NULL) compiler = "cc";
  char *str = calloc(sizeof(char), strlen(compiler) + 1);
  strcpy(str, compiler);
  data->compiler = str;

  return AXON_SUCCESS;
}

static int axon_collectSourceFiles(AxonCompileTriggersData *data) {
  char *path = realpath("./db/migrate", NULL);
  DIR *d = opendir(path);
  size_t pathLen = strlen(path);

  if (d == NULL) return AXON_FAILURE; /* LCOV_EXCL_LINE */

  struct dirent *p = readdir(d);

  while (p) {
    char *buf = 0;
    size_t len = 0;
    if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, "..")) {
      p = readdir(d);
      continue;
    }

    len = pathLen + strlen(p->d_name) + 2;
    buf = malloc(len);

    if (buf == NULL) {
      /* LCOV_EXCL_START */
      fprintf(stderr, "%s\n", strerror(errno));
      return AXON_FAILURE;
      /* LCOV_EXCL_STOP */
    }

    struct stat statBuf;
    snprintf(buf, len, "%s/%s", path, p->d_name);

    char isFile = !stat(buf, &statBuf) && S_ISREG(statBuf.st_mode);
    if (isFile && strstr(p->d_name, ".c") != NULL) {
      data->len += 1;
      data->files = data->files ?
                    realloc(data->files, sizeof(char *) * (data->len + 1)) :
                    calloc(sizeof(char *), data->len + 1);
      data->files[data->len - 1] = buf;
      data->files[data->len] = 0;
      p = readdir(d);
    } else {
      /* LCOV_EXCL_START */
      free(buf);
      p = NULL;
      /* LCOV_EXCL_STOP */
    }
  }

  free(path);
  closedir(d);
  return AXON_SUCCESS;
}

static void axon_freeAxonCompileTriggersData(AxonCompileTriggersData *data) {
  char **files = data->files;
  while (files && *files) {
    free(*files);
    files += 1;
  }
  if (data->compiler) free(data->compiler);
  if (data->files) free(data->files);
  if (data->pwd) free(data->pwd);
  free(data);
}

static const char *CREATE_TRIGGERS_OBJECTS = " -std=c11 -Wall -ldl -fPIC -c ";

static const char *ASSEMBLY_TRIGGERS_OBJECTS = " -ldl -shared -o ./.axon/triggers.so ";

// gcc -fPIC -c ./db/migrate/source_1.c -o ./.axon/source_1.o
static int axon_createObjectFiles(AxonCompileTriggersData *data) {
  int result = AXON_SUCCESS;

  char **files = data->files;
  while (files && *files) {
    char *file = *files;
    char *object = axon_getObjectFilePath(file);
    size_t len = strlen(data->compiler);
    len += 1;
    len += strlen(CREATE_TRIGGERS_OBJECTS);
    len += strlen(file);
    len += strlen(" -o ");
    len += strlen(object) + 1;
    if (data->config->flags) len = len + strlen(" ") + strlen(data->config->flags);

    char *command = calloc(sizeof(char), len + 1);
    strcpy(command, data->compiler);
    strcat(command, " ");
    strcat(command, CREATE_TRIGGERS_OBJECTS);
    strcat(command, " ");
    strcat(command, file);
    strcat(command, " -o ");
    strcat(command, object);
    if (data->config->flags) {
      strcat(command, " ");
      strcat(command, data->config->flags);
    }
    command = axon_appendLibsToCommand(command, data->config->libs);
    files += 1;

    result = axon_runCommand(command);
    free(object);
    free(command);
    if (result != AXON_SUCCESS) break;
  }

  return result;
}

// gcc -shared -o libtest.so source_1.o source_2.o
static int axon_assemblyObjectFiles(AxonCompileTriggersData *data) {
  size_t len = strlen(data->compiler) + strlen(ASSEMBLY_TRIGGERS_OBJECTS);
  if (data->config->flags) len = len + strlen(" ") + strlen(data->config->flags);
  char *command = calloc(sizeof(char), len + 1);
  strcpy(command, data->compiler);
  strcat(command, ASSEMBLY_TRIGGERS_OBJECTS);
  if (data->config->flags) {
    strcat(command, " ");
    strcat(command, data->config->flags);
  }
  command = axon_appendLibsToCommand(command, data->config->libs);
  len = strlen(command);
  int result = AXON_SUCCESS;

  char **read = data->files;

  while (read && *read) {
    char *objectPath = axon_getObjectFilePath(*read);

    len = len + strlen(objectPath) + 1;
    char *ptr = realloc(command, sizeof(char) * (len + 1));
    if (ptr == NULL) {
      /* LCOV_EXCL_START */
      free(objectPath);
      result = AXON_FAILURE;
      break;
      /* LCOV_EXCL_STOP */
    } else {
      command = ptr;
    }
    strcat(command, " ");
    strcat(command, objectPath);
    command[len] = 0;
    read += 1;
    free(objectPath);
  }

  if (result == AXON_SUCCESS) result = axon_runCommand(command);
  free(command);

  return result;
}

static int axon_callCompiler(AxonCompileTriggersData *data) {
  int result;
  chdir(data->pwd);
  result = axon_createObjectFiles(data);
  chdir(data->pwd);
  if (result != AXON_SUCCESS) return result;
  chdir(data->pwd);
  result = axon_assemblyObjectFiles(data);
  chdir(data->pwd);
  return result;
}

int axon_runCompiler(int argc, char **argv) {
  if (argc < 2) return AXON_FAILURE;

  if (strcmp(argv[1], "triggers") == 0) {
    return axon_buildTriggers();
  } else {
    return AXON_UNKNOWN_COMMAND;
  }
}

int axon_buildTriggers(void) {
  int result = axon_createConfig();
  if (result != AXON_SUCCESS) return result;

  AxonCompileTriggersData *data = calloc(sizeof(AxonCompileTriggersData), 1);
  data->pwd = calloc(sizeof(char), 1024);
  getcwd(data->pwd, 1024);
  data->config = axon_readTriggersConfig();
  result = axon_resolveCompiler(data);
  if (result == AXON_SUCCESS) result = axon_collectSourceFiles(data);
  if (result == AXON_SUCCESS) result = axon_callCompiler(data);
  axon_freeTriggersConfig(data->config);
  axon_freeAxonCompileTriggersData(data);
  return result;
}
