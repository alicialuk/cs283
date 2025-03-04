1. Your shell forks multiple child processes when executing piped commands. How does your implementation ensure that all child processes complete before the shell continues accepting user input? What would happen if you forgot to call waitpid() on all child processes?

   My implementation ensures that all child processes complete before the shell continues to accept user input as it uses waitpid() on each child process. If I forgot to call waitpid() on all the processes, the processes would stop executing but still use system resources until the exit status is collected.

2. The dup2() function is used to redirect input and output file descriptors. Explain why it is necessary to close unused pipe ends after calling dup2(). What could go wrong if you leave pipes open?

   It is necessary to close unused pipe ends after calling dup2() because failing to do so would make programs in the pipeline hand indefinitely. This is because they wait for end of file conditions that will not occur. Leaving pipes open also created file descriptor leaks. 

3. Your shell recognizes built-in commands (cd, exit, dragon). Unlike external commands, built-in commands do not require execvp(). Why is cd implemented as a built-in rather than an external command? What challenges would arise if cd were implemented as an external process?

   cd was implemented as a built-in rather than an external command because if it were the opposite, each external command would runs in a separate child process with its own environment and working directory.

4. Currently, your shell supports a fixed number of piped commands (CMD_MAX). How would you modify your implementation to allow an arbitrary number of piped commands while still handling memory allocation efficiently? What trade-offs would you need to consider?

	I modified my implementation to allow an arbitrary number of piped commands by replacing the fixed-size arrays with dynamically allocated listed links or resizable arrays. Trade-offs you need to consider are the increased code complexity for memory management vs flexibility and higher runtimes for memory allocation versus the ability to handle complex pipelines. 
