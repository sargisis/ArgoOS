// ATA (Advanced Technology Attachment) Driver
// Hard disk driver for reading/writing sectors

#ifndef ATA_H
#define ATA_H

#include "types.h"

// ATA ports
#define ATA_PRIMARY_DATA       0x1F0
#define ATA_PRIMARY_ERROR      0x1F1
#define ATA_PRIMARY_SECTOR     0x1F2
#define ATA_PRIMARY_LBA_LOW    0x1F3
#define ATA_PRIMARY_LBA_MID    0x1F4
#define ATA_PRIMARY_LBA_HIGH   0x1F5
#define ATA_PRIMARY_DEVICE     0x1F6
#define ATA_PRIMARY_COMMAND    0x1F7
#define ATA_PRIMARY_STATUS     0x1F7
#define ATA_PRIMARY_CONTROL    0x3F6

// ATA commands
#define ATA_CMD_READ_PIO       0x20
#define ATA_CMD_READ_PIO_EXT   0x24
#define ATA_CMD_WRITE_PIO      0x30
#define ATA_CMD_WRITE_PIO_EXT  0x34
#define ATA_CMD_IDENTIFY       0xEC

// ATA status bits
#define ATA_STATUS_ERR         0x01
#define ATA_STATUS_DRQ         0x08  // Data Request Ready
#define ATA_STATUS_DF          0x20  // Drive Fault
#define ATA_STATUS_BSY         0x80  // Busy

// Sector size (512 bytes)
#define ATA_SECTOR_SIZE 512

// Initialize ATA driver
void ata_init(void);

// Read sectors from disk
int ata_read_sectors(uint32_t lba, uint8_t num_sectors, void* buffer);

// Write sectors to disk
int ata_write_sectors(uint32_t lba, uint8_t num_sectors, const void* buffer);

// Identify ATA device
int ata_identify(void);

#endif // ATA_H
