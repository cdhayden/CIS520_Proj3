#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "bitmap.h"
#include "block_store.h"
// include more if you need
#include <fcntl.h>    // for open()
#include <sys/stat.h> // for mode constants
#include <unistd.h>   // for write(), close()
#include <errno.h>    // for errno
#include <string.h>

// You might find this handy. I put it around unused parameters, but you should
// remove it before you submit. Just allows things to compile initially.
#define UNUSED(x) (void)(x)

// struct def
struct block_store {
    // arr for "disk" data
    uint8_t data[BLOCK_STORE_NUM_BYTES];
    // free block map
    bitmap_t *fbm;
};

///
/// This creates a new BS device, ready to go
/// \return Pointer to a new block storage device, NULL on error
///
block_store_t *block_store_create()
{
    // create store
    block_store_t *bs = calloc(1, sizeof(block_store_t));
    if (!bs) {
        // corner case
        return NULL;
    }
    // find loc for fbm
    uint8_t *loc = bs->data + (BITMAP_START_BLOCK * BLOCK_SIZE_BYTES);

    // overlay the bitmap
    //
    bs->fbm = bitmap_overlay(BITMAP_SIZE_BITS, loc);
    if (!bs->fbm) {
        free(bs);
        return NULL; // corner case
    }

    // :o
    for (size_t i = BITMAP_START_BLOCK; i < BITMAP_START_BLOCK + BITMAP_NUM_BLOCKS; i++) {
        block_store_request(bs, i); // see minimal request impl below
    }
    return bs;
}

///
/// Destroys the provided block storage device
/// This is an idempotent operation, so there is no return value
/// \param bs BS device
///
void block_store_destroy(block_store_t *const bs)
{
    if (bs) {
        // free overlay
        if (bs->fbm) {
            bitmap_destroy(bs->fbm);
        }
        free(bs);
    }
}

///
/// Searches for a free block, marks it as in use, and returns the block's id
/// \param bs BS device
/// \return Allocated block's id, SIZE_MAX on error
///
// Changed: Originally used bitmap_ffs (which finds a set bit),
// but now uses bitmap_ffz (which looks for a 0).
size_t block_store_allocate(block_store_t *const bs) {
    if (!bs) {
        return SIZE_MAX; // invalid pointer
    }
    // find first free (zero) bit in the bitmap
    size_t freeBlock = bitmap_ffz(bs->fbm);
    // check if no free block found or out of range
    if (freeBlock == SIZE_MAX || freeBlock >= BLOCK_STORE_NUM_BLOCKS) {
        return SIZE_MAX;
    }
    // mark the block as allocated
    bitmap_set(bs->fbm, freeBlock);
    return freeBlock;
}

///
/// Attempts to allocate the requested block id
/// \param bs the block store object
/// \block_id the requested block identifier
/// \return boolean indicating succes of operation
///

bool block_store_request(block_store_t *const bs, const size_t block_id)
{
    if (!bs) return false;
    if (block_id >= BLOCK_STORE_NUM_BLOCKS) return false;
    // if bit set, fail
    if (bitmap_test(bs->fbm, block_id)) return false;
    // else set bit
    bitmap_set(bs->fbm, block_id);
    return true;
}

///
/// Frees the specified block
/// \param bs BS device
/// \param block_id The block to free
///
void block_store_release(block_store_t *const bs, const size_t block_id)
{
    //check for valid input
    if(bs && block_id < BLOCK_STORE_NUM_BLOCKS)
    {
        // Clear :o
        memset(bs->data + block_id * BLOCK_SIZE_BYTES, 0, BLOCK_SIZE_BYTES);

        //release the bit
	    bitmap_reset(bs->fbm, block_id);
    }
}

///
/// Counts the number of blocks marked as in use
/// \param bs BS device
/// \return Total blocks in use, SIZE_MAX on error
///
size_t block_store_get_used_blocks(const block_store_t *const bs)
{
    //check for valid inputs, return amount of total set bits if yes
	return bs && bs->fbm ? bitmap_total_set(bs->fbm) : SIZE_MAX;
}

///
/// Counts the number of blocks marked free for use
/// \param bs BS device
/// \return Total blocks free, SIZE_MAX on error
///
size_t block_store_get_free_blocks(const block_store_t *const bs)
{
    //check for valid inputs, return total bits - total set bits if yes
	if (!bs || !bs->fbm)
	{
		return SIZE_MAX; // return error
	}
	return bitmap_get_bits(bs->fbm) - bitmap_total_set(bs->fbm);
}

///
/// Returns the total number of user-addressable blocks
/// \return Total blocks
///
size_t block_store_get_total_blocks()
{
    return BLOCK_STORE_NUM_BLOCKS;
}

///
/// Reads data from the specified block and writes it to the designated buffer
/// \param bs BS device
/// \param block_id Source block id
/// \param buffer Data buffer to write to
/// \return Number of bytes read, 0 on error
///
size_t block_store_read(const block_store_t *const bs, const size_t block_id, void *buffer)
{
    //check for valid inputs
    if(bs && buffer && block_id < BLOCK_STORE_NUM_BLOCKS && bitmap_test(bs->fbm, block_id))
    {
        //copy memory and return sizes
        memcpy(buffer, bs->data + (block_id * BLOCK_SIZE_BYTES), BLOCK_SIZE_BYTES);
        return BLOCK_SIZE_BYTES;
    }

	return 0;
}

///
/// Reads data from the specified buffer and writes it to the designated block
/// \param bs BS device
/// \param block_id Destination block id
/// \param buffer Data buffer to read from
/// \return Number of bytes written, 0 on error
///
size_t block_store_write(block_store_t *const bs, const size_t block_id, const void *buffer)
{
	//check for valid inputs
    if(bs && buffer && block_id < BLOCK_STORE_NUM_BLOCKS && bitmap_test(bs->fbm, block_id))
    {
        //copy memory and return sizes
        memcpy(bs->data + (block_id * BLOCK_SIZE_BYTES), buffer, BLOCK_SIZE_BYTES);
        return BLOCK_SIZE_BYTES;
    }

	return 0;
}

///
/// Imports BS device from the given file - for grads/bonus
/// \param filename The file to load
/// \return Pointer to new BS device, NULL on error
///

block_store_t *block_store_deserialize(const char *const filename)
{
    // Return NULL on error
    if (!filename) {
        return NULL;
    }

    // Open file for reading only
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        perror("deserialize: open failed");
        return NULL;
    }

    // Allocate a fresh block_store_t
    //   We'll read data into bs->data
    block_store_t *bs = calloc(1, sizeof(block_store_t));
    if (!bs) {
        close(fd);
        return NULL;
    }

    // We'll loop to read exactly BLOCK_STORE_NUM_BYTES (or hit EOF early)
    size_t total_got = 0;
    size_t bytes_left = BLOCK_STORE_NUM_BYTES;
    while (bytes_left > 0) {
        ssize_t got = read(fd, bs->data + total_got, bytes_left);
        if (got < 0) {
            // read error
            perror("deserialize: read failed");
            block_store_destroy(bs); 
            close(fd);
            return NULL;
        }
        if (got == 0) {
            // EOF reached; break out
            break;
        }
        total_got  += (size_t)got;
        bytes_left -= (size_t)got;
    }
    close(fd);

    // If we didn't get the full BLOCK_STORE_NUM_BYTES, 
    // pad the remainder with zero
    if (bytes_left > 0) {
        memset(bs->data + total_got, 0, bytes_left);
    }

    // Now that bs->data is filled (fully or partially), 
    // overlay the bitmap so we have a valid fbm pointer
    bs->fbm = bitmap_overlay(BITMAP_SIZE_BITS,
                             bs->data + (BITMAP_START_BLOCK * BLOCK_SIZE_BYTES));
    if (!bs->fbm) {
        // If overlay fails, clean up
        block_store_destroy(bs);
        return NULL;
    }

    // Return the fresh block_store_t 
    // The fbm is now pointed at the portion of bs->data holding bitmap bits
    return bs;
}

///
/// Writes the entirety of the BS device to file, overwriting it if it exists - for grads/bonus
/// \param bs BS device
/// \param filename The file to write to
/// \return Number of bytes written, 0 on error
///

size_t block_store_serialize(const block_store_t *const bs, const char *const filename)
{
    // Return 0 on any error (null pointers, open failure, write failure, etc.)
    if (!bs || !filename) {
        // minimal early-out, no need for big error message
        return 0;
    }

    // Open the file for writing (create if needed), truncate to empty
    // 0666 gives read/write perms (umask can restrict it further if needed)
    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd < 0) {
        perror("serialize: open failed");
        return 0;
    }

    // We want to write all BLOCK_STORE_NUM_BYTES from bs->data
    const uint8_t *data_ptr = bs->data; 
    size_t total_written = 0;
    size_t bytes_left    = BLOCK_STORE_NUM_BYTES;

    // We'll loop until we write all bytes or an error occurs
    while (bytes_left > 0) {
        ssize_t written = write(fd, data_ptr + total_written, bytes_left);
        if (written < 0) {
            // If write fails, print error and bail
            perror("serialize: write failed");
            close(fd);
            return 0;
        }
        // written can be 0 if the filesystem is full or something else
        if (written == 0) {
            // We can't proceed further; partial file left, but exit anyway
            // Could do zero-padding if needed, but typically 0 means no space
            close(fd);
            return 0;
        }
        total_written += (size_t)written;
        bytes_left    -= (size_t)written;
    }

    // Done writing everything
    close(fd);

    // If we wrote exactly BLOCK_STORE_NUM_BYTES, return that 
    return (total_written == BLOCK_STORE_NUM_BYTES) ? total_written : 0;
}

