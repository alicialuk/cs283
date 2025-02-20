1. Can you think of why we use `fork/execvp` instead of just calling `execvp` directly? What value do you think the `fork` provides?

    > **Answer**: We use fork/execvp() together in a shell because this mechanism allows Unix shells to execute new programs by first forking a child process and then using exec() to run the desired binary while the parent process waits for the child to complete using wait(). Calling execvp would directly replace the shell process thus terminating it. When a process calls fork(), the operating system creates a new child process than is an exact copy of the parent, inheriting its memory, file descriptors, and execution state.

2. What happens if the fork() system call fails? How does your implementation handle this scenario?

    > **Answer**:  If the fork() system call fails, it means that the shell cannot create a new child process. In my file, an error message is printed and it also returns an error code.

3. How does execvp() find the command to execute? What system environment variable plays a role in this process?

    > **Answer**:  execvp() looks for the command in each directory specified in the path environment variable and it then searches these directories. The system environment variable that plays a role in the process is the PATH variable.

4. What is the purpose of calling wait() in the parent process after forking? What would happen if we didn’t call it?

    > **Answer**:  The purpose of calling wait() in the parent process after forking is to ensure that the parent process waits for the child process to complete its execution. If we don’t call it the parents process may not wait for the child process to complete its execution.

5. In the referenced demo code we used WEXITSTATUS(). What information does this provide, and why is it important?

    > **Answer**: WEEXITSTATUS() returns the exit code of the child process. It is used to extract the exit status of a child process that has terminated. It is important because it provides info about how the child process ended with its exit code.

6. Describe how your implementation of build_cmd_buff() handles quoted arguments. Why is this necessary?

    > **Answer**:  build_cmd_buff() handles quoted arguments by treating them as a single argument. It does not touch the spaces within the quotes. This is necessary in order to correctly interpret the commands.

7. What changes did you make to your parsing logic compared to the previous assignment? Were there any unexpected challenges in refactoring your old code?

    > **Answer**:  In the previous assignment, the command line was split based on pipes while in this assignment, the command line was read using gets and the command buffer was then parsed into arguments.

8. For this quesiton, you need to do some research on Linux signals. You can use [this google search](https://www.google.com/search?q=Linux+signals+overview+site%3Aman7.org+OR+site%3Alinux.die.net+OR+site%3Atldp.org&oq=Linux+signals+overview+site%3Aman7.org+OR+site%3Alinux.die.net+OR+site%3Atldp.org&gs_lcrp=EgZjaHJvbWUyBggAEEUYOdIBBzc2MGowajeoAgCwAgA&sourceid=chrome&ie=UTF-8) to get started.

- What is the purpose of signals in a Linux system, and how do they differ from other forms of interprocess communication (IPC)?

    > **Answer**:  The purpose of signals is that each has a current disposition which determines how the process behaves when it is delivered the signal. Signals differ from other forms of IPC because they can interrupt processes at any time while IPC methods are synchornous. Along with this signals use signal handlers while other IPC methods involve explicit read or write operations.

- Find and describe three commonly used signals (e.g., SIGKILL, SIGTERM, SIGINT). What are their typical use cases?

    > **Answer**:  SIGILL is when there is an illegal instruction. A use case would be when a process tries to execute an invalid instruction. SIGPIPE is when there is a broken pipe. A use case would be when a process write to a pipe with no readers. SIGSYS is a bad system call. A use case would be if the process makes an invalid system call.

- What happens when a process receives SIGSTOP? Can it be caught or ignored like SIGINT? Why or why not?

    > **Answer**:  What happens when a process receives SIGSTOP? Can it be caught or ignored like SIGINT? Why or why not?
When a process receives SIGSTOP, it is immediately stopped by the operating system and it cannot be caught or ignored. This is because it is designed to ensure that the process would be paused no matter what. 
