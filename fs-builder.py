import sys
import json
import os
import struct

from pathlib import Path

def write_byte(val):
    sys.stdout.buffer.write(struct.pack("B", val))  


def write_short(val):
    sys.stdout.buffer.write(struct.pack("<H", val))  


def write_int(val):
    sys.stdout.buffer.write(struct.pack("<I", val))  


def entry_metadata_for(descriptor):
    if not os.path.exists(descriptor[0]):
        raise ValueError(f"File {descriptor[0]} does not exist")

    if len(descriptor[1]) > 32:
        raise ValueError(f"File identifier {descriptor[1]} too large (max 32)")

    with open(descriptor[0], "rb") as f:
        data = f.read()

    checksum = 0
    for b in data:
        checksum ^= b

    return {
            "identifier": descriptor[1],
            "entry_size": len(data),  
            "sector_count": int((len(data) + 511) / 512),
            "data": data,
            "checksum": checksum,
            }


def parse_file_descriptor(descriptor):
    if len(descriptor.split(",")) != 2:
        raise ValueError(f"Invalid file descriptor: {descriptor}")

    file_path = descriptor.split(",")[0]
    identifier = descriptor.split(",")[1]
    return (file_path, identifier)


def write_file_descriptors(descriptors):
    descriptors = list(map(parse_file_descriptor, descriptors))
    entries = list(map(entry_metadata_for, descriptors))
    
    # Calculate sectors needed for entries (8 entries per sector)
    entry_sectors = (len(entries) + 7) // 8  # Ceiling division
    total_data_sectors = sum(entry["sector_count"] for entry in entries)
    
    # Write header
    write_short(len(descriptors))
    write_short(entry_sectors + total_data_sectors)
    
    # Pad rest of header sector
    for _ in range(0, 512 - 4):
        write_byte(0)
    
    # Allocate sector positions for data (after header + entry sectors)
    current_sector = 1 + entry_sectors  # Start after header and entry sectors
    
    for entry in entries:
        entry["sector_start"] = current_sector
        current_sector += entry["sector_count"]
    
    # Write entries
    for entry in entries:
        # Write identifier (pad to 32 bytes)
        identifier_bytes = entry["identifier"].encode('ascii')
        for i in range(32):
            if i < len(identifier_bytes):
                write_byte(identifier_bytes[i])
            else:
                write_byte(0)
        
        write_int(entry["sector_start"])
        write_int(entry["sector_count"])
        write_int(entry["entry_size"])
        write_byte(entry["checksum"])
        
        # Padding to complete 64-byte entry
        for _ in range(0, 19):
            write_byte(0x00)
    
    # Pad remaining entries in the last entry sector
    entries_written = len(entries)
    entries_in_last_sector = entries_written % 8
    if entries_in_last_sector != 0:
        remaining_entries = 8 - entries_in_last_sector
        for _ in range(remaining_entries):
            for _ in range(0, 64):  # 64 bytes per entry
                write_byte(0)
    
    # Write the actual file data
    for entry in entries:
        # Write file data
        for b in entry["data"]:
            write_byte(b)
        
        # Pad to sector boundary
        padding_required = 512 - (entry["entry_size"] % 512)
        if padding_required != 512:  # Only pad if not already aligned
            for _ in range(padding_required):
                write_byte(0)


if __name__ == "__main__":
    args = sys.argv
    args.pop(0)
    write_file_descriptors(args);
