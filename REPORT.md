PROJECT REPORT

NAME - Chandra Sekhar Panda
Regd No - 2241016069

MiniShell in C++ [CAPSTONE PROJECT]

1. Introduction

A shell is a program that allows users to interact with an operating system via commands. This project focuses on building a simplified version of a UNIX-like shell using C++. The primary objective is to understand system-level programming and how shells execute commands at the operating system level. The project implements basic command execution, background process handling, input/output redirection, piping, and job management.

2. Project Objectives

1) The key goals of this project are
2) To take user input and convert it into meaningful commands.
3) To execute commands using system calls (fork(), execvp(), and waitpid()).
4) To add the ability to run commands in background mode.
5) To support input and output file redirection using < and >.
6) To enable command piping using | to pass output of one command as input to another.
7) To track and display running background jobs.

3. Development Process (Day-wise Work)

The work was divided into five days:

Day 1 – Planning and Input Parsing
Finalized shell requirements and implemented a tokenizer that reads user input and splits it into individual command components.

Day 2 – Basic Command Execution
Focused on executing commands entered by the user using fork() to create a child process and execvp() to replace the child process with the new command.

Day 3 – Background Process Support
Implemented support for running commands in the background using &. The shell stores background process IDs and prevents zombie processes by reaping terminated ones using waitpid().

Day 4 – Redirection and Piping
Implemented input (<) and output (>) redirection using dup2() to modify standard input or output.
Implemented command piping using pipe() and two child processes to link output of one command to another.

Day 5 – Job Control
Added job tracking. The jobs command prints currently running background tasks by storing their process IDs along with command strings.


4. System Design and Workflow

The sequence of operations in the shell is:
1) Display prompt (minishell>).
2) Read user input
3) Convert input into tokens
4) Detect and process:
     -background execution (&)
     -redirection (< / >)
     -piping (|)
     -normal command execution
5) Use system calls to execute the requested operation.
The shell runs continuously until the user types exit.

5. Important Code Logic

Command execution is done by creating a child process:

pid_t pid = fork();
if (pid == 0) {
    execvp(args[0], args.data());
} else {
    waitpid(pid, &status, 0);
}

Background process execution:

if (is_background) {
    g_jobs[pid] = command_string;
}

Redirection is handled using dup2():

dup2(fd_in, STDIN_FILENO);
dup2(fd_out, STDOUT_FILENO);


6. Output Example

Example of running a normal command:
minishell> ls -l

Running a command in background:
minishell> ping google.com &
[1234] ping google.com

Listing background jobs:
minishell> jobs
[1234] ping google.com

Using piping and redirection:
minishell> ls | grep cpp > output.txt

7. Conclusion

This project successfully demonstrates how a shell works internally by interacting with the operating system using system calls. The implementation helps understand:

  -Process creation and termination
  -Foreground and background process execution
  -Handling input/output redirection
  -Piping between processes
  -Managing multiple running jobs

Through this project, significant knowledge was gained about low-level system programming and Linux process management.

