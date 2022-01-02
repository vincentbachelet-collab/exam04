/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   vincent_exam04.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vbachele <vbachele@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/01/02 14:05:47 by vbachele          #+#    #+#             */
/*   Updated: 2022/01/02 16:30:12 by vbachele         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define	STDIN 0
#define	STDOUT 1
#define STDERR 2

#define END 3
#define BREAK 4
#define PIPE 5

typedef struct s_root
{
	char			**arg;
	int				size;
	int				fd[2];
	int				type;	
	struct s_root	*next;
	struct s_root	*previous;
} t_root;

int	ft_strlen(char *str)
{
	int	i;

	i = 0;
	while (str[i])
		i++;
	return (i);
}

int	exit_fatal(void)
{
	write(STDERR, "error: fatal\n", 13);
    exit (EXIT_FAILURE);
}

void	print_liste(t_root *root)
{
	t_root *tmp;
	int 	i;

	i = 0;
	tmp = root;
	while(--tmp->size >= 0)
	{
		printf("%s\n", tmp->arg[i]);
		i++;
	}
}

void	ft_lstadd_back(t_root **root, t_root *new)
{
	t_root	*tmp;
	
	if (!(*root)) //si la valeur de ma structure est null (donc premier noeud) je donne la valeur new;
		*root = new;
	else
	{
		tmp = *root;
		while (tmp->next)
			tmp = tmp->next;
		tmp->next = new;
		new->previous = tmp;
	}
}

char	*ft_strdup(char *str)
{
	char	*res;
	int		size;
	
	if (str == 0)
		return (0);
	size = ft_strlen(str);
	res = malloc(sizeof(char) * (size + 1));
	if (!str)
		return (0);
	res[size] = 0;
	while (--size >= 0)
		res[size] = str[size];
	return (res);
}
int	ft_get_size(char **argv) // Dans cette fonction je check le nombre d'arguments avant un pipe ou un ";"
{
	int	i;

	i = 0;
	while (argv[i] && (strcmp(argv[i], "|") != 0) && strcmp(argv[i], ";") != 0)
		i++;
	return (i);
}

int	ft_get_type(char *argv)
{
	if (argv == 0)
		return (END);
	if (strcmp(argv, "|"))
		return (PIPE);
	if (strcmp(argv, ";"))
		return (BREAK);
	return (0);
}

int ft_parse_arg(t_root ** root, char **argv)
{
	t_root	*new;
	int		size;
	
	size = ft_get_size(argv);
	new = malloc(sizeof(t_root));
	if (!new)
		exit_fatal();
	new->arg = malloc(sizeof(char *) * (size + 1));
	if (new->arg == 0)
		exit_fatal();
	new->size = size;
	new->type = ft_get_type(argv[size]);
	new->next = NULL;
	new->previous = NULL;
	new->arg[size] = NULL;
	while (--size >= 0)
		new->arg[size] = ft_strdup(argv[size]);
	ft_lstadd_back(root, new);
	return (new->size);
}

int main(int argc, char **argv, char **env)
{
	t_root	*root;
	int		i; // va servir a se balader dans les arguments rentres

	root = NULL;

	if (argc > 1)
	{
		i = 1;
		while (argv[i])
		{
			if (strcmp(argv[i], ";") == 0) // si on croise un ";" on redemarre la boucle while
			{
				i++;
				continue ;
			}
			i += ft_parse_arg(&root, &argv[i]);
			if (argv[i] == 0)
				break;
			else
				i++;
		}
		print_liste(root);
	}
	return (0);
}