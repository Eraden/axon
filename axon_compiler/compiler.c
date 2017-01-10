#include "compiler.h"

static int axon_resolveCompiler(AxonCompileTriggersData *data);

static int axon_collectSourceFiles(AxonCompileTriggersData *data);

static void axon_freeAxonCompileTriggersData(AxonCompileTriggersData *data);

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

  if (d == NULL) {
    return AXON_FAILURE;
  }

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

static const char *ASSEMBLY_TRIGGERS_OBJECTS = " -ldl -shared -o ./triggers.so";

// gcc -fPIC -c source_1.c source_2.c
static int axon_createObjectFiles(AxonCompileTriggersData *data) {
  chdir("./.axon");

  size_t len = strlen(data->compiler) + strlen(CREATE_TRIGGERS_OBJECTS);
  char *command = calloc(sizeof(char), len + 1);
  strcpy(command, data->compiler);
  strcat(command, CREATE_TRIGGERS_OBJECTS);

  char **files = data->files;
  while (files && *files) {
    char *file = *files;
    len = len + strlen(file) + 1;
    char *ptr = realloc(command, len + 1);
    if (ptr == NULL) {
      /* LCOV_EXCL_START */
      free(command);
      return AXON_FAILURE;
      /* LCOV_EXCL_STOP */
    } else {
      command = ptr;
    }
    strcat(command, " ");
    strcat(command, file);
    command[len] = 0;
    files += 1;
  }

  int result = axon_runCommand(command);
  free(command);
  return result;
}

// gcc -shared -o libtest.so source_1.o source_2.o
static int axon_assemblyObjectFiles(AxonCompileTriggersData *data) {
  chdir("./.axon");
  char *pwd = realpath("./", NULL);

  size_t len = strlen(data->compiler) + strlen(ASSEMBLY_TRIGGERS_OBJECTS);
  char *command = calloc(sizeof(char), len + 1);
  strcpy(command, data->compiler);
  strcat(command, ASSEMBLY_TRIGGERS_OBJECTS);
  int result;

  char **read = data->files;

  while (read && *read) {
    size_t pathLen = strlen(*read);
    char *filePath = calloc(sizeof(char), pathLen + 1);
    strcpy(filePath, *read);
    char *basename = strrchr(filePath, '/');
    pathLen = strlen(pwd) + strlen(basename);
    char *objectPath = calloc(sizeof(char), pathLen + 1);
    strcat(objectPath, pwd);
    strcat(objectPath, basename);
    objectPath[pathLen - 1] = 'o';

    len = len + pathLen + 1;
    char *ptr = realloc(command, sizeof(char) * (len + 1));
    if (ptr == NULL) {
      /* LCOV_EXCL_START */
      free(filePath);
      free(objectPath);
      result = AXON_FAILURE;
      /* LCOV_EXCL_STOP */
      goto axon_compiler_assembly_done;
    } else {
      command = ptr;
    }
    strcat(command, " ");
    strcat(command, objectPath);
    command[len] = 0;
    read += 1;
    free(filePath);
    free(objectPath);
  }

  result = axon_runCommand(command);

axon_compiler_assembly_done:
  free(command);
  free(pwd);

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
  int result = axon_ensureStructure();
  if (result != AXON_SUCCESS) return result;

  AxonCompileTriggersData *data = calloc(sizeof(AxonCompileTriggersData), 1);
  data->pwd = calloc(sizeof(char), 1024);
  getcwd(data->pwd, 1024);
  axon_resolveCompiler(data);
  axon_collectSourceFiles(data);
  result = axon_callCompiler(data);
  axon_freeAxonCompileTriggersData(data);
  return result;
}
