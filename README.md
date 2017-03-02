# linux-shell

Author: Kaylee Yuhas

Project Description: 
This program creates a linux shell environment. The shell can take in 1024 characters, and can hold 512 arguments. There is also a path variable that can hold 512 different paths. The program works by taking in the user input and parsing it, separating it into individual commands. The program handles three internal commands (cd, path, and quit) and executes these commands in the parent process. If a command other than any of these is typed, the command will be executed in a child process if it is found. The program "finds" these commands by searching through each directory in the path variable. 
