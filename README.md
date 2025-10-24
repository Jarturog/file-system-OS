# File System Implementation – Operating Systems 2

A Linux-based file system implementation project featuring a complete set of file and directory manipulation utilities with inode-based storage management.

## Table of Contents
- [Features](#features)
- [Prerequisites](#prerequisites)
- [Installation](#installation)
- [Quick Start](#quick-start)
- [Command Reference](#command-reference)
- [Usage Examples](#usage-examples)

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

3. (Optional) Clean build artifacts:
```bash
make clean
```

## Quick Start

1. **Create a virtual disk:**
```bash
./mi_mkfs virtual_disk.dat 100000
```

2. **Create a directory:**
```bash
./mi_mkdir virtual_disk.dat 6 /my_directory
```

3. **Create and write to a file:**
```bash
./mi_escribir virtual_disk.dat /my_directory/file.txt "Hello, World!" 0
```

4. **Read the file:**
```bash
./mi_cat virtual_disk.dat /my_directory/file.txt
```

## Command Reference

### System Initialization

#### `mi_mkfs`
Creates and formats a new virtual disk device.

**Syntax:**
```bash
./mi_mkfs <device_name> <num_blocks>
```

**Parameters:**
- `device_name`: Name of the virtual disk file to create
- `num_blocks`: Total number of blocks to allocate

**Example:**
```bash
./mi_mkfs virtual_disk.dat 100000
```

---

#### `leer_sf`
Reads and displays superblock information from the device.

**Syntax:**
```bash
./leer_sf <device_name>
```

**Example:**
```bash
./leer_sf virtual_disk.dat
```

---

### Low-Level File Operations

These commands work directly with inode numbers rather than file paths.

#### `escribir`
Writes data directly to one or more inodes.

**Syntax:**
```bash
./escribir <device_name> <"$(cat file)"> <inode_numbers>
```

**Parameters:**
- `device_name`: Virtual disk file
- `content`: Text content to write (use `"$(cat file)"` to write from file)
- `inode_numbers`: Space-separated list of target inodes

**Example:**
```bash
./escribir virtual_disk.dat "Test data" 5 6 7
```

---

#### `leer`
Reads and displays content from a specific inode.

**Syntax:**
```bash
./leer <device_name> <inode_number>
```

**Example:**
```bash
./leer virtual_disk.dat 5
```

---

#### `permitir`
Changes permissions on a specific inode.

**Syntax:**
```bash
./permitir <device_name> <inode_number> <permissions>
```

**Parameters:**
- `permissions`: Octal permission value (e.g., 6 for rw-, 7 for rwx)

**Example:**
```bash
./permitir virtual_disk.dat 5 7
```

---

#### `truncar`
Truncates an inode to a specific size.

**Syntax:**
```bash
./truncar <device_name> <inode_number> <num_bytes>
```

**Example:**
```bash
./truncar virtual_disk.dat 5 1024
```

---

### High-Level File Operations

These commands work with file paths, similar to standard Unix utilities.

#### `mi_escribir`
Writes text to a file at a specific offset.

**Syntax:**
```bash
./mi_escribir <disk> </file_path> <text> <offset>
```

**Parameters:**
- `offset`: Byte position where writing begins (0 for start of file)

**Example:**
```bash
./mi_escribir virtual_disk.dat /docs/readme.txt "Introduction\n" 0
```

---

#### `mi_cat`
Displays the contents of a file (similar to Unix `cat`).

**Syntax:**
```bash
./mi_cat <disk> </file_path>
```

**Example:**
```bash
./mi_cat virtual_disk.dat /docs/readme.txt
```

---

#### `mi_chmod`
Changes file or directory permissions (similar to Unix `chmod`).

**Syntax:**
```bash
./mi_chmod <disk> <permissions> </path>
```

**Parameters:**
- `permissions`: Octal permission value (0-7)

**Example:**
```bash
./mi_chmod virtual_disk.dat 6 /docs/readme.txt
```

---

#### `mi_touch`
Creates a new empty file with specified permissions.

**Syntax:**
```bash
./mi_touch <disk> <permissions> </path>
```

**Example:**
```bash
./mi_touch virtual_disk.dat 6 /docs/newfile.txt
```

---

#### `mi_cp`
Copies a file to a new location.

**Syntax:**
```bash
./mi_cp <disk> </source/name> </destination/>
```

**Example:**
```bash
./mi_cp virtual_disk.dat /docs/file.txt /backup/
```

---

#### `mi_cp_f`
Force copies a file, overwriting destination if it exists.

**Syntax:**
```bash
./mi_cp_f <disk> </source/name> </destination/>
```

**Example:**
```bash
./mi_cp_f virtual_disk.dat /docs/file.txt /backup/
```

---

#### `mi_mv`
Moves or renames a file.

**Syntax:**
```bash
./mi_mv <disk> </source/name> </destination/>
```

**Example:**
```bash
./mi_mv virtual_disk.dat /docs/oldname.txt /docs/newname.txt
```

---

#### `mi_rn`
Renames a file or directory.

**Syntax:**
```bash
./mi_rn <disk> </old_path> <new_name>
```

**Example:**
```bash
./mi_rn virtual_disk.dat /docs/file.txt newfile.txt
```

---

#### `mi_rm`
Removes a file.

**Syntax:**
```bash
./mi_rm <disk> </path>
```

**Example:**
```bash
./mi_rm virtual_disk.dat /docs/unwanted.txt
```

---

#### `mi_link`
Creates a hard link to an existing file.

**Syntax:**
```bash
./mi_link <disk> </original_file_path> </link_path>
```

**Example:**
```bash
./mi_link virtual_disk.dat /docs/original.txt /docs/link.txt
```

---

#### `mi_stat`
Displays detailed information about a file or directory (similar to Unix `stat`).

**Syntax:**
```bash
./mi_stat <disk> </path>
```

**Example:**
```bash
./mi_stat virtual_disk.dat /docs/file.txt
```

---

### Directory Operations

#### `mi_mkdir`
Creates a new directory with specified permissions.

**Syntax:**
```bash
./mi_mkdir <disk> <permissions> </path>
```

**Example:**
```bash
./mi_mkdir virtual_disk.dat 7 /projects/new_project
```

---

#### `mi_ls`
Lists directory contents.

**Syntax:**
```bash
./mi_ls <disk> </directory_path> <format>
```

**Parameters:**
- `format`: Output format option (specific formats depend on implementation)

**Example:**
```bash
./mi_ls virtual_disk.dat /docs 0
```

---

#### `mi_rm_r`
Recursively removes a directory and all its contents.

**Syntax:**
```bash
./mi_rm_r <disk> </path>
```

**Example:**
```bash
./mi_rm_r virtual_disk.dat /old_projects
```

---

## Usage Examples

### Example 1: Basic File Management

```bash
# Create a virtual disk with 50,000 blocks
./mi_mkfs mydisk.dat 50000

# Create a directory structure
./mi_mkdir mydisk.dat 7 /home
./mi_mkdir mydisk.dat 7 /home/user

# Create and write to a file
./mi_touch mydisk.dat 6 /home/user/notes.txt
./mi_escribir mydisk.dat /home/user/notes.txt "My first note" 0

# Read the file
./mi_cat mydisk.dat /home/user/notes.txt

# Copy the file
./mi_cp mydisk.dat /home/user/notes.txt /home/

# List directory contents
./mi_ls mydisk.dat /home/user 0
```

### Example 2: Working with Permissions

```bash
# Create a file with read-write permissions (6)
./mi_touch mydisk.dat 6 /private.txt

# Change to read-write-execute (7)
./mi_chmod mydisk.dat 7 /private.txt

# View file metadata
./mi_stat mydisk.dat /private.txt
```

### Example 3: Hard Links

```bash
# Create original file
./mi_touch mydisk.dat 6 /original.txt
./mi_escribir mydisk.dat /original.txt "Shared content" 0

# Create hard link
./mi_link mydisk.dat /original.txt /link.txt

# Both files reference the same inode
./mi_stat mydisk.dat /original.txt
./mi_stat mydisk.dat /link.txt
```

### Example 4: Low-Level Operations

```bash
# Create disk
./mi_mkfs testdisk.dat 10000

# Read superblock information
./leer_sf testdisk.dat

# Write directly to inode 5
./escribir testdisk.dat "Direct inode write" 5

# Read from inode 5
./leer testdisk.dat 5

# Change permissions on inode 5
./permitir testdisk.dat 5 7
```

## Known Limitations

- Single-user system (no multi-user support)
- No symbolic links (only hard links supported)
- Maximum file size depends on block size and inode structure
- No built-in defragmentation
