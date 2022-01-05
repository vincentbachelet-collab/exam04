/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vbachele <vbachele@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/01/05 21:37:38 by vbachele          #+#    #+#             */
/*   Updated: 2022/01/05 22:44:21 by vbachele         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>

#define STDIN 0
#define STDOUT 1
#define STDERR 2

#define PIPE 3
#define BREAK 4
#define END 5

typedef struct s_root
{
	int fd[2];
	int size;
	int type;
	char **arg;
	struct s_root *previous;
	struct s_root *next;
}t_root;

int ft_strlen(char *str)
{
	int i;

	i = 0;
	while (str[i])
		i++;
	return (i);
}

int exit_fatal(void)
{
	write(STDERR, "error: fatal\n", 13);
	exit(EXIT_FAILURE);
}

void exit_cd1(void)
{
	write(STDERR, "error: cd: bad arguments\n", 25);
}

void exit_cd2(char *str)
{
	int i;
	
	i = ft_strlen("error: cd: cannot change directory to ");
	write(STDERR, "error: cd: cannot change directory to ", i);
	write(STDERR, str, ft_strlen(str));
	write(STDERR, "\n", 1);
}

int exit_execve(char *str)
{
	int i;
	
	i = ft_strlen("error: cannot execute ");
	write(STDERR, "error: cannot execute ", i);
	write(STDERR, str, ft_strlen(str));
	write(STDERR, "\n", 1);
	exit(EXIT_FAILURE);
}

void	ft_free_all(t_root *root)
{
	t_root *tmp;
	int		i;
	
	i = 0;
	while (root)
	{
		tmp = root->next;
		while (i < root->size)
		{
			free(root->arg[i]);
			i++;
		}
		free(root->arg);
		free(root);
		root = tmp;
	}
	root = NULL;
}

char *ft_strdup(char *str)
{
	int size;
	char *res;
	
	if (!str)
		return (0);
	res = NULL;
	size = ft_strlen(str);
	res = malloc(sizeof(char) * (size + 1));
	if (!res)
		exit_fatal();
	res[size] = 0;
	while (--size >= 0)
		res[size] = str[size];
	return (res);
}

void	ft_lstaddback(t_root **root, t_root *new)
{
	t_root *tmp;

	if (!(*root))
		*root = new;
	else
	{
		tmp = *root;
		while (tmp && tmp->next)
			tmp = tmp->next;
		tmp->next = new;
		new->previous = tmp;
	}
}

int get_type(char *str)
{
	if (!str)
		return (END);
	if (strcmp(str, "|") == 0)
		return (PIPE);
	if (strcmp(str, ";") == 0)
		return (BREAK);
	return (0);
}

int	get_size(char **argv)
{
	int i;
	
	i = 0;
	while (argv[i] && strcmp(argv[i], "|") != 0 && strcmp(argv[i], ";") != 0)
		i++;
	return (i);
}

int ft_get_parse(t_root **root, char **argv)
{
	t_root *new;
	int size;
	
	size = get_size(argv);
	new = malloc(sizeof(t_root));
	if (!new)
		exit_fatal();
	new->arg = malloc(sizeof(char *) * (size + 1));
	if (!new->arg)
		exit_fatal();
	new->size = size;
	new->type = get_type(argv[size]);
	new->arg[size] = 0;
	new->previous = NULL;
	new->next = NULL;
	while (--size >= 0)
		new->arg[size] = ft_strdup(argv[size]);
	ft_lstaddback(root, new);
	return (new->size);
}

void	exec_cmd(t_root *tmp, char **env)
{
	pid_t pid;
	int status;
	int pipe_open;

	pipe_open = 0;
	if (tmp->type == PIPE || (tmp->previous && tmp->previous->type == PIPE))
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
		if (tmp->previous && tmp->previous->type == PIPE && dup2(tmp->previous->fd[STDIN], STDIN) < 0)
			exit_fatal();
		if (execve(tmp->arg[0], tmp->arg, env))
			exit_execve(tmp->arg[0]);
		exit(EXIT_SUCCESS);
	}
	else
	{
		waitpid(pid, &status, 0);
		if (pipe_open)
		{
			close(tmp->fd[STDOUT]);
			if (!tmp->next || tmp->type	== BREAK)
				close(tmp->fd[STDIN]);
		}
		else 
		{
			if (tmp->previous && tmp->previous->type == PIPE)
				close(tmp->previous->fd[STDIN]);
		}
	}
}

void	exec_cmds(t_root *root, char **env)
{
	t_root *tmp;

	tmp = root;
	while (tmp)
	{
		if (strcmp(tmp->arg[0], "cd") == 0)
		{
			if (tmp->size < 2)
				exit_cd1();
			else if(chdir(tmp->arg[1]))
				exit_cd2(tmp->arg[1]);			
		}
		else 
			exec_cmd(tmp, env);
		tmp = tmp->next;
	}
}

int main(int argc, char **argv, char **env)
{
	t_root *root;
	int		i;
	
	root = NULL;
	if (argc > 1)
	{
		i = 1;
		while (argv[i])
		{
			if (strcmp(argv[i], ";") == 0)
			{
				i++;
				continue;
			}
			i += ft_get_parse(&root, &argv[i]);
			if (!argv[i])
				break;
			else
				i++;
		}
		if (root)
			exec_cmds(root, env);
		// ft_free_all(root);
	}
	return (0);
}
