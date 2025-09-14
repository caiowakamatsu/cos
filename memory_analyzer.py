import sys
import os

# --- Constants ---
# The size of a single physical page in bytes.
# As specified in the C++ struct.
PAGE_SIZE = 4096
# The total memory size to analyze in bytes (32 MB).
TOTAL_MEMORY_SIZE = 32 * 1024 * 1024
# The starting offset of the bitmap in the memory dump.
# Assuming the bitmap is at the very beginning of the file.
BITMAP_START = 0x10_0000

# --- Calculation ---
# The total number of pages in the 32 MB of memory.
# This is derived from TOTAL_MEMORY_SIZE // PAGE_SIZE.
# Note: The original code had TOTAL_PAGES = 8160. Let's recalculate for accuracy:
# 32 * 1024 * 1024 bytes / 4096 bytes/page = 8192 pages
TOTAL_PAGES = TOTAL_MEMORY_SIZE // PAGE_SIZE
# The size of the bitmap in bytes.
# Each bit represents a page, so we divide the total number of pages by 8.
BITMAP_SIZE = TOTAL_PAGES // 8

def analyze_memory_dump(file_path):
    """
    Reads a memory dump file, interprets the page bitmap at the beginning,
    and prints a formatted memory map.
    """
    if not os.path.exists(file_path):
        print(f"Error: The file '{file_path}' was not found.")
        return

    try:
        # Open the memory dump file in binary read mode.
        with open(file_path, 'rb') as f:
            # Seek to the start of the bitmap.
            f.seek(BITMAP_START)
            # Read only the portion of the file that contains the bitmap.
            bitmap_data = f.read(BITMAP_SIZE)

        # Check if we read the expected number of bytes.
        if len(bitmap_data) < BITMAP_SIZE:
            print(f"Error: The file is too small. Expected {BITMAP_SIZE} bytes for the bitmap, but only read {len(bitmap_data)}.")
            return

    except IOError as e:
        print(f"Error reading file: {e}")
        return

    print(f"Analyzing {TOTAL_MEMORY_SIZE // (1024*1024)} MB of memory with a page size of {PAGE_SIZE} bytes.")
    print(f"Total pages: {TOTAL_PAGES}")
    print(f"Bitmap start offset: {BITMAP_START} bytes")
    print(f"Bitmap size: {BITMAP_SIZE} bytes\n")
    print("--- Memory Map ---")
    print("Page Range\t\t\t\tStatus\t\tAddress Range")
    print("-" * 80)

    # Variables to track the current block of contiguous pages.
    current_status = None
    block_start_page = 0

    # Iterate through the bitmap, byte by byte.
    for byte_index, byte_value in enumerate(bitmap_data):
        # Iterate through each bit of the current byte.
        for bit_index in range(8):
            # Calculate the current page number.
            page_number = (byte_index * 8) + bit_index

            # Ensure we don't exceed the total number of pages.
            if page_number >= TOTAL_PAGES:
                break

            # Check the status of the current page (1 for used, 0 for free).
            # We use a bitwise AND operation to check if the bit is set.
            is_used = (byte_value >> bit_index) & 1

            # Determine the status based on the bit value.
            page_status = "USED" if is_used else "FREE"

            # This is the main logic for grouping pages into blocks.
            if current_status is None:
                # Initialize the first block.
                current_status = page_status
                block_start_page = page_number
            elif page_status != current_status:
                # The status has changed. Print the previous block.
                block_end_page = page_number - 1
                start_address = block_start_page * PAGE_SIZE
                end_address = (block_end_page + 1) * PAGE_SIZE - 1

                print(f"Page {block_start_page:<6} to {block_end_page:<6}\t{current_status:<8}\t0x{start_address:08x} - 0x{end_address:08x}")

                # Start a new block with the new status.
                current_status = page_status
                block_start_page = page_number
        
        # Break outer loop if we've processed all pages
        if page_number >= TOTAL_PAGES -1:
            break


    # Print the final block after the loop has finished.
    # Ensure the last block doesn't go beyond TOTAL_PAGES
    block_end_page = TOTAL_PAGES - 1
    start_address = block_start_page * PAGE_SIZE
    end_address = (block_end_page + 1) * PAGE_SIZE - 1
    print(f"Page {block_start_page:<6} to {block_end_page:<6}\t{current_status:<8}\t0x{start_address:08x} - 0x{end_address:08x}")
    print("-" * 80)
    print("Analysis complete.")


if __name__ == "__main__":
    file_name = "memory_dump.bin"
    analyze_memory_dump(file_name)
