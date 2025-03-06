# Project 3: Block Storage

```
DueApr 6 by 11:59pm
Points 100
Submittinga file upload
File Typestar.gz, zip, and tgz
Availableafter Mar 1 at 12am
```
```
Start Assignment
```
# Project 3: Block Storage

This is a group project. You can view your group assignment under the "People" page on
Canvas (https://k-state.instructure.com/courses/168886/groups (https://k-
state.instructure.com/courses/168886/groups#tab-29602) ). You are responsible for getting in
touch with your group members to start collaborating. Don't put this off!!

### 1. Introduction:

As in the previous project, you will need to login to Beocat and copy the project files to
your home directory. To do so, enter the following command from your terminal:

```
ssh your_eid@headnode.beocat.ksu.edu
```
Enter your password when prompted. Next, run the following command from your
terminal:

```
cp ~eyv/cis520/hw3.zip.
```
This will copy "hw3.zip" located in Eugene's home directory to the directory you are
currently in (in most cases, your home directory). Alternatively, it's also available on
Canvas. (https://k-state.instructure.com/courses/168886/files/40922417?wrap=1) (https://k-
state.instructure.com/courses/168886/files/40922417/download?download_frd=1) Verify that the
copy is successful (and that you did not miss the period at the end of the preceding
command) using the following command:

```
ls hw*
```
You should see the file in your current directory.

Next, you need to extract the contents of the zipped file. For this, you need to enter the
following command:


```
unzip hw3.zip
```
All required files should now be extracted to the folder "hw3". Before you can start working
on the project, however, you will need to load the required modules (cmake, googletest,
and foss). To do so, enter the following command:

```
module load googletest/1.11.0-GCCcore-11.3.0 CMake/3.21.1-GCCcore-11.2.0 foss/2021b
```
Complete information on the available modules (and how to load and unload them) can
be found on this page
(https://support.beocat.ksu.edu/BeocatDocs/index.php/Installed_software).

### 2. Objectives:

Become more familiar with the following concepts:

```
Bitmaps, bytes, and blocks
Block storage devices
Free block maps
Checking system errors with errno
```
You are to implement a storage library capable of storing blocks of data for the user.
This data is to be file-backed, i.e. the stored data is written to disk so that it may be used
later (through the serialize/deserialize block store operations). Your implementation
should create opaque (e.g. "no user serviceable parts inside"), self-contained objects.

This exercise touches on many concepts with which you may be unfamiliar, so be sure to
read this document completely.


## Modular Programming:

1. Modular Programming and proper organization
2. Documentation of code via thorough comments
3. Parameter testing and error checking

## Bitmaps:

A bitmap is a data structure that can represent a large set of boolean data in a compact
way. A boolean value can be represented with a single bit, but the smallest addressable
unit is an entire byte, which leaves a lot of wasted space. If you need to represent multiple
boolean values, you're going to waste even more memory, and this can add up quickly.
This leaves less memory for other programs, which is a terrible thing for an OS or service
to do.

A bitmap uses binary operations to store multiple boolean variables in a single byte,
reducing your memory footprint to ⅛ of what it would be otherwise. In doing this, however,
a bitmap must use a more complex addressing system that uses both a bit and byte
address in addition to masking.

Please review your "bits" learning module and these resource links:

```
cheung_bit_array_page_hw1.pdf (https://k-
state.instructure.com/courses/168886/files/40001055?wrap=1) (https://k-
state.instructure.com/courses/168886/files/40001055/download?download_frd=1)
https://en.wikipedia.org/wiki/Bit_array (https://en.wikipedia.org/wiki/Bit_array)
```
## Block Storage Devices:

A block storage device is exactly what it sounds like: a device that stores blocks of data.
Most storage devices (hard disk, optical disc, and flash drives) are block devices with
varying block sizes (the number of bytes per block). Reading and writing data in blocks
reduces overhead and allows for better data handling. Each block is referenced by an id
number starting from 0 to N - 1 number of blocks, and these blocks are contiguous,
essentially making a block device a giant physical array.

Please review your "array" learning module and this resource link:
https://en.wikipedia.org/wiki/Block_(data_storage)
(https://en.wikipedia.org/wiki/Block_(data_storage))

## Block Maps:

A block map is how block storage devices keep track of important metadata. With this
block storage library, we will be using the Free Block Map (FBM), implemented as a
bitmap, to keep track of important device data.


The FBM keeps track of which blocks are currently in use. A bit value of 1 means the block
is in use, and 0 means the block is free. When a user requests a block of space, the
library scans the FBM, finds a free block, marks that block as in use in the FBM, and
returns the block id number for the user to use. When a block is freed, the corresponding
bit in the FBM is cleared. Without this, devices would have no way of tracking which
blocks are in use, and current data present is easily overwritten.

You must store your FBM starting in block 127 of the device and continuingto
subsequent blocks as many as needed, but not more.The tests in test/tests.cpp are
written with the assumption that your FBM is stored starting at this particular block. The
choice of this block is well-motivated as many physical devices are more performant in
the middle physical blocks, and it is important that FBM access is as fast as possible.

Please review this resource link: https://en.wikipedia.org/wiki/Block_allocation_map
(https://en.wikipedia.org/wiki/Block_allocation_map)

# Assignment Requirements:

```
You will build a shared object (a dynamic library) called libblock_store.so
You must implement the functions src/block_store.c defined in include/block_store.h
You must update and complete the given CMakeLists.txt to build libblock_store.so
that uses src/block_store.c and src/bitmap.c for source files.
You must check all function calls for error conditions. Occasionally, this will mean you
will have to check errno, so don’t forget to #include <errno.h>. Please refer to manual
pages for information. Hint: There's only one global errno per thread so you have to
check it right after you make a system call, otherwise the next system call might set it
to something else (like "Success"). Hint 2: You can print the "plain English" error
message associated with the current errno code by calling perror("some useful text").
You must ensure that your solution is free of memory leaks. You can check your
program for leaks with Valgrind (don't forget to load its module: module load
Valgrind/3.20.0-gompi-2022a).
```
### Rubric (100 points possible):

This assignment is out of 100 points. The included tests.cpp file provides points for a
variety of tests.

All file operations should be using POSIX interface; you will lose points for using FILE
objects. If your code does not compile, you will receive a zero for the assignment.

Insufficient Parameter Validation: Deduction of up to 20% of rubric score

Insufficient Error Checking: Deduction of up to 20% of rubric score

Insufficient Block and Inline Comments: Deduction of up to 20% of rubric score


Submission compiles with warnings (with -Wall -Wextra -Wshadow): Deduction of up to 30%

### Implementation guidelines:

1. block_store_create(): This function creates a new block store and returns a pointer to it. It
    first allocates memory for the block store and initializes it to zeros using the memset
    (an alternative method to initialize newly-allocated memory to all 0s is to use calloc
    instead of malloc). Then it sets the bitmap field of the block store to an overlay of a
    bitmap with size BITMAP_SIZE_BYTES on the blocks starting at index
    BITMAP_START_BLOCK. (You should define BITMAP_START_BLOCK based on already
    defined constants.) Finally, it marks the blocks used by the bitmap as allocated using
    the block_store_request function.
2. block_store_destroy(block_store_t *const bs): This function destroys a block store by freeing the
memory allocated to it. It first checks if the pointer to the block store is not NULL, and
if so, it frees the memory allocated to the bitmap and then to the block store.
3. block_store_allocate(block_store_t *const bs): This function finds the first free block in the block
store and marks it as allocated in the bitmap. It returns the index of the allocated
block or SIZE_MAX if no free block is available.
4. block_store_request(block_store_t *const bs, const size_t block_id): This function marks a specific
block as allocated in the bitmap. It first checks if the pointer to the block store is not
NULL and if the block_id is within the range of valid block indices. If the block is
already marked as allocated, it returns false. Otherwise, it marks the block as
allocated and checks that the block was indeed marked as allocated by testing the
bitmap. It returns true if the block was successfully marked as allocated, false
otherwise.
5. block_store_release(block_store_t *const bs, const size_t block_id): This function marks a specific
block as free in the bitmap. It first checks if the pointer to the block store is not NULL
and if the block_id is within the range of valid block indices. Then, it resets the bit
corresponding to the block in the bitmap.
6. block_store_get_used_blocks(const block_store_t *const bs): This function returns the number of
blocks that are currently allocated in the block store. It first checks if the pointer to
the block store is not NULL and then uses the bitmap_total_set function to count the
number of set bits in the bitmap.
7. block_store_get_free_blocks(const block_store_t *const bs): This function returns the number of
blocks that are currently free in the block store. It first checks if the pointer to the
block store is not NULL and then calculates the difference between the total number
of blocks and the number of used blocks using the block_store_get_used_blocks and
BLOCK_STORE_NUM_BLOCKS.
8. block_store_get_total_blocks(): This function returns the total number of blocks in the block
store, which is defined by BLOCK_STORE_NUM_BLOCKS.
9. block_store_read(const block_store_t *const bs, const size_t block_id, void *buffer): This function reads the
contents of a block into a buffer. It returns the number of bytes successfully read.


10. block_store_write(block_store_t *const bs, const size_t block_id, const void *buffer): This function writes
    the contents of a buffer to a block. It returns the number of bytes successfully written.
11. block_store_deserialize(const char *const filename): This function deserializes a block store from
a file. It returns a pointer to the resulting block_store_t struct.
12. block_store_serialize(const block_store_t *const bs, const char *const filename): This function serializes a
    block store to a file. It returns the size of the resulting file in bytes. Note: If a test case
    expects a specific number of bytes to be written but your file is smaller, pad the rest of
    the file with zeros until the file is of the expected size. Modify your block_store_deserialize
    function accordingly to accept padding if present.




