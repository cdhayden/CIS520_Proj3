#include <stdio.h>
#include <stdint.h>

#include "bitmap.h"
#include "block_store.h"
// include more if you need


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

    // I'm storing the bitmap in the same array as the disk data here rather than
    // making a totally separate array becuase in a real block device,
    // everything (the user data plus all metadata) normally resides in the same
    // physical storage. This helps keep the entire “disk” self-contained.
    //
    // Your right, we could use a separate array for the bitmap, but we might forget to sync them
    // together or we might lose track of the map. But placing the free-block map
    // in the "middle" of the array ensures that we only have one main chunk of
    // memory to keep track of, making it simpler to treat "bs->data" as the entire
    // device image.I really just read this is the better practice and decided to roll with it,
    // if we want to use a seperate array I can definately do that.
    //
    bs->fbm = bitmap_overlay(BITMAP_SIZE_BITS, loc);
    if (!bs->fbm) {
        free(bs);
        return NULL; // corner case
    }

    // The for-loop marks the blocks used by the map itself as allocated, so
    // that the user can’t override the area that physically stores the map. If we
    // didn’t do that, then we'd have the user’s data overwriting the bits that say
    // which blocks are free or taken, causing a big mess. So the loop ensures
    // the FBM blocks remain off-limits for user data.
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
size_t block_store_allocate(block_store_t *const bs)
{
    //check for valid inputs
    if(bs)
    {
        //return first free bit
        return bitmap_ffs(bs->fbm);
    }
    return SIZE_MAX;
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
    if(bs && block_id < BLOCK_STORE_NUM_BLOCKS && block_id >= 0)
    {
        //release the bit
	    bitmap_reset(bs->fbm, block_id);
        //FIXME: Possibly add clearing the block
    }
}

size_t block_store_get_used_blocks(const block_store_t *const bs)
{
	UNUSED(bs);
	return 0;
}

size_t block_store_get_free_blocks(const block_store_t *const bs)
{
	UNUSED(bs);
	return 0;
}

///
/// Returns the total number of user-addressable blocks
/// \return Total blocks
///
size_t block_store_get_total_blocks()
{
    return BLOCK_STORE_NUM_BLOCKS;
}

size_t block_store_read(const block_store_t *const bs, const size_t block_id, void *buffer)
{
	UNUSED(bs);
	UNUSED(block_id);
	UNUSED(buffer);
	return 0;
}

size_t block_store_write(block_store_t *const bs, const size_t block_id, const void *buffer)
{
	UNUSED(bs);
	UNUSED(block_id);
	UNUSED(buffer);
	return 0;
}

block_store_t *block_store_deserialize(const char *const filename)
{
	UNUSED(filename);
	return NULL;
}

size_t block_store_serialize(const block_store_t *const bs, const char *const filename)
{
	UNUSED(bs);
	UNUSED(filename);
	return 0;
}
