#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

#define MAX_LINE 80
#define NUM_TOKENS (MAX_LINE / 2 + 1)
#define HISTORY_DEPTH 10


/**
 * Command Input and Processing
 */
int index = 0;
int counter = 0;
int historyIterator = 0;
char temp_buffer[HISTORY_DEPTH][MAX_LINE];

void Get_History(char *buff)
{
	char shift_buffer[HISTORY_DEPTH - 1][MAX_LINE];
	if (index >= 10)
	{
		for (int shift = 0; shift < 9; shift++)
		{
			memmove(shift_buffer[shift], temp_buffer[shift + 1], strlen(temp_buffer[shift + 1]));
			memset(temp_buffer[shift + 1], 0, strlen(temp_buffer[shift + 1]));
		}
		memset(temp_buffer[0], 0, strlen(temp_buffer[0]));
		for (int shift = 0; shift < 9; shift++)
		{
			memmove(temp_buffer[shift], shift_buffer[shift], strlen(shift_buffer[shift]));
			memset(shift_buffer[shift], 0, strlen(shift_buffer[shift]));
		}
		memset(temp_buffer[9], 0, strlen(temp_buffer[9]));
		memmove(temp_buffer[9], buff, strlen(buff));
		counter = 0;
	}
	else
	{
		memmove(temp_buffer[counter], buff, strlen(buff));
		counter++;
	}
	index++;
}

int IndexCall(char *buff)
{
	char maxIndex[MAX_LINE];
	char minIndex[MAX_LINE];
	char check[MAX_LINE];
	sprintf(maxIndex, "%i", index);
	sprintf(minIndex, "%i", index - 10);

	if (index != 0)
	{
		if (buff[1] != '!')
		{
			/*
			*
			*	Handles the command !n
			*
			*/
			if ((strlen(buff) - 1 == strlen(maxIndex)) || ((strlen(buff) - 1 == strlen(minIndex))))
			{
				int x = 0;
				int numberCap = 0;
				if (index <= 10)
				{
					numberCap = index;
				}
				else
				{
					numberCap = 10;
				}

				for (int i = 0; i <= numberCap; i++)
				{
					if (buff[i + 1] <= maxIndex[i])
					{
						x++;
					}
					else
					{
						x--;
					}

					if (buff[i + 1] >= minIndex[i])
					{
						x++;
					}
					else
					{
						x--;
					}
				}
				if (x < 0)
				{
					write(STDOUT_FILENO, "SHELL: Unknown history command.", strlen("SHELL: Unknown history command."));
					historyIterator--;
					return 1;
				}

				if (x >= 0)
				{
					buff[0] = '\n';
					memmove(buff, buff + 1, strlen(buff));
					int convertedIndex = -1;
					int y = index - 10;
					if (index <= 10)
					{
						y = 0;
					}
					sprintf(check, "%i", y);
					while (memcmp(buff, check, strlen(check)) != 0)
					{
						y++;
						convertedIndex++;
						sprintf(check, "%i", y);

					}
					memset(buff, 0, strlen(buff));
					memmove(buff, temp_buffer[convertedIndex], strlen(temp_buffer[convertedIndex]));
				}

			}
			else
			{
				write(STDOUT_FILENO, "SHELL: Unknown history command.", strlen("SHELL: Unknown history command."));
				historyIterator--;
				return 1;
			}
		}
		/*
		*
		*	Handles the command !!
		*
		*/
		if (buff[1] == '!')
		{
			if (index <= 10)
			{
				memset(buff, 0, strlen(buff));
				memmove(buff, temp_buffer[index - 1], strlen(temp_buffer[index - 1]));
			}
			else
			{
				memset(buff, 0, strlen(buff));
				memmove(buff, temp_buffer[9], strlen(temp_buffer[9]));
			}
		}

	}
	if (index == 0)
	{
		write(STDOUT_FILENO, "SHELL: Unknown history command.", strlen("SHELL: Unknown history command."));
		historyIterator--;
		return 1;
	}
	return 0;
}



/*
 * Tokenize the string in 'buff' into 'tokens'.
 * buff: Character array containing string to tokenize.
 *       Will be modified: all whitespace replaced with '\0'
 * tokens: array of pointers of size at least MAX_LINE/2 + 1.
 *       Will be modified so tokens[i] points to the i'th token
 *       in the string buff. All returned tokens will be non-empty.
 *       NOTE: pointers in tokens[] will all point into buff!
 *       Ends with a null pointer.
 * returns: number of tokens.
 */
int Tokenize_Command(char *buff, char *tokens[])
{
	int addHistory = 0;
	if ((buff[0] == '!'))
	{
		addHistory = IndexCall(buff);
	}
	if ((buff[0]) == '\0')
	{
		addHistory = 1;
		historyIterator--;
	}

	if (addHistory != 1)
	{
		Get_History(buff);
	}

	/*if (buff[0] == '!')
	{
		collect_exclamation(buff);
	}*/
	int token_count = 0;
	_Bool in_token = false;
	int num_chars = strnlen(buff, MAX_LINE);




	for (int i = 0; i < num_chars; i++) {
		switch (buff[i]) {
			// Handle token delimiters (ends):
		case ' ':
		case '\t':
		case '\n':
			buff[i] = '\0';
			in_token = false;
			break;

			// Handle other characters (may be start)
		default:
			if (!in_token) {
				tokens[token_count] = &buff[i];
				token_count++;
				in_token = true;
			}
		}
	}
	tokens[token_count] = NULL;
	return token_count;
}

/**
 * Read a command from the keyboard into the buffer 'buff' and tokenize it
 * such that 'tokens[i]' points into 'buff' to the i'th token in the command.
 * buff: Buffer allocated by the calling code. Must be at least
 *       MAX_LINE bytes long.
 * tokens[]: Array of character pointers which point into 'buff'. Must be at
 *       least NUM_TOKENS long. Will strip out up to one final '&' token.
 *       tokens will be NULL terminated (a NULL pointer indicates end of tokens).
 * in_background: pointer to a boolean variable. Set to true if user entered
 *       an & as their last token; otherwise set to false.
 */
void Read_Command(char *buff, char *tokens[], _Bool *in_background)
{
	*in_background = false;

	// Read input

	int length = read(STDIN_FILENO, buff, MAX_LINE - 1);
	if ((length < 0) && (errno != EINTR))
	{
		perror("Unable to read command. Terminating.\n");
		exit(-1); /* terminate with error */
	}

	// Null terminate and strip \n.
	buff[length] = '\0';
	if (buff[strlen(buff) - 1] == '\n') {
		buff[strlen(buff) - 1] = '\0';
	}


	// Tokenize (saving original command string)
	int token_count = Tokenize_Command(buff, tokens);
	if (token_count == 0) {
		return;
	}

	// Extract if running in background:
	if (token_count > 0 && strcmp(tokens[token_count - 1], "&") == 0) {
		*in_background = true;
		tokens[token_count - 1] = 0;
	}
}
void History()
{
	if (index > 10)
	{
		historyIterator = 10;
	}

	int historyIndex = index - historyIterator + 1;
	char historyIndexS[] = "0";

	for (int i = 0; i < historyIterator; i++)
	{
		sprintf(historyIndexS, "%i", historyIndex);
		write(STDOUT_FILENO, historyIndexS, strlen(historyIndexS));
		write(STDOUT_FILENO, ":\t", strlen(":\t"));
		write(STDOUT_FILENO, temp_buffer[i], sizeof(temp_buffer[i]));
		write(STDOUT_FILENO, "\n", strlen("\n"));
		historyIndex++;
	}
}

void Siginthandler(int param)
{
	History();
	signal(SIGINT, Siginthandler);
}

/**
 * Main and Execute Commands
 */
int main(int argc, char* argv[])
{
	char input_buffer[MAX_LINE];
	char *tokens[NUM_TOKENS];
	char execvpError[] = "Unknown command.\n";
	char invalidDir[] = "Invalid Directory.\n";
	char currentPath[MAX_LINE];

	while (true)
	{
		int status = 0;
		int error = 0;
		// Get command
		// Use write because we need to use read() to work withmemcpy
		// signals, and read() is incompatible with printf().
		getcwd(currentPath, sizeof(currentPath));
		write(STDOUT_FILENO, currentPath, strlen(currentPath));
		write(STDOUT_FILENO, "> ", strlen("> "));
		_Bool in_background = false;
		signal(SIGINT, Siginthandler);

		Read_Command(input_buffer, tokens, &in_background);


		/*
		*
		*	2.2 Internal Commands
		*	2.3 Creating a History Feature
		*
		*/
		historyIterator++;

		if (tokens[0] != NULL)
		{
			if (strcmp(tokens[0], "pwd") == 0)
			{
				getcwd(currentPath, sizeof(currentPath));
				write(STDOUT_FILENO, currentPath, strlen(currentPath));
				status = 1;
			}
			else if (strcmp(tokens[0], "cd") == 0)
			{
				int checkDir = chdir(tokens[1]);
				if (checkDir == -1)
				{
					write(STDOUT_FILENO, invalidDir, strlen(invalidDir));
				}
				status = 1;
			}
			else if (strcmp(tokens[0], "exit") == 0)
			{
				exit(0);
			}
			else if (strcmp(tokens[0], "history") == 0)
			{
				History();
				status = 1;
			}
			else if (strncmp(tokens[0], "!", 1) == 0)
			{
				status = 1;
			}
		}

		/*
		*
		*	End of 2.2 & 2.3
		*
		*/

		if (status == 0)
		{
			pid_t pid = fork();
			if (pid < 0)
			{
				perror("Failed to create child");
			}
			else if (pid == 0)
			{
				error = execvp(tokens[0], tokens);
				if (error == (-1))
				{
					write(STDOUT_FILENO, tokens[0], strlen(tokens[0]));
					write(STDOUT_FILENO, ":\t", strlen(":\t"));
					write(STDOUT_FILENO, execvpError, sizeof(execvpError));
					exit(0);
				}
			}
			if (!in_background)
			{
				waitpid(pid, NULL, 0);
			}
		}


		// DEBUG: Dump out arguments:
		for (int i = 0; tokens[i] != NULL; i++) {
			write(STDOUT_FILENO, "   Token: ", strlen("   Token: "));
			write(STDOUT_FILENO, tokens[i], strlen(tokens[i]));
			write(STDOUT_FILENO, "\n", strlen("\n"));
		}
		if (in_background)
		{
			write(STDOUT_FILENO, "Run in background.", strlen("Run in background."));
		}

		/**
		 * Steps For Basic Shell:
		 * 1. Fork a child process
		 * 2. Child process invokes execvp() using results in token array.
		 * 3. If in_background is false, parent waits for
		 *    child to finish. Otherwise, parent loops back to
		 *    Read_Command() again immediately.
		 */

	}
	return 0;
}
