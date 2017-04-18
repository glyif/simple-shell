#include "header.h"

/**
 * exec_builtins - custom function to execute builtin commands
 * @arginv: arguments inventory
 *
 * Return: 1 on success, 0 on failure
 */
int exec_builtins(arg_inventory_t *arginv)
{
	int i, retval, old_stdout;
	char *str, **commands;
	builtins_t builtins_list[] = {

		{"arsine", _arsine}, {"env", _env}, {"setenv", _setenv},
		{"unsetenv", _unsetenv}, {"history", _history}, {"cd", _cd},
		{"alias", _alias}, {"unalias", _unalias}, {"help", the_help},
		{"exit", the_exit},
		{NULL, NULL}
	};

	retval = EXT_FAILURE;
	commands = (char **)arginv->commands;

	old_stdout = redirect_output(arginv, 0);

	for (i = 0; ((str = builtins_list[i].command) != NULL); i++)
	{
		if (_strcmp(str, commands[0]) == 0)
		{
			retval = builtins_list[i].builtin_func(arginv);
			break;
		}
	}

	if (arginv->io_redir == TOKEN_REWRITE || arginv->io_redir ==
		TOKEN_APPEND || arginv->pipeout)
	{
		/* revert back to old_stdout */
		safe_dup2(old_stdout, STDOUT_FILENO);

		close(old_stdout);
	}

	return (retval);
}

/**
 * exec_path - custom function to execute from PATH
 * @command: command to execute
 * @arginv: arg inventory
 *
 * Return: pid of parent
 */
pid_t exec_path(char *command, arg_inventory_t *arginv)
{
	pid_t pid;
	char **_environ;

	pid = fork();
	if (pid < 0)
	{
		perror("Critical error: unable to fork()!");
		exit(1);
	}

	if (pid == 0)
	{
		redirect_input(arginv);
		redirect_output(arginv, 1);

		_environ = zelda_to_ganondorf(arginv->envlist);

		if (execve(command, (char **)arginv->commands, _environ) < 0)
		{
			perror("No Command");
			exit(1);
		}
	}
	return (pid);
}

/**
 * execute - completes execution of input commands
 * @arginv: arguments inventory
 *
 * Return: void
 */
pid_t execute(arg_inventory_t *arginv)
{
	env_t *envlist;
	char **commands;
	char *path, *command;
	char **paths;

	envlist = arginv->envlist;

	commands = (char **)arginv->commands;

	command = safe_malloc(sizeof(char) * BUFSIZE);
	command = _strcpy(command, *commands);

	path = safe_malloc(sizeof(char) * BUFSIZE);

	if (exec_builtins(arginv) == EXT_FAILURE)
	{
		if (is_path(command))
		{
			return (exec_path(command, arginv));
		}
		else
		{
			locate_path(path, envlist);
			paths = tokenize_path(path);
			cat_path(paths, command);
			free_paths(paths);
			return (exec_path(command, arginv));
		}
	}
	return (-1);
}
