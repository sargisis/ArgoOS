// File System
// Simple file system interface

#ifndef FS_H
#define FS_H

#include "types.h"

// File types
#define FS_TYPE_FILE     0x01
#define FS_TYPE_DIR      0x02

// File open modes
#define FS_MODE_READ     0x01
#define FS_MODE_WRITE    0x02
#define FS_MODE_APPEND   0x04

// File descriptor structure
struct file_descriptor {
    uint32_t inode;         // File inode number
    uint32_t position;      // Current read/write position
    uint32_t mode;          // Open mode
    uint32_t size;          // File size
    int valid;              // Is this descriptor valid?
};

// Directory entry structure
struct dirent {
    char name[256];         // File name
    uint32_t inode;         // Inode number
    uint32_t type;          // File type
    uint32_t size;          // File size
};

// Initialize file system
int fs_init(void);

// Open a file
int fs_open(const char* path, uint32_t mode);

// Close a file
int fs_close(int fd);

// Read from file
int32_t fs_read(int fd, void* buffer, uint32_t size);

// Write to file
int32_t fs_write(int fd, const void* buffer, uint32_t size);

// Seek in file
int32_t fs_seek(int fd, int32_t offset, uint32_t whence);

// Get file size
uint32_t fs_size(int fd);

// List directory
int fs_readdir(const char* path, struct dirent* entries, uint32_t max_entries);

// Check if file exists
int fs_exists(const char* path);

// Get file type
uint32_t fs_type(const char* path);

// Create directory
int fs_mkdir(const char* path);

// Remove file or directory
int fs_remove(const char* path);

// Get current directory (internal structure)
void* fs_get_current_dir(void);

// Set current directory
int fs_set_current_dir(void* dir);

// Get current directory path
void fs_get_current_path(char* path, uint32_t max_len);

// Resolve path to inode (internal, but needed for cd)
void* fs_resolve_path_export(const char* path);

// Save filesystem to disk
int fs_save(void);

// Load filesystem from disk
int fs_load(void);

#endif // FS_H
