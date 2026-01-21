// Simple In-Memory File System Implementation

#include "fs.h"
#include "ata.h"
#include "heap.h"
#include "string.h"
#include "vga.h"
#include "serial.h"

#define MAX_FILE_DESCRIPTORS 64
#define MAX_FILES 256
#define MAX_DIRS 64
#define MAX_FILE_SIZE 65536  // 64KB max file size
#define MAX_PATH_LENGTH 256

// In-memory file structure
struct inode {
    char name[256];
    uint32_t type;          // FS_TYPE_FILE or FS_TYPE_DIR
    uint32_t size;          // File size
    void* data;             // File data (for files) or first child (for dirs)
    int valid;              // Is this inode valid?
    struct inode* parent;   // Parent directory
    struct inode* next;     // Next sibling in directory
    uint32_t inode_num;     // Inode number
};

static struct file_descriptor file_descriptors[MAX_FILE_DESCRIPTORS];
static struct inode inodes[MAX_FILES + MAX_DIRS];
static struct inode* root_dir = NULL;
static struct inode* current_dir = NULL;
static int fs_initialized = 0;
static uint32_t next_inode = 1;

// Helper functions
static struct inode* fs_find_inode_in_dir(struct inode* dir, const char* name);
static struct inode* fs_create_inode(const char* name, uint32_t type, struct inode* parent);
static struct inode* fs_resolve_path(const char* path);

// Get current directory
void* fs_get_current_dir(void) {
    return (void*)(current_dir ? current_dir : root_dir);
}

// Set current directory
int fs_set_current_dir(void* dir_ptr) {
    struct inode* dir = (struct inode*)dir_ptr;
    if (dir && dir->type == FS_TYPE_DIR) {
        current_dir = dir;
        return 0;
    }
    return -1;
}

// Get current directory path
void fs_get_current_path(char* path, uint32_t max_len) {
    (void)max_len; // For now, we assume path is large enough
    if (!current_dir || current_dir == root_dir) {
        strcpy(path, "/");
        return;
    }
    
    // Build path by traversing up
    char temp_path[MAX_PATH_LENGTH] = {0};
    struct inode* node = current_dir;
    int pos = MAX_PATH_LENGTH - 1;
    temp_path[pos] = '\0';
    
    while (node && node != root_dir) {
        int name_len = strlen(node->name);
        pos -= name_len;
        if (pos < 0) break;
        memcpy(temp_path + pos, node->name, name_len);
        pos--;
        if (pos >= 0) temp_path[pos] = '/';
        node = node->parent;
    }
    
    if (pos < 0) {
        strcpy(path, "/");
    } else {
        strcpy(path, temp_path + pos);
    }
}

int fs_init(void) {
    if (fs_initialized) {
        return 0;
    }
    
    serial_puts("fs_init: Starting...\n");
    
    // Initialize file descriptors
    for (int i = 0; i < MAX_FILE_DESCRIPTORS; i++) {
        file_descriptors[i].valid = 0;
    }
    
    // Initialize inodes
    for (int i = 0; i < MAX_FILES + MAX_DIRS; i++) {
        inodes[i].valid = 0;
        inodes[i].data = NULL;
        inodes[i].parent = NULL;
        inodes[i].next = NULL;
    }
    
    // Try to load from disk first
    if (fs_load() == 0) {
        serial_puts("fs_init: Loaded filesystem from disk\n");
        fs_initialized = 1;
        return 0;
    }
    
    // If load failed, create new filesystem
    serial_puts("fs_init: Creating new filesystem\n");
    root_dir = fs_create_inode("/", FS_TYPE_DIR, NULL);
    current_dir = root_dir;
    
    serial_puts("fs_init: In-memory file system initialized\n");
    
    fs_initialized = 1;
    serial_puts("fs_init: Done\n");
    return 0;
}

static struct inode* fs_create_inode(const char* name, uint32_t type, struct inode* parent) {
    // Find free inode
    for (int i = 0; i < MAX_FILES + MAX_DIRS; i++) {
        if (!inodes[i].valid) {
            struct inode* node = &inodes[i];
            node->valid = 1;
            node->inode_num = next_inode++;
            strncpy(node->name, name, 255);
            node->name[255] = '\0';
            node->type = type;
            node->size = 0;
            node->parent = parent;
            node->next = NULL;
            
            if (type == FS_TYPE_FILE) {
                node->data = kmalloc(MAX_FILE_SIZE);
                if (!node->data) {
                    node->valid = 0;
                    return NULL;
                }
                memset(node->data, 0, MAX_FILE_SIZE);
            } else {
                node->data = NULL; // Directory - will point to first child
            }
            
            // Add to parent directory
            if (parent && parent->type == FS_TYPE_DIR) {
                node->next = (struct inode*)parent->data;
                parent->data = node;
            }
            
            return node;
        }
    }
    return NULL;
}

static struct inode* fs_find_inode_in_dir(struct inode* dir, const char* name) {
    if (!dir || dir->type != FS_TYPE_DIR) {
        return NULL;
    }
    
    struct inode* child = (struct inode*)dir->data;
    while (child) {
        if (strcmp(child->name, name) == 0) {
            return child;
        }
        child = child->next;
    }
    return NULL;
}

// Export for shell
void* fs_resolve_path_export(const char* path) {
    return (void*)fs_resolve_path(path);
}

static struct inode* fs_resolve_path(const char* path) {
    if (!path || !*path) {
        return (struct inode*)fs_get_current_dir();
    }
    
    // Absolute path
    if (path[0] == '/') {
        struct inode* node = root_dir;
        path++;
        
        if (!*path) return root_dir;
        
        // Parse path components
        char component[256];
        while (*path) {
            // Skip slashes
            while (*path == '/') path++;
            if (!*path) break;
            
            // Get component
            int i = 0;
            while (*path && *path != '/' && i < 255) {
                component[i++] = *path++;
            }
            component[i] = '\0';
            
            if (strcmp(component, ".") == 0) {
                continue;
            } else if (strcmp(component, "..") == 0) {
                node = node->parent ? node->parent : root_dir;
            } else {
                node = fs_find_inode_in_dir(node, component);
                if (!node) return NULL;
            }
        }
        return node;
    } else {
        // Relative path
        struct inode* node = (struct inode*)fs_get_current_dir();
        
        // Handle "." and ".."y
        if (strcmp(path, ".") == 0) {
            return node;
        }
        
        if (strcmp(path, "..") == 0) {
            return node->parent ? node->parent : root_dir;
        }
        
        // Parse relative path components
        char component[256];
        const char* p = path;
        
        while (*p) {
            // Skip slashes
            while (*p == '/') p++;
            if (!*p) break;
            
            // Get component
            int i = 0;
            while (*p && *p != '/' && i < 255) {
                component[i++] = *p++;
            }
            component[i] = '\0';
            
            if (strcmp(component, ".") == 0) {
                continue;
            } else if (strcmp(component, "..") == 0) {
                node = node->parent ? node->parent : root_dir;
            } else {
                node = fs_find_inode_in_dir(node, component);
                if (!node) return NULL;
            }
        }
        
        return node;
    }
}

int fs_open(const char* path, uint32_t mode) {
    if (!fs_initialized || !path) {
        return -1;
    }
    
    // Find or create file
    struct inode* node = fs_resolve_path(path);
    
    // If file doesn't exist and we're writing, create it
    if (!node && (mode & FS_MODE_WRITE)) {
        // Extract filename from path
        const char* filename = path;
        const char* last_slash = path;
        while (*path) {
            if (*path == '/') last_slash = path + 1;
            path++;
        }
        filename = last_slash;
        
        struct inode* parent = fs_get_current_dir();
        if (path[0] == '/') {
            // Absolute path - need to resolve parent
            // Simplified: use current dir for now
        }
        
        node = fs_create_inode(filename, FS_TYPE_FILE, parent);
        if (!node) return -1;
    }
    
    if (!node || node->type != FS_TYPE_FILE) {
        return -1;
    }
    
    // Find free file descriptor
    int fd = -1;
    for (int i = 0; i < MAX_FILE_DESCRIPTORS; i++) {
        if (!file_descriptors[i].valid) {
            fd = i;
            break;
        }
    }
    
    if (fd == -1) {
        return -1;
    }
    
    file_descriptors[fd].valid = 1;
    file_descriptors[fd].inode = node->inode_num;
    file_descriptors[fd].position = (mode & FS_MODE_APPEND) ? node->size : 0;
    file_descriptors[fd].mode = mode;
    file_descriptors[fd].size = node->size;
    
    return fd;
}

int fs_close(int fd) {
    if (fd < 0 || fd >= MAX_FILE_DESCRIPTORS) {
        return -1;
    }
    
    if (!file_descriptors[fd].valid) {
        return -1;
    }
    
    file_descriptors[fd].valid = 0;
    return 0;
}

int32_t fs_read(int fd, void* buffer, uint32_t size) {
    if (fd < 0 || fd >= MAX_FILE_DESCRIPTORS || !buffer) {
        return -1;
    }
    
    if (!file_descriptors[fd].valid) {
        return -1;
    }
    
    if (!(file_descriptors[fd].mode & FS_MODE_READ)) {
        return -1;
    }
    
    // Find inode
    struct inode* node = NULL;
    for (int i = 0; i < MAX_FILES + MAX_DIRS; i++) {
        if (inodes[i].valid && inodes[i].inode_num == file_descriptors[fd].inode) {
            node = &inodes[i];
            break;
        }
    }
    
    if (!node || node->type != FS_TYPE_FILE) {
        return -1;
    }
    
    // Calculate how much to read
    uint32_t available = node->size - file_descriptors[fd].position;
    if (size > available) {
        size = available;
    }
    
    if (size == 0) {
        return 0;
    }
    
    // Copy data
    memcpy(buffer, (char*)node->data + file_descriptors[fd].position, size);
    file_descriptors[fd].position += size;
    
    return size;
}

int32_t fs_write(int fd, const void* buffer, uint32_t size) {
    if (fd < 0 || fd >= MAX_FILE_DESCRIPTORS || !buffer) {
        return -1;
    }
    
    if (!file_descriptors[fd].valid) {
        return -1;
    }
    
    if (!(file_descriptors[fd].mode & FS_MODE_WRITE)) {
        return -1;
    }
    
    // Find inode
    struct inode* node = NULL;
    for (int i = 0; i < MAX_FILES + MAX_DIRS; i++) {
        if (inodes[i].valid && inodes[i].inode_num == file_descriptors[fd].inode) {
            node = &inodes[i];
            break;
        }
    }
    
    if (!node || node->type != FS_TYPE_FILE) {
        return -1;
    }
    
    // Check if we need to expand
    uint32_t new_size = file_descriptors[fd].position + size;
    if (new_size > MAX_FILE_SIZE) {
        size = MAX_FILE_SIZE - file_descriptors[fd].position;
        new_size = MAX_FILE_SIZE;
    }
    
    // Copy data
    memcpy((char*)node->data + file_descriptors[fd].position, buffer, size);
    file_descriptors[fd].position += size;
    
    if (new_size > node->size) {
        node->size = new_size;
        file_descriptors[fd].size = new_size;
    }
    
    return size;
}

int32_t fs_seek(int fd, int32_t offset, uint32_t whence) {
    if (fd < 0 || fd >= MAX_FILE_DESCRIPTORS) {
        return -1;
    }
    
    if (!file_descriptors[fd].valid) {
        return -1;
    }
    
    uint32_t new_position;
    switch (whence) {
        case 0: // SEEK_SET
            new_position = offset;
            break;
        case 1: // SEEK_CUR
            new_position = file_descriptors[fd].position + offset;
            break;
        case 2: // SEEK_END
            new_position = file_descriptors[fd].size + offset;
            break;
        default:
            return -1;
    }
    
    if (new_position > file_descriptors[fd].size) {
        new_position = file_descriptors[fd].size;
    }
    
    file_descriptors[fd].position = new_position;
    return new_position;
}

uint32_t fs_size(int fd) {
    if (fd < 0 || fd >= MAX_FILE_DESCRIPTORS) {
        return 0;
    }
    
    if (!file_descriptors[fd].valid) {
        return 0;
    }
    
    return file_descriptors[fd].size;
}

int fs_readdir(const char* path, struct dirent* entries, uint32_t max_entries) {
    if (!entries || max_entries == 0) {
        return -1;
    }
    
    // Resolve path (handles ".", "..", empty path, etc.)
    struct inode* dir = fs_resolve_path(path);
    if (!dir || dir->type != FS_TYPE_DIR) {
        return -1;
    }
    
    uint32_t count = 0;
    struct inode* child = (struct inode*)dir->data;
    
    while (child && count < max_entries) {
        strncpy(entries[count].name, child->name, 255);
        entries[count].name[255] = '\0';
        entries[count].inode = child->inode_num;
        entries[count].type = child->type;
        entries[count].size = child->size;
        count++;
        child = child->next;
    }
    
    return count;
}

int fs_exists(const char* path) {
    struct inode* node = fs_resolve_path(path);
    return (node != NULL) ? 1 : 0;
}

uint32_t fs_type(const char* path) {
    struct inode* node = fs_resolve_path(path);
    if (!node) {
        return 0;
    }
    return node->type;
}

// Additional functions for shell commands
int fs_mkdir(const char* path) {
    if (!path) return -1;
    
    const char* dirname = path;
    const char* last_slash = path;
    while (*path) {
        if (*path == '/') last_slash = path + 1;
        path++;
    }
    dirname = last_slash;
    
    struct inode* parent = fs_get_current_dir();
    if (path[0] == '/') {
        parent = root_dir; // Simplified
    }
    
    // Check if already exists
    if (fs_find_inode_in_dir(parent, dirname)) {
        return -1; // Already exists
    }
    
    struct inode* dir = fs_create_inode(dirname, FS_TYPE_DIR, parent);
    return dir ? 0 : -1;
}

int fs_remove(const char* path) {
    struct inode* node = fs_resolve_path(path);
    if (!node) return -1;
    
    // Can't remove root
    if (node == root_dir) return -1;
    
    // Can't remove non-empty directory (simplified - just check if it has children)
    if (node->type == FS_TYPE_DIR && node->data != NULL) {
        return -1; // Directory not empty
    }
    
    // Remove from parent
    if (node->parent) {
        struct inode** prev = (struct inode**)&node->parent->data;
        struct inode* child = *prev;
        if (child == node) {
            *prev = node->next;
        } else {
            while (child && child->next != node) {
                child = child->next;
            }
            if (child) {
                child->next = node->next;
            }
        }
    }
    
    // Free resources
    if (node->type == FS_TYPE_FILE && node->data) {
        kfree(node->data);
    }
    node->valid = 0;
    
    return 0;
}

// Disk-based filesystem format:
// Sector 0: Superblock (magic, inode count, etc.)
// Sectors 1-64: Inode table (serialized inodes)
// Sectors 65+: Data blocks

#define FS_MAGIC 0x464C4F57  // "FLOW"
#define FS_SUPERBLOCK_LBA 0
#define FS_INODE_TABLE_LBA 1
#define FS_DATA_BLOCKS_LBA 65
#define FS_MAX_INODES_ON_DISK 256

// On-disk inode structure (simplified, without pointers)
struct disk_inode {
    char name[256];
    uint32_t type;
    uint32_t size;
    uint32_t inode_num;
    uint32_t parent_inode;
    uint32_t data_lba;        // LBA where file data starts
    uint32_t valid;
};

// Superblock structure
struct superblock {
    uint32_t magic;
    uint32_t version;
    uint32_t inode_count;
    uint32_t data_start_lba;
    uint32_t reserved[124];   // Pad to 512 bytes
};

// Helper: Collect all valid inodes into array
static int collect_inodes(struct inode** out_inodes, uint32_t max_count) {
    uint32_t count = 0;
    for (int i = 0; i < MAX_FILES + MAX_DIRS && count < max_count; i++) {
        if (inodes[i].valid) {
            out_inodes[count++] = &inodes[i];
        }
    }
    return count;
}

// Helper: Find inode by number
static struct inode* find_inode_by_num(uint32_t inode_num) {
    for (int i = 0; i < MAX_FILES + MAX_DIRS; i++) {
        if (inodes[i].valid && inodes[i].inode_num == inode_num) {
            return &inodes[i];
        }
    }
    return NULL;
}

// Save filesystem to disk
int fs_save(void) {
    if (!fs_initialized) {
        return -1;
    }
    
    serial_puts("fs_save: Starting...\n");
    
    // Collect all inodes
    struct inode* inode_list[FS_MAX_INODES_ON_DISK];
    int inode_count = collect_inodes(inode_list, FS_MAX_INODES_ON_DISK);
    
    if (inode_count == 0) {
        serial_puts("fs_save: No inodes to save\n");
        return -1;
    }
    
    // Write superblock
    struct superblock sb;
    memset(&sb, 0, sizeof(sb));
    sb.magic = FS_MAGIC;
    sb.version = 1;
    sb.inode_count = inode_count;
    sb.data_start_lba = FS_DATA_BLOCKS_LBA;
    
    if (ata_write_sectors(FS_SUPERBLOCK_LBA, 1, &sb) != 0) {
        serial_puts("fs_save: Failed to write superblock\n");
        return -1;
    }
    
    // Calculate how many sectors needed for inode table
    uint32_t inode_sectors = (inode_count * sizeof(struct disk_inode) + ATA_SECTOR_SIZE - 1) / ATA_SECTOR_SIZE;
    
    // Allocate buffer for inode table
    char* inode_buffer = (char*)kmalloc(inode_sectors * ATA_SECTOR_SIZE);
    if (!inode_buffer) {
        serial_puts("fs_save: Out of memory\n");
        return -1;
    }
    memset(inode_buffer, 0, inode_sectors * ATA_SECTOR_SIZE);
    
    // Convert in-memory inodes to disk format
    struct disk_inode* disk_inodes = (struct disk_inode*)inode_buffer;
    uint32_t current_data_lba = FS_DATA_BLOCKS_LBA;
    
    for (int i = 0; i < inode_count; i++) {
        struct inode* node = inode_list[i];
        struct disk_inode* dnode = &disk_inodes[i];
        
        strncpy(dnode->name, node->name, 255);
        dnode->name[255] = '\0';
        dnode->type = node->type;
        dnode->size = node->size;
        dnode->inode_num = node->inode_num;
        dnode->valid = 1;
        
        // Find parent inode number
        if (node->parent) {
            dnode->parent_inode = node->parent->inode_num;
        } else {
            dnode->parent_inode = 0;
        }
        
        // For files, save data and record LBA
        if (node->type == FS_TYPE_FILE && node->data && node->size > 0) {
            uint32_t data_sectors = (node->size + ATA_SECTOR_SIZE - 1) / ATA_SECTOR_SIZE;
            dnode->data_lba = current_data_lba;
            
            // Write file data
            if (ata_write_sectors(current_data_lba, data_sectors, node->data) != 0) {
                serial_puts("fs_save: Failed to write file data\n");
                kfree(inode_buffer);
                return -1;
            }
            
            current_data_lba += data_sectors;
        } else {
            dnode->data_lba = 0;
        }
    }
    
    // Write inode table
    if (ata_write_sectors(FS_INODE_TABLE_LBA, inode_sectors, inode_buffer) != 0) {
        serial_puts("fs_save: Failed to write inode table\n");
        kfree(inode_buffer);
        return -1;
    }
    
    kfree(inode_buffer);
    serial_puts("fs_save: Successfully saved filesystem\n");
    return 0;
}

// Load filesystem from disk
int fs_load(void) {
    serial_puts("fs_load: Starting...\n");
    
    // Read superblock
    struct superblock sb;
    if (ata_read_sectors(FS_SUPERBLOCK_LBA, 1, &sb) != 0) {
        serial_puts("fs_load: No disk or timeout (using in-memory FS)\n");
        return -1;
    }
    
    // Check magic
    if (sb.magic != FS_MAGIC) {
        serial_puts("fs_load: Invalid magic number (no filesystem on disk)\n");
        return -1;
    }
    
    if (sb.inode_count == 0 || sb.inode_count > FS_MAX_INODES_ON_DISK) {
        serial_puts("fs_load: Invalid inode count\n");
        return -1;
    }
    
    // Calculate inode table size
    uint32_t inode_sectors = (sb.inode_count * sizeof(struct disk_inode) + ATA_SECTOR_SIZE - 1) / ATA_SECTOR_SIZE;
    
    // Read inode table
    char* inode_buffer = (char*)kmalloc(inode_sectors * ATA_SECTOR_SIZE);
    if (!inode_buffer) {
        serial_puts("fs_load: Out of memory\n");
        return -1;
    }
    
    if (ata_read_sectors(FS_INODE_TABLE_LBA, inode_sectors, inode_buffer) != 0) {
        serial_puts("fs_load: Failed to read inode table\n");
        kfree(inode_buffer);
        return -1;
    }
    
    struct disk_inode* disk_inodes = (struct disk_inode*)inode_buffer;
    
    // First pass: Create all inodes (without linking)
    for (uint32_t i = 0; i < sb.inode_count; i++) {
        struct disk_inode* dnode = &disk_inodes[i];
        if (!dnode->valid) continue;
        
        // Find free inode slot
        struct inode* node = NULL;
        for (int j = 0; j < MAX_FILES + MAX_DIRS; j++) {
            if (!inodes[j].valid) {
                node = &inodes[j];
                break;
            }
        }
        
        if (!node) {
            serial_puts("fs_load: Too many inodes\n");
            kfree(inode_buffer);
            return -1;
        }
        
        // Initialize inode
        node->valid = 1;
        node->inode_num = dnode->inode_num;
        strncpy(node->name, dnode->name, 255);
        node->name[255] = '\0';
        node->type = dnode->type;
        node->size = dnode->size;
        node->parent = NULL;  // Will link in second pass
        node->next = NULL;
        
        // Allocate data for files
        if (node->type == FS_TYPE_FILE) {
            node->data = kmalloc(MAX_FILE_SIZE);
            if (!node->data) {
                serial_puts("fs_load: Out of memory for file data\n");
                kfree(inode_buffer);
                return -1;
            }
            memset(node->data, 0, MAX_FILE_SIZE);
            
            // Load file data if it exists
            if (dnode->data_lba > 0 && node->size > 0) {
                uint32_t data_sectors = (node->size + ATA_SECTOR_SIZE - 1) / ATA_SECTOR_SIZE;
                if (ata_read_sectors(dnode->data_lba, data_sectors, node->data) != 0) {
                    serial_puts("fs_load: Failed to read file data\n");
                    kfree(inode_buffer);
                    return -1;
                }
            }
        } else {
            node->data = NULL;  // Directory - will link children in second pass
        }
        
        // Find root directory
        if (strcmp(node->name, "/") == 0 && node->type == FS_TYPE_DIR) {
            root_dir = node;
            current_dir = root_dir;
        }
    }
    
    // Second pass: Link parent-child relationships
    for (uint32_t i = 0; i < sb.inode_count; i++) {
        struct disk_inode* dnode = &disk_inodes[i];
        if (!dnode->valid) continue;
        
        struct inode* node = find_inode_by_num(dnode->inode_num);
        if (!node) continue;
        
        // Link parent
        if (dnode->parent_inode > 0) {
            node->parent = find_inode_by_num(dnode->parent_inode);
        }
        
        // Link to parent's children list (for directories)
        if (node->parent && node->parent->type == FS_TYPE_DIR) {
            node->next = (struct inode*)node->parent->data;
            node->parent->data = node;
        }
    }
    
    if (!root_dir) {
        serial_puts("fs_load: Root directory not found\n");
        kfree(inode_buffer);
        return -1;
    }
    
    kfree(inode_buffer);
    serial_puts("fs_load: Successfully loaded filesystem\n");
    return 0;
}
