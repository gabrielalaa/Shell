# IMCSH - Alternative Linux Shell Implementation

## Introduction

This project consists of the implementation of an alternative **Linux shell** in **C** without calling anything from the standard Linux to implement any of the commands.  
The shell, called **IMCSH**, is able to:
- Execute commands
- Manage background processes
- Redirect output  

The following document serves as **technical documentation**, describing the usability of the project, its core functionalities, and execution details, as well as how we tested the code.

---

## Code Structure

The project is structured into several functions, each handling a specific aspect of the shell:

### **1. `Init_shell()`**
- Initializes the shell and prints a welcome message.
- Displays the **current user** and **hostname** using `getlogin_r()` and `gethostname()`.
- If retrieval fails, it displays `"unknown"`.

### **2. `Display_prompt()`**
- Displays the shell prompt.
- Uses `getlogin_r()` and `gethostname()` to show: username@hostname >
- If retrieval fails, `"unknown"` is displayed.

### **3. `takeInput()`**
- Reads the command inputted by the user.
- Uses `display_prompt()` to show the prompt.
- Reads input using `fgets()`.
- Removes trailing newline characters.
- Handles input errors by setting the input to an empty string.

### **4. `parseCommand()`**
- Parses the input string into **command arguments**.
- Identifies:
- `"&"` → background execution request
- `">"` → output redirection
- `"exec"` → command execution
- Tokenizes the string using `strtok()`.
- Parameters:
- `input`: Input command string.
- `args`: Array of command arguments.
- `background`: Pointer that determines background execution.
- `outputFile`: Pointer that stores the name of the output file in case of redirection.

### **5. `executeCommand()`**
- Forks a new process to execute the command.
- Handles both:
- **Background execution**
- **Output redirection**
- Uses:
- `fork()` → Creates a new process.
- `execvp()` → Executes the command in the child process.
- `open()`, `dup2()`, `close()` → Handles output redirection.
- **Background process handling:**
- If the process is a **background process**, it prints the PID and adds it to `backgroundProcesses`.
- If **not**, the parent process **waits** for the child to terminate.
- The shell periodically checks and reports the completion of background processes using `waitpid()`.

### **6. `main()`**
- The main control function of the shell.
- **Loop** that continuously:
1. Reads input.
2. Parses input.
3. Executes commands.
- Manages **built-in commands** (`quit`, `globalusage`, `bglist`).
- Handles **exit confirmation** when background processes are still running.

---

## Built-in Commands

### **1. `quit`**
- Exits the shell.
- If background processes are running, the user is asked for confirmation before exiting.

### **2. `globalusage`**
- Displays shell usage information.

### **3. `bglist`**
- Lists **running background processes**.
- Shows:
- **Process IDs (PIDs)**
- **Commands** running in the background.

---

## How to Run the Project

To run the shell, use the following command:

```sh
./imcsh
```

---

## Notes

- This project was developed as part of an Operating Systems course at IMC University of Applied Sciences Krems.
- The project was designed without relying on standard Linux commands for implementation.
- The goal was to explore process management, command execution, and input/output handling at a low level.

---

## Contributors 

Project by:
- Alexandra-Elena Ianovici 
- Carla-Gabriela Radulescu

Under the guidance of Dr. Rubén Ruiz Torrubiano. 