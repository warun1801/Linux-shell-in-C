/********************************************************************************************
This is a template for assignment on writing a custom Shell. 

Students may change the return types and arguments of the functions given in this template,
but do not change the names of these functions.

Though use of any extra functions is not recommended, students may use new functions if they need to, 
but that should not make code unnecessorily complex to read.

Students should keep names of declared variable (and any new functions) self explanatory,
and add proper comments for every logical step.

Students need to be careful while forking a new process (no unnecessory process creations) 
or while inserting the signal handler code (which should be added at the correct places).

Finally, keep your filename as myshell.c, do not change this name (not even myshell.cpp, 
as you dp not need to use any features for this assignment that are supported by C++ but not by C).
*********************************************************************************************/

#include <stdio.h>	  // i/o functions
#include <string.h>	  // basic string manipulation
#include <stdlib.h>	  // exit()
#include <unistd.h>	  // fork(), getpid(), exec()
#include <sys/wait.h> // wait()
#include <sys/types.h>
#include <signal.h> // signal()
#include <fcntl.h>	// close(), open()

//MACROS
#define MAXCOMM 4
#define MAXSTR 1024

//Globals
int alpha = 1;

//parser functions
void parseSpace(char *str, char **commands)
{
	// for parsing strings with spaces and no other special tokens
	int i;
	for (i = 0; i < MAXSTR; i++)
	{
		commands[i] = strsep(&str, " "); //seperating via space(" ")
		if (commands[i] == NULL)
			break;
		if (strlen(commands[i]) == 0)
			i--;
	}
}

int parseParallel(char *str, char **commands)
{
	// for parsing strings with "&&" seperator, for parallel running
	int i;
	for (i = 0; i < MAXCOMM; i++)
	{
		commands[i] = strsep(&str, "&&"); //seperating via ("&&")
		if (commands[i] == NULL)
			break;
		if (strlen(commands[i]) == 0)
			i--;
	}
	if (commands[1] == NULL)
		return 0;
	return 1;
}

int parseSequential(char *str, char **commands)
{
	//for parsing strings with "##" seperator, for sequential running
	int i;
	for (i = 0; i < MAXCOMM; i++)
	{
		commands[i] = strsep(&str, "##"); //seperating via ("##")
		if (commands[i] == NULL)
			break;
		if (strlen(commands[i]) == 0)
			i--;
	}
	if (commands[1] == NULL)
		return 0;
	return 1;
}

int parseRedirect(char *str, char **commands)
{
	//for parsing strings with ">" seperator, for redirecting output
	int i;
	for (i = 0; i < 2; i++)
	{
		commands[i] = strsep(&str, ">"); //seperating via (">")
		if (commands[i] == NULL)
			break;
		if (strlen(commands[i]) == 0)
			i--;
	}
	if (commands[1] == NULL)
		return 0;
	return 1;
}

int parseInput(char *str, char **commands[])
{
	// This function will parse the input string into multiple commands or a single command with arguments depending on the delimiter (&&, ##, >, or spaces).
	int parseflag = 0;
	char *parser[MAXCOMM];

	//checks for each case
	parseflag = parseParallel(str, parser); //parallel checking
	if (parseflag)
	{
		// for handling if the string is a parallel chain of commands
		int numCommands = 0;
		while (numCommands < MAXCOMM)
		{
			if (parser[numCommands] == NULL)
				break;
			parseSpace(parser[numCommands], commands[numCommands]);
			numCommands++;
		}
		return 0;
	}

	parseflag = parseSequential(str, parser); //sequential checking
	if (parseflag)
	{
		// for handling if the string is a sequential chain of commands
		int numCommands = 0;
		while (numCommands < MAXCOMM)
		{
			if (parser[numCommands] == NULL)
				break;
			parseSpace(parser[numCommands], commands[numCommands]);
			numCommands++;
		}
		return 1;
	}

	parseflag = parseRedirect(str, parser); //for checking the redirect case
	if (parseflag)
	{
		// for handling the redirect case
		parseSpace(parser[0], commands[0]);
		parseSpace(parser[1], commands[1]);
		return 2;
	}

	parseSpace(str, commands[0]); //for single command cases

	if (strcmp("exit", commands[0][0]) == 0) //for exit case
		return 3;
	else if (strcmp("cd", commands[0][0]) == 0) //for cd case
		return 4;
	else
	{
		return 5; //for normal commands
	}
}

//EXECUTABLE COMMANDS
void executeCommand(char **parsed)
{
	// This function will fork a new process to execute a command
	if (strcmp("exit", parsed[0]) == 0)
	{
		//for handling the exit case
		printf("Exiting shell...\n");
		alpha = -1;
		exit(0);
	}
	else if (strcmp(parsed[0], "cd") == 0)
	{
		//for handling the cd case
		int err = chdir(parsed[1]);
		if (err == -1)
		{
			printf("Shell: Incorrect command\n");
			exit(0);
		}
	}
	else
	{
		pid_t pid = fork(); //forking a child process

		if (pid < 0)
		{
			printf("Shell: Incorrect command\n"); //fork unsuccessful
		}
		else if (pid == 0)
		{

			if (execvp(parsed[0], parsed) < 0)
			{
				printf("Shell: Incorrect command\n"); //wrong command
			}
			exit(0);
		}
		else
		{
			//parent
			wait(NULL);
			return;
		}
	}
}

void executeParallelCommands(char **commands[])
{
	// This function will run multiple commands in parallel54
	int num;
	for (num = 0; num < MAXCOMM; num++)
	{
		//checking for the num of commands to execute parallely
		if (commands[num] == NULL || commands[num][0] == NULL)
			break;
	}
	printf("num = %d\n", num);

	pid_t pid = fork(); //forking a child process
	if (pid < 0)
	{
		printf("Shell: Incorrect command\n"); //unable to fork a child
	}
	else if (pid == 0 && num >= 1)
	{
		if (strcmp("exit", commands[0][0]) == 0)
		{
			//for handling the exit case
			printf("Exiting shell...\n");
			alpha = -1;
			return;
		}
		else if (strcmp(commands[0][0], "cd") == 0)
		{
			//for handling cd case
			int err = chdir(commands[0][1]);
			if (err == -1)
				printf("Shell: Incorrect command\n"); //wrong address
		}
		else if (execvp(commands[0][0], commands[0]) < 0) //simple command execution
		{
			printf("Shell: Incorrect command\n"); //incorrect command
		}
	}
	else
	{
		if (strcmp("exit", commands[1][0]) == 0)
		{
			//for handling the exit case
			printf("Exiting shell...\n");
			alpha = -1;
			return;
		}
		else if (strcmp(commands[1][0], "cd") == 0)
		{
			//for handling cd case
			int err = chdir(commands[1][1]);
			if (err == -1)
				printf("Shell: Incorrect command\n"); //wrong address
		}
		else
		{
			pid_t pid2 = fork();
			if (pid2 == 0)
			{
				if (execvp(commands[1][0], commands[1]) < 0) //simple command execution
				{
					printf("Shell: Incorrect command\n"); //incorrect command
				}
				return;
			}
			else if (pid2 && num > 2)
			{
				//If 3 commands
				if (strcmp("exit", commands[2][0]) == 0)
				{
					//for handling the exit case
					printf("Exiting shell...\n");
					alpha = -1;
					return;
				}
				else if (strcmp(commands[2][0], "cd") == 0)
				{
					//for handling cd case
					int err = chdir(commands[2][1]);
					if (err == -1)
						printf("Shell: Incorrect command\n"); //wrong address
				}
				else
				{
					pid_t pid3 = fork();
					if (pid3 == 0)
					{
						if (execvp(commands[2][0], commands[2]) < 0) //simple command execution
						{
							printf("Shell: Incorrect command\n"); //incorrect command
						}
						return;
					}
					else if (pid3 && num > 3)
					{
						//If 4 commands
						if (strcmp("exit", commands[2][0]) == 0)
						{
							//for handling the exit case
							printf("Exiting shell...\n");
							alpha = -1;
							return;
						}
						else if (strcmp(commands[2][0], "cd") == 0)
						{
							//for handling cd case
							int err = chdir(commands[2][1]);
							if (err == -1)
								printf("Shell: Incorrect command\n"); //wrong address
						}
						else
						{
							pid_t pid4 = fork();
							if (pid4 == 0)
							{
								if (execvp(commands[3][0], commands[3]) < 0) //simple command execution
								{
									printf("Shell: Incorrect command\n"); //incorrect command
								}
								return;
							}
							else
							{
								wait(NULL);
								return;
							}
						}
					}
				}
			}
		}
	}
}

void executeSequentialCommands(char **commands[])
{
	// This function will run multiple commands in sequence
	int i;
	int sz = MAXCOMM;
	int ex;
	for (i = 0; i < sz; i++)
	{
		if (commands[i] == NULL || commands[i][0] == NULL)
			break;
		executeCommand(commands[i]);
		if (alpha == -1)
			break;
	}
}

void executeCommandRedirection(char **commands[])
{
	// This function will run a single command with output redirected to an output file specificed by user
	if (commands[1] == NULL || commands[1][0] == NULL || strlen(commands[1][0]) == 0)
		printf("Shell: Incorrect command\n");
	else
	{
		pid_t pid = fork();
		if (pid == 0)
		{
			// puts(commands[1][0]);
			close(STDOUT_FILENO);
			open(commands[1][0], O_CREAT | O_RDWR, S_IRWXU);

			if (execvp(commands[0][0], commands[0]) < 0)
			{
				printf("Shell: Incorrect command\n");
			}
			return;
		}
		else
		{
			wait(NULL);
			return;
		}
	}
}

int main()
{
	// Initial declarations
	char cwd[10000];
	char *buffer = NULL;
	int parse_flag, err;
	size_t bufsize = 0;
	char **commands[MAXCOMM + 1];
	// commands = (char **)malloc(MAXCOMM*sizeof(char **));
	int i;
	for (i = 0; i < MAXCOMM; i++)
	{
		commands[i] = (char **)malloc(MAXSTR * sizeof(char **));
	}
	commands[MAXCOMM] = NULL;
	while (1) // This loop will keep your shell running until user exits.
	{
		// Print the prompt in format - currentWorkingDirectory$
		getcwd(cwd, sizeof(cwd));
		printf("%s$", cwd);

		// accept input with 'getline()'
		getline(&buffer, &bufsize, stdin);
		buffer = strsep(&buffer, "\n");
		if (strlen(buffer) == 0)
		{
			continue;
		}

		// Parse input with 'strsep()' for different symbols (&&, ##, >) and for spaces.
		parse_flag = parseInput(buffer, commands);

		if (parse_flag == 0)
		{
			executeParallelCommands(commands); // This function is invoked when user wants to run multiple commands in parallel (commands separated by &&)
			if (alpha == -1)
				break;
		}
		else if (parse_flag == 1)
		{
			executeSequentialCommands(commands); // This function is invoked when user wants to run multiple commands sequentially (commands separated by ##)
			if (alpha == -1)
				break;
		}
		else if (parse_flag == 2)
		{
			executeCommandRedirection(commands); // This function is invoked when user wants redirect output of a single command to and output file specificed by user
			if (alpha == -1)
				break;
		}
		else if (parse_flag == 3) // When user uses exit command.
		{
			printf("Exiting shell...\n");
			break;
		}
		else if (parse_flag == 4) //When user uses cd command.
		{
			err = chdir(commands[0][1]);
			if (err == -1)
				printf("Shell: Incorrect command\n");
		}
		else
		{
			executeCommand(commands[0]); // This function is invoked when user wants to run a single commands
		}
	}

	return 0;
}
