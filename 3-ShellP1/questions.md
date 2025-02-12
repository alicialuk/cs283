1. In this assignment I suggested you use `fgets()` to get user input in the main while loop. Why is `fgets()` a good choice for this application?

    > **Answer**:  fgets() is a good choice for this application due to it being "lines of input" based. fgets() reads up to one less than size characters or until a newline or EOF. It always includes the newlien if it is read and then always adds a null terminator.

2. You needed to use `malloc()` to allocte memory for `cmd_buff` in `dsh_cli.c`. Can you explain why you needed to do that, instead of allocating a fixed-size array?

    > **Answer**:  We need to use malloc() because SH_CMD_MAX is large and stacked-allocated arrays can cause stack overflow. Heap allocation with malloc is safer.


3. In `dshlib.c`, the function `build_cmd_list(`)` must trim leading and trailing spaces from each command before storing it. Why is this necessary? If we didn't trim spaces, what kind of issues might arise when executing commands in our shell?

    > **Answer**:  We need to trim leading and trailing spaces from each command before storing it because it can cause command lookup failures. The system may try to read the spaces as its own command or not properly register the commands following the spaces.

4. For this question you need to do some research on STDIN, STDOUT, and STDERR in Linux. We've learned this week that shells are "robust brokers of input and output". Google _"linux shell stdin stdout stderr explained"_ to get started.

- One topic you should have found information on is "redirection". Please provide at least 3 redirection examples that we should implement in our custom shell, and explain what challenges we might have implementing them.

    > **Answer**: One redirection example that should be implemented is 'cat <input.txt' which reads from input.txt and writes to the terminal. A second example is 'cat    >output.txt' which reads from the terminal and write to output.txt. A third example is 'cat <input.txt >output.txt' which reads from input.txt and writes to output.txt. One challenge that we may face when implementing these redirection examples is the parsing complexity as it needs to accurately handle multiple redirections in one command as with the 3rd example and the system has to knpw how to handle spaces within the command. Another challenge is teaching the system how to handle errors if it can't open a file or something of that sort because of permissions. 


- You should have also learned about "pipes". Redirection and piping both involve controlling input and output in the shell, but they serve different purposes. Explain the key differences between redirection and piping.

    > **Answer**:  While redirection connects a command with files, pipes connect the output of one command to the input of another command. When you use pipes, it doesn't go through the filesystem which is why it doesn't use the redirection operators.

- STDERR is often used for error messages, while STDOUT is for regular output. Why is it important to keep these separate in a shell?

    > **Answer**:  It is important to keep these separate in a shell because you need to seperate error logs from the normal output. This can cause a lot of confusion for the user if these two are outputted in the same terminal. 

- How should our custom shell handle errors from commands that fail? Consider cases where a command outputs both STDOUT and STDERR. Should we provide a way to merge them, and if so, how?

    > **Answer**:  When encountering a nonexistent command, an error message should be immediately displayed on the terminal. From there we should offer redirection options. We should provide a way to merge commands which output to both STDOUT and STDERR. This would involve two different print functions so that the command can output to each respective file.
