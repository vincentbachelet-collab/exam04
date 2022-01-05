#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#define STDIN   0
#define STDOUT  1
#define STDERR  2

#define END     3
#define BREAK   4
#define PIPE    5

typedef struct s_base
{
    char            **av;
    int             size;
    int             type;
    int             fd[2];
    struct s_base   *prev;
    struct s_base   *next;
} t_base;

int ft_strlen(char *str)
{
    int i = 0;

    while (str[i])
        i++;
    return (i);
}

char    *ft_strdup(char *str)
{
    int     size;
    char    *res;

    if (!str)
        return (0);
    size = ft_strlen(str);
    res = malloc(sizeof(char) * (size + 1));
    if (!res)
        return (0);
    res[size] = '\0';
    while (--size >= 0)
        res[size] = str[size];
    return (res);
}

void    ft_free_all(t_base *ptr)
{
    t_base  *tmp;
    int     i;

    while (ptr)
    {
        tmp = ptr->next;
        i = 0;
        while (i < ptr->size)
        {
            free(ptr->av[i]);
            i++;
        }
        free(ptr->av);
        free(ptr);
        ptr = tmp;
    }
    ptr = NULL;
}

void    ft_lstadd_back(t_base **ptr, t_base *new)
{
    t_base *tmp;

    if (!(*ptr))
        *ptr = new;
    else
    {
        tmp = *ptr;
        while (tmp->next)
            tmp = tmp->next;
        tmp->next = new;
        new->prev = tmp;
    }
}

void    exit_fatal(void)
{
    write(STDERR, "error: fatal\n", 13);
    exit (EXIT_FAILURE);
}

void    exit_execve(char *str)
{
    write(STDERR, "error: cannot execute ", 22);
    write(STDERR, str, ft_strlen(str));
    write(STDERR, "\n", 1);
    exit (EXIT_FAILURE);
}

int exit_cd1(void)
{
    write(STDERR, "error: cd: bad arguments\n", 25);
    return (EXIT_FAILURE);
}

int exit_cd2(char *str)
{
    write(STDERR, "error: cd: cannot change directory to ", 39);
    write(STDERR, str, ft_strlen(str));
    write(STDERR, "\n", 1);
    return (EXIT_FAILURE);
}

int ft_get_size(char **av)
{
    int i = 0;

    while (av[i] && strcmp(av[i], "|") != 0 && strcmp(av[i], ";") != 0)
        i++;
    return (i);
}

int ft_get_type(char *av)
{
    if (!av)
        return (END);
    if (strcmp(av, "|") == 0)
        return (PIPE);
    if (strcmp(av, ";") == 0)
        return (BREAK);
    return (0);
}

int ft_parse_arg(t_base **ptr, char **av)
{
    t_base *new;
    int     size;

    size = ft_get_size(av);
    new = malloc(sizeof(t_base));
    if (!new)
        exit_fatal();
    new->av = malloc(sizeof(char *) * (size + 1));
    if (!new->av)
        exit_fatal();
    new->size = size;
    new->type = ft_get_type(av[size]);
    new->prev = NULL;
    new->next = NULL;
    new->av[size] = NULL;
    while (--size >= 0)
        new->av[size] = ft_strdup(av[size]);
    ft_lstadd_back(ptr, new);
    return (new->size);
}

void    exec_cmd(t_base *tmp, char **env)
{
    pid_t   pid;
    int     status;
    int     pipe_open;

    pipe_open = 0;
    if (tmp->type == PIPE || (tmp->prev && tmp->prev->type == PIPE))
    {
        pipe_open = 1;
        if (pipe(tmp->fd))
            exit_fatal();
    }
    pid = fork();
    if (pid < 0)
        exit_fatal();
    else if (pid == 0)
    {
        if (tmp->type == PIPE && dup2(tmp->fd[STDOUT], STDOUT) < 0)
            exit_fatal();
        if (tmp->prev && tmp->prev->type == PIPE && dup2(tmp->prev->fd[STDIN], STDIN) < 0)
            exit_fatal();
        if (execve(tmp->av[0], tmp->av, env))
            exit_execve(tmp->av[1]);
        exit(EXIT_SUCCESS);
    }
    else
    {
        waitpid(pid, &status, 0);
        if (pipe_open)
        {
            close(tmp->fd[STDOUT]);
            if (!tmp->next || tmp->type == BREAK)
                close(tmp->fd[STDIN]);
        }
        if (tmp->prev && tmp->prev->type == PIPE)
            close(tmp->prev->fd[STDIN]);
    }
}

void    exec_cmds(t_base *ptr, char **env)
{
    t_base  *tmp;

    tmp = ptr;
    while (tmp)
    {
        if (strcmp(tmp->av[0], "cd") == 0)
        {
            if (tmp->size < 2)
                exit_cd1();
            else if (chdir(tmp->av[1]))
                exit_cd2(tmp->av[1]);
        }
        else
            exec_cmd(tmp, env);
        tmp = tmp->next;
    }
}

int main(int ac, char **av, char **env)
{
    t_base  *ptr;
    int     i;

    ptr = NULL;
    if (ac > 1)
    {
        i = 1;
        while (av[i])
        {
            if (strcmp(av[i], ";") == 0)
            {
                i++;
                continue ;
            }
            i += ft_parse_arg(&ptr, &av[i]);
            if (!av[i])
                break;
            else
                i++;
        }
        if (ptr)
            exec_cmds(ptr, env);
        ft_free_all(ptr);
    }
    return (0);
}