#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "tokenizer.h"

/* Convenience macro to silence compiler warnings about unused function parameters. */
#define unused __attribute__((unused))

/* Whether the shell is connected to an actual terminal or not. */
bool shell_is_interactive;

/* File descriptor for the shell input */
int shell_terminal;

/* Terminal mode settings for the shell */
struct termios shell_tmodes;

/* Process group id for the shell */
pid_t shell_pgid;

int cmd_help(struct tokens *tokens);
int cmd_exit(struct tokens *tokens);
int cmd_pwd(struct tokens *tokens);
int cmd_cd(struct tokens *tokens);

/* Built-in command functions take token array (see parse.h) and return int */
typedef int cmd_fun_t(struct tokens *tokens);

/* Built-in command struct and lookup table */
typedef struct fun_desc {
  cmd_fun_t *fun;
  char *cmd;
  char *doc;
} fun_desc_t;

fun_desc_t cmd_table[] = {
  {cmd_help, "?", "show this help menu"},
  {cmd_exit, "exit", "exit the command shell"},
  {cmd_pwd, "pwd", "print current dir"},
  {cmd_cd, "cd", "change current dir"},
};

/* Prints a helpful description for the given command */
int cmd_help(unused struct tokens *tokens) {
  for (unsigned int i = 0; i < sizeof(cmd_table) / sizeof(fun_desc_t); i++)
    printf("%s - %s\n", cmd_table[i].cmd, cmd_table[i].doc);
  return 1;
}

/* Exits this shell */
int cmd_exit(unused struct tokens *tokens) {
  exit(0);
}

int cmd_pwd(struct tokens *tokens)
{
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL)
    {
        fprintf(stdout, "Current working dir: %s\n", cwd);
    }
    else
    {
        perror("getcwd() error");
    }
    return 0;
}

int cmd_cd(struct tokens *ts)
{
    if ( ts->tokens_length > 2 )
    {
        printf("too many arguments");
    }
    else if ( ts->tokens_length == 2 ) 
    {
        if ( chdir(ts->tokens[1]) ) 
        {
            printf("%s no such dir:", ts->tokens[1]);
        }
    }
    else if ( ts->tokens_length == 1 )
    {
        const char* home = getenv("HOME"); 
        chdir(home); 
    }
    else{}
    return 0;
}

void redirect(const char* sign, const char* file)
{
    if ( 0 == strcmp(sign, ">") )
    {
        int fd = open(file, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
        if ( -1 != fd )
        {
            dup2(fd, 1);
        }
        else
        {
            printf("failed to open file, rc: %d\n", errno);
        }
    }
    else if ( 0 == strcmp(sign, "<") )
    {
        printf("input %s\n", file); 
    }
    else{}
}

int exe_sys(struct tokens* tokens, const struct tokens* pathes)
{
    if ( 0 == tokens->tokens_length )
    {
        return 0;
    }
    else
    {
        char** argv = (char**)malloc(tokens->tokens_length * sizeof(char*) + 1);
        bool is_redirect = false;
        size_t len = tokens->tokens_length;
        if ( len > 2 && (0 == strcmp("<", tokens->tokens[len-2]) || 0 == strcmp(">", tokens->tokens[len-2])) )
        {
            is_redirect = true;
        }
        for ( size_t i = 0; i < len; ++i )
        {
            if ( is_redirect && i == len - 2 )
            {
                break;
            }
            argv[i] = tokens->tokens[i];
        }
        argv[tokens->tokens_length] = NULL;
        resolve_path(tokens, pathes);
        pid_t pid = fork();
        int exit = 0;
        if ( 0 == pid )
        {
            if ( is_redirect )
            {
                size_t len = tokens->tokens_length;
                redirect(tokens->tokens[len-2], tokens->tokens[len-1]); 
            }
            if ( execv(tokens->tokens[0], argv) )
            {
                printf("Unknown command: %s\n", tokens->tokens[0]);
                print_tokens(tokens);
            }
        }
        else
        {
            wait(&exit);
            free(argv);
        }
        return 0;
    }
}

void resolve_path(struct tokens* tokens, const struct tokens* pathes)
{
    struct dirent*  de;
    char*           path = NULL;
    for ( size_t i = 0; i < pathes->tokens_length; ++i )
    {   
        DIR* dr = opendir(pathes->tokens[i]);
        if ( NULL == dr )
        {
            continue;
        }
        while ( NULL != (de = readdir(dr)) )
        {
            if ( 0 == strcmp(tokens->tokens[0], de->d_name) )
            {
                path = pathes->tokens[i];
                break;
            }
        }
        if ( NULL != path )
        {
            char* fullPath = (char*)malloc(strlen(tokens->tokens[0]) + strlen(path) + 2);
            strcpy(fullPath, path);
            strcat(fullPath, "/");
            strcat(fullPath, tokens->tokens[0]);
            free(tokens->tokens[0]);
            tokens->tokens[0] = fullPath;
            break;
        }
    }
}

/* Looks up the built-in command, if it exists. */
int lookup(char cmd[]) {
  for (unsigned int i = 0; i < sizeof(cmd_table) / sizeof(fun_desc_t); i++)
    if (cmd && (strcmp(cmd_table[i].cmd, cmd) == 0))
      return i;
  return -1;
}

/* Intialization procedures for this shell */
void init_shell() {
  /* Our shell is connected to standard input. */
  shell_terminal = STDIN_FILENO;

  /* Check if we are running interactively */
  shell_is_interactive = isatty(shell_terminal);

  if (shell_is_interactive) {
    /* If the shell is not currently in the foreground, we must pause the shell until it becomes a
     * foreground process. We use SIGTTIN to pause the shell. When the shell gets moved to the
     * foreground, we'll receive a SIGCONT. */
    while (tcgetpgrp(shell_terminal) != (shell_pgid = getpgrp()))
      kill(-shell_pgid, SIGTTIN);

    /* Saves the shell's process id */
    shell_pgid = getpid();

    /* Take control of the terminal */
    tcsetpgrp(shell_terminal, shell_pgid);

    /* Save the current termios to a variable, so it can be restored later. */
    tcgetattr(shell_terminal, &shell_tmodes);
  }
}

int main(unused int argc, unused char *argv[]) {
  init_shell();

  static char line[4096];
  int line_num = 0;

  const struct tokens *pathes = tokenize_path();

  /* Please only print shell prompts when standard input is not a tty */
  if (shell_is_interactive)
    fprintf(stdout, "%d: ", line_num);

  while (fgets(line, 4096, stdin)) {
    /* Split our line into words. */
    struct tokens *tokens = tokenize(line);

    /* Find which built-in function to run. */
    int fundex = lookup(tokens_get_token(tokens, 0));

    if (fundex >= 0) {
      cmd_table[fundex].fun(tokens);
    } else {
        if ( exe_sys(tokens, pathes) )
        {
            fprintf(stdout, "This shell doesn't know how to run programs.\n");
        }
    }

    if (shell_is_interactive)
      /* Please only print shell prompts when standard input is not a tty */
      fprintf(stdout, "%d: ", ++line_num);

    /* Clean up memory */
    tokens_destroy(tokens);
  }

  return 0;
}
