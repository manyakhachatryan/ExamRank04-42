#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int	perr(const char *error, const char *argument)
{
	while (*error)
	{
		write(2, error, 1);
		++error;
	}
	while (argument && *argument)
	{
		write(2, argument, 1);
		++argument;
	}
	write(2, "\n", 1);
	return (1);
}

static int exec(char **argv, char **env, int i, int fd)
{
	argv[i] = NULL;
	dup2(fd, 0);
	close(fd);
	execve(argv[0], argv, env);
	return (perr("error: cannot execute ", argv[0]));
}

int	main(int argc, char **argv, char **env)
{
	(void) argc;
	int	i = 0;
	int fd[2];
	int	tmp_fd = dup(0);

	while (argv[i] && argv[i + 1])
	{
		argv = &argv[i + 1];
		i = 0;
		while (argv[i] && strcmp(argv[i], "|") && strcmp(argv[i], ";"))
			++i;			
		if (strcmp(argv[0], "cd") == 0)
		{
			if (i != 2)
			{
				perr("error: cd: bad arguments", NULL);
			}
			else if (chdir(argv[1]) != 0)
			{
				perr("error: cd: cannot change directory to ", argv[1]);
			}	
		}
		else if (i && (argv[i] == NULL || strcmp(argv[i], ";") == 0))
		{
			if (fork() == 0)
			{
				if (exec(argv, env, i, tmp_fd))
				{
					return (1);
				}
			}
			else
			{
				close(tmp_fd);
				while (waitpid(-1, NULL, WUNTRACED) != -1)
					;
				tmp_fd = dup(0);
				
			}
		}
		else if (i && strcmp(argv[i], "|") == 0)
		{
			pipe(fd);
			if (fork() == 0)
			{
				dup2(fd[1], 1);
				close(fd[0]);
				close(fd[1]);
				if (exec(argv, env, i, tmp_fd))
				{
					return (1);
				}
			}
			else
			{
				close(fd[1]);
				close(tmp_fd);
				tmp_fd = fd[0];
			}
		}
	}
	close(tmp_fd);
}
