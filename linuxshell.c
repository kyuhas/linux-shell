/*
 * Author(s): Kaylee Yuhas
 * Date: 10 Nov 2016
 * CIS 340 Systems Programming
 * Description: This project creates a shell environment that recognizing the internal
 * commands: cd, path, and quit. External commands are searching for in the directories
 * specified by the user.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include "command.h"
#include "paths.h"	

#define MAX_SIZE	1024

/*
 * This method creates a pointer to a command data type. It makes argv a "string" array 
 * that has MAX_SIZE/2 elements. From here, each word is parsed and placed in the array. 
 * The name of the command is called the same as the first command typed 
 * (e.g. "path + /abc" is named "path")
 */
command *parseCommand(char *userInput) {
	command *c;
	c = (command *)malloc(sizeof(command));
	c->argv = (char **)malloc(sizeof(char *) * MAX_SIZE/2);
	c->name = NULL;
	
	int i = 0;
	char *ptr = strtok(userInput, " ");
	while(ptr != NULL && i < MAX_SIZE/2) {
		c->argv[i] = (char *)malloc(sizeof(char) * strlen(ptr));
		strcpy(c->argv[i], ptr);
		i++;
		ptr = strtok(NULL, " ");
	}
	if(ptr == NULL && i == 0) {
		printf("Invalid command.\n");
		c->name = "null";
		c->argv[i] = "null";
		c->argc = 1;
		return c;
	}
	c->name = (char *)malloc(sizeof(char) * strlen(c->argv[0]));
	strcpy(c->name, c->argv[0]);
	c->argv[i] = (char *)malloc(sizeof(char));
	c->argv[i] = NULL;
	c->argc = i;
	
	return c;
}

/* 
 * This method frees all of the dynamically-allocated memory associated with 
 * a command data type. First, the argv[i] values are all freed. 
 * Then, the rest of the dynamically allocated data members are freed.
 * Note: argc does not need to be freed because we did not use malloc to create it.
*/
void freeCommand(command *c) {
	int i;
	for(i = 0; i < c->argc; i++) {
		c->argv[i] = NULL;
		free(c->argv[i]);
	}
	free(c->argv);
	free(c->name);
}

/* 
 * This method frees all of the dynamically-allocated memory associated with 
 * a paths data type. First, the dirs[i] values are all freed. 
 * Then, the rest of the dynamically allocated data members are freed.
 * Note: numDirs does not need to be freed because we did not use malloc to create it.
*/
void freePaths(paths *path) {
	int i;
	for(i = 0; i < path->numDirs; i++) {
		path->dirs[i] = NULL;
		free(path->dirs[i]);
	}	
	free(path->dirs);
	free(path->name);
}

/*
 * This method starts off by freeing the previous path name to "reset" its value.
 * Then, it finds the length of the new path name by looking through the dirs array.
 * Once found, a new path name is created and set to its new value.
 */
void getPathName(paths *p) {
	p->name = NULL;
	free(p->name); //free what was there before
	int i; int length = 0;
	for(i = 0; i < p->numDirs; i++) {
		if(i != 0) {
			length++; // for the colon you need to add!
		}
		length = length + strlen(p->dirs[i]);
	}
	p->name = (char *)malloc(sizeof(char)*(length + 1));
	for(i = 0; i < p->numDirs; i++) {
		if(i > 0) {
			strcat(p->name, ":");
		}
		strcat(p->name, p->dirs[i]);
	}
}

/* 
 * This method tests to see if the string the user wants to append will fit. 
 * The path array "dirs" will only fit 1024 strings.
 * If it can fit, it is added to the array and the path name is updated accordingly.
*/
void addPathName(paths *p, char *add) {
	if(p->numDirs == MAX_SIZE) {
		printf("Can't add that path because the path name is full.\n");
		return;
	}
	p->dirs[p->numDirs] = (char *)malloc(sizeof(char) * strlen(add + 1));
	strcpy(p->dirs[p->numDirs], add);
	p->numDirs = p->numDirs + 1;
	getPathName(p);
	printf("%s successfully added to the path name!\n", add);
}

/* 
 * This method searches through the path's dirs array to look for the directory specified 
 * by the user. If found, the index of the value is returned. If not, -1 is returned.
*/
int findLocation(paths *p, char *string) {
	int i; int location = -1;
	for(i = 0; i < p->numDirs; i++) {
		if(strcmp(p->dirs[i], string) == 0) {
			//we found the entry!
			location = i;
			break;
		}
	}
	return location;
}

/* 
 * This method frees the element at index "location" in the dirs array. 
 * It then shifts everything else in the array to the left one.
*/
void removeElement(paths *p, int location) {
	if(location != p->numDirs - 1) {
		//it is NOT the last element in the dirs array
		int i;
		for(i = location; i < p->numDirs - 1; i++) {
			free(p->dirs[i]);
			unsigned long nextEntryLength = strlen(p->dirs[i+1]);
			p->dirs[i] = (char *)malloc(sizeof(char) * nextEntryLength);
			strcpy(p->dirs[i], p->dirs[i+1]);
		}
		free(p->dirs[i]); // free the last element of the dirs array
	}
	else {
		//it IS the last element
		free(p->dirs[location]);
	}
}

/* 
 * This method checks to see if there is anything to remove from the path array.
 * If it is possible to remove something, it checks to see if that thing is present in 
 * the array. If it is, it will be removed and the pathname will be updated. If it is not
 * in the array, an error message will be returned and the paths data will be unchanged.
*/
void removePathName(paths *p, char *remove) {
	if(p->numDirs == 0) {
		printf("Can't remove that path because the path name is empty.\n");
		return;
	}
	int location = findLocation(p, remove);
	if(location == -1) {
		printf("That directory is not included in the path name. Try again.\n");
		return;
	}
	removeElement(p, location);
	p->numDirs = p->numDirs - 1;
	getPathName(p);
	printf("%s successfully removed from the path name!\n", remove);	
}

/*
 * This method prints out a "$ " to the screen, similar to the bash shell.
 */
void printOutput() {
	char buffer[2] = "$ ";
	write(1, buffer, 2);
}

/*
 * This method takes in a command and paths data type and finds the length of the new
 * string that will hold the file' name. It will append a slash and the command name 
 * (e.g. "/ls" to the end of each entry in the dirs array. If it is found, it will 
 * execute that file. If not, it will print an error message.
 */
void executeCommand(command *c, paths *p) {
	unsigned long commandLength = strlen(c->name);
	unsigned long pathLength, i;
	char *fileName;
	for(i = 0; i < p->numDirs; i++) {
		pathLength = commandLength + strlen(p->dirs[i]) + 1; // 1 is for slash and '\0'
		fileName = (char *)malloc(sizeof(char) * pathLength);
		strcpy(fileName, p->dirs[i]);
		strcat(fileName, "/");
		strcat(fileName, c->name);
		execv(fileName, c->argv);
	}
	printf("Did not find the file.\n");
}

/*
 * This method takes in a command and paths data type and determines what command it 
 * should execute. 
 */
int determineCommands(command *cmd, paths *path) {
	/*					quit					*/
	if(strcmp(cmd->name, "quit") == 0 && cmd->argc == 1) {
		int bool = 0;
		return bool;
	}
	
	/*					path					*/
	else if(strcmp(cmd->name, "path") == 0) {
		if(cmd->argc == 1) {
			if(path->name != NULL) {
				getPathName(path);
				printf("The current pathname is: %s\n", path->name);
			}
			else {
				printf("The current pathname is empty.\n");
			}
		}
		else if(cmd->argc == 3) {
			if(strcmp(cmd->argv[1], "+") == 0) {
				addPathName(path, cmd->argv[2]);
			}
			else if(strcmp(cmd->argv[1], "-") == 0) {
				removePathName(path, cmd->argv[2]);
			}
			else {
				//they entered something other than + or - as second input
				printf("Invalid format\n");
			}
		}
		else {
			//they entered an incorrect amount of commands
			printf("Incorrect format.\n");
		}
		return 1;
	}
	
	/*					cd					*/
	else if(strcmp(cmd->name, "cd") == 0 && cmd->argc == 2) {
		if(cmd->argc == 2) {
			int dir = chdir(cmd->argv[1]);
			if(dir == -1) {
				printf("Could not change directories.\n");
			}
			else if(dir == 0) {
				printf("Successfully changed to %s directory.\n", cmd->argv[1]);
			}
		}
		else {
			//they entered an incorrect amount of commands
			printf("Incorrect format.\n");
		}
		return 1;
	}
	
	/*					not internal command					*/
	else {
		pid_t pid;
		int status;
		pid = fork();
		
		if(pid == -1) {
			printf("An error occurred. Cannot run that command.\n");
			return 1;
		}
		else if(pid == 0) {
			//child process
			if(path->numDirs > 0) {
				executeCommand(cmd, path); 
			}
			else {	
				printf("Cannot find file. No directories to search.\n");
			}
			_exit(0);
		}
		else {	
			//good parent -- waits for child to finish executing
			wait(&status);
			return 1;
		}
	}
}

/*
 * The main method of this program creates a paths data type first, which will hold
 * the pathname and an array of all the directories that the shell will search through
 * for any commands that are not internal commands. The structure can hold up to 512 
 * directories. 
 */
int main() {	
	paths *path;
	path = (paths *)malloc(sizeof(paths));
	path->name = NULL;
	path->numDirs = 0;
	path->dirs = (char **)malloc(sizeof(char *) * (MAX_SIZE/2));
	//can hold max_size/2 values

	command *c = (command *)malloc(sizeof(command));
	c->name = NULL;
	c->argc = 0;
	c->argv = (char **)malloc(sizeof(char *) * (MAX_SIZE/2));
	//can hold max_size/2 values
	
	char *userInput; 
	int boolean = 1;
	while(boolean) {
		printOutput();
		//can hold up to 1024 characters
		userInput = (char *)malloc(sizeof(char) * MAX_SIZE); 
		ssize_t sizeofInput = read(0, (void *)userInput, MAX_SIZE);
		userInput[sizeofInput - 1] = '\0';
		
		c = parseCommand(userInput);
		
		if(strcmp(c->name, "null") != 0) {
			boolean = determineCommands(c, path);
			freeCommand(c);
			free(c);
		}
		else {
			c->argv[0] = NULL;
			free(c->argv);
			free(c);
		}
		
		free(userInput);
	}
	freePaths(path);
	free(path);
	return 0;
}


