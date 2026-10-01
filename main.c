/***************************************************************************//**

  @file         main.c

  @author       Add connectors by Yanwei Wu; Fill blanks by Gabe

*******************************************************************************/

#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

int size = 64;

/*
  Function Declarations for builtin shell commands:
 */
int lsh_cd(char **args);
int lsh_help(char **args);
int lsh_exit(char **args);
int lsh_about(char **args);

/*
  List of builtin commands, followed by their corresponding functions.
 */
char *builtin_str[] = {
  "cd",
  "help",
  "exit",
  "about"
};

int (*builtin_func[]) (char **) = {
  &lsh_cd,
  &lsh_help,
  &lsh_exit,
  &lsh_about
};

int lsh_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

/*
  Builtin function implementations.
 */
int lsh_cd(char **args)
{
  if (args[1] == NULL) {
    fprintf(stderr, "lsh: expected argument to \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("lsh");
    }
  }
  return 1;
}

int lsh_help(char **args)
{
  int i;
  printf("Stephen Brennan's LSH\n");
  printf("Type program names and arguments, and hit enter.\n");
  printf("The following are built in:\n");

  for (i = 0; i < lsh_num_builtins(); i++) {
    printf("  %s\n", builtin_str[i]);
  }

  printf("Use the man command for information on other programs.\n");
  return 1;
}

int lsh_exit(char **args)
{
  return 0;
}

int lsh_about(char **args)
{
  printf("CS3230 Shell Program\n");
  printf("Programmer: Gabe\n");
  printf("Built-in commands: cd, help, exit, about\n");
  printf("Features: prompt, external commands, built-in command, &, ;, |, input validation\n");
  return 1;
}

/**
  @brief Launch a program and wait for it to terminate.
  @param args Null terminated list of arguments (including program).
  @return Always returns 1, to continue execution.
 */
int lsh_launch(char **args)
{
  pid_t pid;
  int status;

  pid = fork();
  if (pid == 0) {
    if (execvp(args[0], args) == -1) {
      perror("lsh");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    perror("lsh");
  } else {
    do {
      waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}

/* Task 6: | connector */
int myPipe(char **args, int j)
{
  int fd[2];
  pid_t pid1, pid2;
  int status;

  args[j] = NULL;
  char **vector = &args[j + 1];

  if (args[0] == NULL || vector[0] == NULL) {
    fprintf(stderr, "lsh: invalid pipe command\n");
    return 1;
  }

  if (pipe(fd) == -1) {
    perror("lsh");
    return 1;
  }

  pid1 = fork();
  if (pid1 == 0) {
    close(fd[0]);
    dup2(fd[1], STDOUT_FILENO);
    close(fd[1]);

    if (execvp(args[0], args) == -1) {
      perror("lsh");
    }
    exit(EXIT_FAILURE);
  } else if (pid1 < 0) {
    perror("lsh");
    close(fd[0]);
    close(fd[1]);
    return 1;
  }

  pid2 = fork();
  if (pid2 == 0) {
    close(fd[1]);
    dup2(fd[0], STDIN_FILENO);
    close(fd[0]);

    if (execvp(vector[0], vector) == -1) {
      perror("lsh");
    }
    exit(EXIT_FAILURE);
  } else if (pid2 < 0) {
    perror("lsh");
    close(fd[0]);
    close(fd[1]);
    return 1;
  }

  close(fd[0]);
  close(fd[1]);

  waitpid(pid1, &status, WUNTRACED);
  waitpid(pid2, &status, WUNTRACED);

  return 1;
}

/* Task 4: & connector */
int myAmphersand(char **args, int j)
{
  pid_t pid1, pid2;
  int status;

  args[j] = NULL;
  char **vector = &args[j + 1];

  if (args[0] == NULL || vector[0] == NULL) {
    fprintf(stderr, "lsh: invalid & command\n");
    return 1;
  }

  pid1 = fork();
  if (pid1 == 0) {
    if (execvp(args[0], args) == -1) {
      perror("lsh");
    }
    exit(EXIT_FAILURE);
  } else if (pid1 < 0) {
    perror("lsh");
    return 1;
  }

  pid2 = fork();
  if (pid2 == 0) {
    if (execvp(vector[0], vector) == -1) {
      perror("lsh");
    }
    exit(EXIT_FAILURE);
  } else if (pid2 < 0) {
    perror("lsh");
    return 1;
  }

  waitpid(pid1, &status, WUNTRACED);
  waitpid(pid2, &status, WUNTRACED);

  return 1;
}

/* Task 5: ; connector */
int mySemicolon(char **args, int j)
{
  char **vector;

  args[j] = NULL;
  lsh_launch(args);

  vector = &args[j + 1];
  if (vector[0] != NULL) {
    lsh_launch(vector);
  }

  return 1;
}

/**
   @brief Execute shell built-in or launch program.
   @param args Null terminated list of arguments.
   @return 1 if the shell should continue running, 0 if it should terminate
 */
int lsh_execute(char **args)
{
  int i;

  if (args[0] == NULL) {
    return 1;
  }

  for (int j = 0; args[j] != NULL; j++)
  {
    if (strcmp(args[j], "&") == 0) {
      return myAmphersand(args, j);
    }

    if (strcmp(args[j], "|") == 0) {
      return myPipe(args, j);
    }

    if (strcmp(args[j], ";") == 0) {
      return mySemicolon(args, j);
    }
  }

  for (i = 0; i < lsh_num_builtins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }

  return lsh_launch(args);
}

#define LSH_RL_BUFSIZE 1024

char *lsh_read_line(void)
{
  int bufsize = LSH_RL_BUFSIZE;
  int position = 0;
  char *buffer = malloc(sizeof(char) * bufsize);
  int c;

  if (!buffer) {
    fprintf(stderr, "lsh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  while (1) {
    c = getchar();

    if (c == EOF) {
      exit(EXIT_SUCCESS);
    } else if (c == '\n') {
      buffer[position] = '\0';
      return buffer;
    } else {
      buffer[position] = c;
    }
    position++;

    if (position >= bufsize) {
      bufsize += LSH_RL_BUFSIZE;
      buffer = realloc(buffer, bufsize);
      if (!buffer) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
  }
}

char* process_line(char *oline)
{
  int i;
  int len = strlen(oline);

  /* (1) check length: if longer than 100, truncate and print error */
  if (len > 100) {
    fprintf(stderr, "lsh: input too long, truncating to 100 characters\n");
    oline[100] = '\0';
    len = 100;
  }

  /*
     (2) check character set
     allowed:
     - letters and digits
     - dash, dot, slash, underscore
     - spaces/tabs
     - connectors: &, ;, |
  */
  for (i = 0; i < len; i++) {
    char c = oline[i];

    if (isalnum((unsigned char)c) ||
        c == '-' || c == '.' || c == '/' || c == '_' ||
        c == ' ' || c == '\t' ||
        c == '&' || c == ';' || c == '|') {
      continue;
    } else {
      fprintf(stderr, "lsh: invalid character '%c' in input\n", c);
      oline[0] = '\0';   /* ignore the rest of the line */
      return oline;
    }
  }

  return oline;
}

#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"

char **lsh_split_line(char *line)
{
  int bufsize = LSH_TOK_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token, **tokens_backup;

  if (!tokens) {
    fprintf(stderr, "lsh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, LSH_TOK_DELIM);
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= bufsize) {
      bufsize += LSH_TOK_BUFSIZE;
      tokens_backup = tokens;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
        free(tokens_backup);
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, LSH_TOK_DELIM);
  }
  tokens[position] = NULL;
  return tokens;
}

void lsh_loop(void)
{
  char *line, *oline;
  char **args;
  int status;

  do {
    printf("CS3230-Gabe>> ");
    fflush(stdout);

    oline = lsh_read_line();
    line = process_line(oline);
    args = lsh_split_line(line);
    status = lsh_execute(args);

    free(line);
    free(args);
  } while (status);
}

int main(int argc, char **argv)
{
  lsh_loop();
  return EXIT_SUCCESS;
}
