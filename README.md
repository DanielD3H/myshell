# Roadmap: Building PosixShell

Here is the roadmap to build your shell, **"PosixShell,"** organized by complexity to gradually introduce each POSIX concept.

---

### Phase 1: The Skeleton (Processes & Basic I/O)
**Goal:** Create a REPL (Read-Eval-Print Loop) that can launch standard programs like `ls`, `grep`, or `sleep`.

1.  **The Loop:** Create an infinite loop that prints a prompt (e.g., `myshell> `).
2.  **Input:** Use `getline()` to read a full line of text from `stdin`.
3.  **Parsing:** Split the string into arguments (tokens) using `strtok()` or `strsep()`.
4.  **Execution (The "Fork-Exec" Pattern):**
    * **Fork:** Call `fork()` to create a duplicate of your shell process.
    * **Child Process:** Inside the child (where `pid == 0`), use `execvp()` to replace the current process image with the command the user typed.
    * **Parent Process:** Inside the parent (where `pid > 0`), use `waitpid()` to pause execution until the child finishes.



**Key System Calls:** `fork`, `execvp`, `waitpid`, `getline`.

---

### Phase 2: Redirection (Filesystem & File Descriptors)
**Goal:** Allow users to save output to files (`ls > out.txt`) or read input from files (`wc < in.txt`).

1.  **File Descriptors (FDs):** Understand that `0` is stdin, `1` is stdout, and `2` is stderr.
2.  **Parsing Symbols:** Detect `>` and `<` in the user input.
3.  **Manipulation:** Before calling `execvp` in the child process:
    * Open the target file using `open()`.
    * Use `dup2()` to replace the standard FD (1 or 0) with your file's FD.
    * Close the original file FD to avoid leaks.

**Key System Calls:** `open`, `close`, `dup2`, `strchr`.

---

### Phase 3: Signal Handling (Signals)
**Goal:** Ensure `Ctrl+C` kills the running command but **not** your shell.

1.  **The Problem:** Currently, if you press `Ctrl+C`, the kernel sends `SIGINT` to the shell process group, killing your shell.
2.  **The Handler:** Write a function that handles `SIGINT`.
3.  **Registration:** Use `sigaction()` (preferred over `signal()`) to register this handler.
4.  **Logic:**
    * If a child process is running, forward the signal to it (or rely on the process group logic).
    * If no child is running, the shell should just print a new line and prompt again.

**Key System Calls:** `sigaction`, `kill`, `signal`.

---

### Phase 4: Persistent History (Memory Mapping)
**Goal:** Implement a command history that survives after you close the shell, using memory mapping instead of standard file I/O.

1.  **The File:** Create a fixed-size file (e.g., `.myshell_history`) on disk.
2.  **Mapping:** Use `mmap()` to map this file directly into your process's memory space. This treats the file like an array in memory.
3.  **Reading/Writing:** When the user types a command, `memcpy` it into the mapped memory region. It automatically syncs to disk.
4.  **Benefits:** This teaches you how the OS manages virtual memory and allows for very fast file access.

**Key System Calls:** `mmap`, `munmap`, `msync`, `lseek`, `ftruncate`.

---

### Phase 5: The "Remote" Shell (Sockets)
**Goal:** Execute commands on your shell from a different terminal (or machine) via TCP.

1.  **The Server:** Create a built-in command (e.g., `listen 8080`) that starts a TCP server.
2.  **Connection:** When a client connects (you can use `telnet` or `nc` to test), the shell accepts the connection.
3.  **Redirection Redux:** Use `dup2()` to redirect the shell's `stdin` and `stdout` to the *socket* file descriptor instead of the terminal.
4.  **Execution:** Now, commands sent over the network drive your shell.

**Key System Calls:** `socket`, `bind`, `listen`, `accept`, `connect`.

---

### Phase 6: Async Task Monitor (Threads)
**Goal:** Monitor the health of background processes without blocking the main prompt.

*Note: Shells typically use processes for background jobs (`&`), but we will use threads here to fulfill your requirement.*

1.  **The Monitor:** Create a detached POSIX thread (`pthread`) when the shell starts.
2.  **Shared State:** The main thread adds PIDs of background jobs to a global list (protected by a mutex).
3.  **The Worker:** The thread wakes up every few seconds to check `/proc/[pid]/status` or use `waitpid` with the `WNOHANG` flag to see if jobs have finished.
4.  **Notification:** If a job finishes, the thread prints "Job [PID] done" to the screen asynchronously.

**Key Functions:** `pthread_create`, `pthread_mutex_lock/unlock`, `pthread_detach`.

---

### Project Summary

| Phase | Core Concept | Key Functionality |
| :--- | :--- | :--- |
| **1** | Processes | `fork`, `exec`, parsing input |
| **2** | Filesystem/IO | `dup2`, `open`, `>`, `<` |
| **3** | Signals | `sigaction`, capturing `SIGINT` |
| **4** | Memory Mapping | `mmap` based command history |
| **5** | Sockets | Remote control via TCP |
| **6** | Threads | Background job monitoring thread |