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
            "entry_size": Path(descriptor[0]).stat().st_size,  
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

    write_short(len(descriptors))
    write_short(int(sum(entry["sector_count"] for entry in entries)) + int(len(entries) / 8))
    # len(entries) / 8 in case we have more than 8
    for _ in range(0, 512 - 4):
        write_byte(0)

    current_write = 0
    # Allocate the sectors and write the entries
    for entry in entries:
        entry["sector_start"] = current_write
        current_write += entry["sector_count"]
        
        # Write identifier
        for c in entry["identifier"]:
            write_byte(ord(c))

        write_int(entry["sector_start"])
        write_int(entry["sector_count"])
        write_int(entry["entry_size"])
        write_byte(entry["checksum"])
        for _ in range(0, 19):
            write_byte(0)

    # Write the actual file data
    for entry in entries:
        for b in entry["data"]:
            write_byte(b)

        padding_required = entry["entry_size"] % 512
        for _ in range(0, padding_required):
            write_byte(0)


if __name__ == "__main__":
    args = sys.argv
    args.pop(0)
    write_file_descriptors(args);
