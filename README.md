# C++ Project: Operating System Disk Simulation (inode)

## Introduction

This project offers a lightweight implementation of a UNIX-based filesystem simulation (inode). The simulator utilizes three direct blocks, one single-indirect block, and one double-indirect block.

## About

This simulator provides a range of functionalities, including:

- Disk (re)formatting.
- Creation of various files.
- Reading from and writing to multiple files.
- Copying and deleting files within the filesystem.

## Structure

- `main.cpp`: Contains the main function definition, enabling users to format the disk, create files, write, read, delete, or copy files.
- `fsInode.cpp`: Defines the class responsible for a single file in the filesystem, storing specific file details such as block locations.
- `FileDescriptor.cpp`: Manages the linkage between a file and its name, handling file-related details like open/closed status and name.
- `fsDisk.cpp`: Represents the filesystem's disk, facilitating operations such as writing, reading, formatting, and managing different blocks and internal fragmentation.

## The Algorithm

The program adheres to the following algorithm:

1. Receive a basic command (e.g., create a file, write to a file) from the user.
2. Validate the feasibility of the command (e.g., preventing writing to a closed file, checking disk capacity).
3. Handle internal fragmentation and assign the next available block for write operations.
4. In case of errors (e.g., full disk, full inode, illegal command), return -1.

## Remarks:

- The simulator accounts for internal fragmentation.
- The simulator enforces Linux-like restrictions on permissible commands (e.g., disallowing deletion of an opened file).

## Getting Started

To compile and run the project, follow these steps:

1. Clone the repository or download the source code.
2. Navigate to the project directory.
3. Compile the project using a C++ compiler (e.g., g++): `g++ main.cpp fsInode.cpp FileDescriptor.cpp fsDisk.cpp -o simulator`
4. Run the compiled executable: `./simulator`

## Examples

### Creating files:

![Creating files](https://github.com/OmriNaor/Disk_Simulation/assets/106623821/297fe0df-82fd-4209-b94f-a914b624273e)

### Writing into a file and handling internal fragmentation:

![Writing (fragmentation included)](https://github.com/OmriNaor/Disk_Simulation/assets/106623821/cb68b090-746f-416d-a00d-cd9ce6ab52fa)

### Reading from a file:

![Reading](https://github.com/OmriNaor/Disk_Simulation/assets/106623821/f7e196bc-21f9-4b36-8818-78cd5c0a88d5)

### Renaming a file:

![Rename](https://github.com/OmriNaor/Disk_Simulation/assets/106623821/f1bc8dc4-a337-4c74-8638-fa0062e123df)

### Copying a file:

![Copied](https://github.com/OmriNaor/Disk_Simulation/assets/106623821/8edabd4e-0de4-428c-a54d-f1ce222ae4c0)
