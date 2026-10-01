# Custom Unix Shell

A lightweight Unix-style shell written in C that demonstrates core operating-system concepts including process creation, command execution, piping, and input handling.

## Features

- Executes external programs using `fork()` and `execvp()`
- Built-in commands:
  - `cd`
  - `help`
  - `about`
  - `exit`
- Command piping with `|`
- Sequential command execution with `;`
- Multi-process command execution with `&`
- Input validation and length checking
- Dynamic command-line parsing

## Build

Compile using GCC:

```bash
gcc main.c -o shell
```
## Run
```bash
./shell
```
