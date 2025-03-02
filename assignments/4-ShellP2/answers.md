1. Can you think of why we use `fork/execvp` instead of just calling `execvp` directly? What value do you think the `fork` provides?

    > **Answer**:  This is because fork creates a new process, the child, and execvp() is used to replace the child process’s memory with a new program. We use fork() because it helps separate processes. The parent can continue running, manage child processes, and maintain control over the execution flow.

2. What happens if the fork() system call fails? How does your implementation handle this scenario?

    > **Answer**:  In my implementation, when fork() system call fails, it returns a value less than 0 (-1), so the program does not create any child process and exits the exec_cmd() function right away.

3. How does execvp() find the command to execute? What system environment variable plays a role in this process?

    > **Answer**:  execvp() uses the PATH variable to find the command to be executed.

4. What is the purpose of calling wait() in the parent process after forking? What would happen if we didn’t call it?

    > **Answer**:  Calling wait() is to get the exit status of the child process and let the parent know about the status of its execution. If we didn't call it, the parent wouldn't know if the child is terminated or still running, and would eventually become a zombie process.

5. In the referenced demo code we used WEXITSTATUS(). What information does this provide, and why is it important?

    > **Answer**: WEXITSTATUS() is a macro in the standard library which contains the exit status of the child process which exits normally without any signals or interruptions. This is important because it provided the parent with the information whether it exited normally or if there we errors.

6. Describe how your implementation of build_cmd_buff() handles quoted arguments. Why is this necessary?

    > **Answer**:  For my implementation, I loop through the command provided after removing all the leading, trailing and duplicate spaces outside quotes, while looping, search for spaces outside quotes which means that is the end of a word. I store this word in the cmd_buff_t struct provided and continue until the end. If I encounter a quote, I have a flag to remember that and the position of the starting quote, and when I find the closing quote, I replace it with '\0' and add this new word to cmd_argv[] in cmd_buff_t struct. This is important because I need to account for quotes and spaces inside them shouldn't be changed, so that I can store the characters in quotes as one single argument.

7. What changes did you make to your parsing logic compared to the previous assignment? Were there any unexpected challenges in refactoring your old code?

    > **Answer**:  I had to not use strtok() as I did before and manually go through the command to split it up into arguments due to the quotes. I had a lot of difficulty while splitting the command into arguments and storing them properly. I kept getting segmentation faults or the arguments weren't getting stored properly or missing. But I finally did it after trying multiple approaches.

8. For this quesiton, you need to do some research on Linux signals. You can use [this google search](https://www.google.com/search?q=Linux+signals+overview+site%3Aman7.org+OR+site%3Alinux.die.net+OR+site%3Atldp.org&oq=Linux+signals+overview+site%3Aman7.org+OR+site%3Alinux.die.net+OR+site%3Atldp.org&gs_lcrp=EgZjaHJvbWUyBggAEEUYOdIBBzc2MGowajeoAgCwAgA&sourceid=chrome&ie=UTF-8) to get started.

- What is the purpose of signals in a Linux system, and how do they differ from other forms of interprocess communication (IPC)?

    > **Answer**:  Signals are used as simple interrupts to end a process, pause or resume a process. They differ from other forms of IPC because they carry little to no data and are not bi-directional.

- Find and describe three commonly used signals (e.g., SIGKILL, SIGTERM, SIGINT). What are their typical use cases?

    > **Answer**: - SIGTERM: This is used to terminate a process gracefully. This can be initiated by the system itself or other processes, and typically gives process the freedom of how to handle the signal.
    - SIGINT: This is typically initiated by the user when they press an interrupt key combination (Ctrl + C). This is used to stop a long-running program, and also allows the process to handle necessary cleanup before exit.
    - SIGKILL: This is used to terminate a process forcefully. This terminates the process immediately and doesn't allow for any cleanup. This is used when SIGTERM cannot be used, like when a page becomes unresponsive, and does not allow the process to save its state or resources.

- What happens when a process receives SIGSTOP? Can it be caught or ignored like SIGINT? Why or why not?

    > **Answer**:  SIGSTOP causes the process to immediately pause, typically done by the operating system. This cannot be caught or ignored like SIGINT because it is intended to be forceful that always pauses execution no matter what.