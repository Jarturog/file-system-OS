# File System Implementation – Operating Systems 2

A Linux-based file system implementation project featuring a complete set of file and directory manipulation utilities with inode-based storage management. Select nivel13 for the latest version.

## Table of Contents
- [Features](#features)
- [Prerequisites](#prerequisites)
- [Installation](#installation)
- [Known Limitations](#known-limitations)
- [Command Reference](#command-reference)

## Features

- **Inode-based file system** with support for files and directories
- **Complete file operations**: create, read, write, copy, move, delete
- **Directory management**: create, list, remove (recursive)
- **Permission control**: chmod-style permission management
- **Hard links**: support for multiple references to the same inode
- **File truncation**: efficient space management
- **Low-level and high-level APIs**: both direct inode access and path-based operations

## Prerequisites

- GCC compiler
- Linux operating system
- Make utility
- Basic understanding of Unix file systems

## Installation

1. Clone the repository:
```bash
git clone <repository_url>
cd <project_directory>
```

2. Compile all utilities:
```bash
make
```

## Known Limitations

- Single-user system (no multi-user support)
- Maximum file size depends on block size and inode structure
- No built-in defragmentation

## Command Reference

All commands are documented with detailed comments in Spanish within their respective source files.
