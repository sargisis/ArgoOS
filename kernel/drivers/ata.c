// ATA Driver Implementation

#include "ata.h"
#include "vga.h"

// Wait for ATA device to be ready
static void ata_wait_ready(void) {
    uint8_t status;
    do {
        asm volatile("inb %1, %0" : "=a"(status) : "Nd"(ATA_PRIMARY_STATUS));
    } while (status & ATA_STATUS_BSY);
}

// Wait for data request
static void ata_wait_data(void) {
    uint8_t status;
    do {
        asm volatile("inb %1, %0" : "=a"(status) : "Nd"(ATA_PRIMARY_STATUS));
    } while (!(status & ATA_STATUS_DRQ) && !(status & ATA_STATUS_ERR));
}

void ata_init(void) {
    // Select master drive
    asm volatile("outb %0, %1" :: "a"((uint8_t)0xA0), "Nd"(ATA_PRIMARY_DEVICE));
    
    // Wait a bit
    for (volatile int i = 0; i < 1000; i++);
    
    // Clear any pending interrupts
    uint8_t dummy;
    asm volatile("inb %1, %0" : "=a"(dummy) : "Nd"(ATA_PRIMARY_STATUS));
    (void)dummy;
    
    vga_puts("ATA driver initialized\n");
}

int ata_read_sectors(uint32_t lba, uint8_t num_sectors, void* buffer) {
    if (num_sectors == 0) {
        return -1; // Invalid sector count
    }
    
    // Wait for device to be ready
    ata_wait_ready();
    
    // Select master drive and send LBA
    uint8_t device = 0xE0 | ((lba >> 24) & 0x0F);
    asm volatile("outb %0, %1" :: "a"(device), "Nd"(ATA_PRIMARY_DEVICE));
    
    // Send sector count
    asm volatile("outb %0, %1" :: "a"(num_sectors), "Nd"(ATA_PRIMARY_SECTOR));
    
    // Send LBA addresses
    asm volatile("outb %0, %1" :: "a"((uint8_t)(lba & 0xFF)), "Nd"(ATA_PRIMARY_LBA_LOW));
    asm volatile("outb %0, %1" :: "a"((uint8_t)((lba >> 8) & 0xFF)), "Nd"(ATA_PRIMARY_LBA_MID));
    asm volatile("outb %0, %1" :: "a"((uint8_t)((lba >> 16) & 0xFF)), "Nd"(ATA_PRIMARY_LBA_HIGH));
    
    // Send read command
    asm volatile("outb %0, %1" :: "a"((uint8_t)ATA_CMD_READ_PIO), "Nd"(ATA_PRIMARY_COMMAND));
    
    // Read sectors
    uint16_t* buf = (uint16_t*)buffer;
    for (uint8_t i = 0; i < num_sectors; i++) {
        // Wait for data
        ata_wait_data();
        
        // Check for errors
        uint8_t status;
        asm volatile("inb %1, %0" : "=a"(status) : "Nd"(ATA_PRIMARY_STATUS));
        if (status & ATA_STATUS_ERR) {
            return -1; // Error reading
        }
        
        // Read 256 words (512 bytes = 1 sector)
        for (int j = 0; j < 256; j++) {
            asm volatile("inw %1, %0" : "=a"(buf[j]) : "Nd"(ATA_PRIMARY_DATA));
        }
        
        buf += 256; // Move to next sector
    }
    
    return 0; // Success
}

int ata_write_sectors(uint32_t lba, uint8_t num_sectors, const void* buffer) {
    if (num_sectors == 0) {
        return -1; // Invalid sector count
    }
    
    // Wait for device to be ready
    ata_wait_ready();
    
    // Select master drive and send LBA
    uint8_t device = 0xE0 | ((lba >> 24) & 0x0F);
    asm volatile("outb %0, %1" :: "a"(device), "Nd"(ATA_PRIMARY_DEVICE));
    
    // Send sector count
    asm volatile("outb %0, %1" :: "a"(num_sectors), "Nd"(ATA_PRIMARY_SECTOR));
    
    // Send LBA addresses
    asm volatile("outb %0, %1" :: "a"((uint8_t)(lba & 0xFF)), "Nd"(ATA_PRIMARY_LBA_LOW));
    asm volatile("outb %0, %1" :: "a"((uint8_t)((lba >> 8) & 0xFF)), "Nd"(ATA_PRIMARY_LBA_MID));
    asm volatile("outb %0, %1" :: "a"((uint8_t)((lba >> 16) & 0xFF)), "Nd"(ATA_PRIMARY_LBA_HIGH));
    
    // Send write command
    asm volatile("outb %0, %1" :: "a"((uint8_t)ATA_CMD_WRITE_PIO), "Nd"(ATA_PRIMARY_COMMAND));
    
    // Write sectors
    const uint16_t* buf = (const uint16_t*)buffer;
    for (uint8_t i = 0; i < num_sectors; i++) {
        // Wait for data request
        ata_wait_data();
        
        // Check for errors
        uint8_t status;
        asm volatile("inb %1, %0" : "=a"(status) : "Nd"(ATA_PRIMARY_STATUS));
        if (status & ATA_STATUS_ERR) {
            return -1; // Error writing
        }
        
        // Write 256 words (512 bytes = 1 sector)
        for (int j = 0; j < 256; j++) {
            asm volatile("outw %0, %1" :: "a"(buf[j]), "Nd"(ATA_PRIMARY_DATA));
        }
        
        // Flush cache
        asm volatile("outb %0, %1" :: "a"((uint8_t)0xE7), "Nd"(ATA_PRIMARY_COMMAND));
        ata_wait_ready();
        
        buf += 256; // Move to next sector
    }
    
    return 0; // Success
}

int ata_identify(void) {
    // Skip ATA identify for now - it can hang if no device
    // In QEMU without disk, this will hang indefinitely
    // TODO: Add timeout mechanism
    return -1; // No device (for now, skip the check)
    
    /*
    ata_wait_ready();
    
    // Select master drive
    asm volatile("outb %0, %1" :: "a"((uint8_t)0xA0), "Nd"(ATA_PRIMARY_DEVICE));
    
    // Send identify command
    asm volatile("outb %0, %1" :: "a"((uint8_t)ATA_CMD_IDENTIFY), "Nd"(ATA_PRIMARY_COMMAND));
    
    // Wait for data
    ata_wait_data();
    
    // Read identify data (512 bytes)
    uint16_t identify_data[256];
    for (int i = 0; i < 256; i++) {
        asm volatile("inw %1, %0" : "=a"(identify_data[i]) : "Nd"(ATA_PRIMARY_DATA));
    }
    
    // Check if device exists
    if (identify_data[0] == 0) {
        return -1; // No device
    }
    
    return 0; // Device found
    */
}
