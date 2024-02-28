/* fs.c
 * 
 * Simplified version of the Unix file system. 
 * Takes the operations specified by the user through the shell and performs them on a disk image.
 * Supported operations are format fs, debug fs, mount fs, create inode, delete inode, get inode size, read disk, and write disk.
 *
 * NOTE: code which is commented out was provided to me for this assignment.
 * My work begins with fs_format() below the commented section.
*/


// #include "fs.h"
// #include "disk.h"

// #include <stdio.h>
// #include <stdint.h>
// #include <stdlib.h>
// #include <stdbool.h>
// #include <string.h>
// #include <assert.h>
// #include <time.h>
// #include <math.h>

// extern struct disk *thedisk;

// #define FS_MAGIC           0x34341023
// #define INODES_PER_BLOCK   128
// #define POINTERS_PER_INODE 3
// #define POINTERS_PER_BLOCK 1024
// #define MIN(a,b) ((a)<(b)?(a):(b))

// int mounted = false;
// unsigned char *freeblock = NULL;

// struct fs_superblock {
// 	uint32_t magic;
// 	uint32_t nblocks;
// 	uint32_t ninodeblocks;
// 	uint32_t ninodes;
// };

// struct fs_inode {
// 	uint32_t isvalid;
// 	uint32_t size;
// 	int64_t ctime;
// 	uint32_t direct[POINTERS_PER_INODE];
// 	uint32_t indirect;
// };

// union fs_block {
// 	struct fs_superblock super;
// 	struct fs_inode inode[INODES_PER_BLOCK];
// 	int pointers[POINTERS_PER_BLOCK];
// 	unsigned char data[BLOCK_SIZE];
// };

// // print out the free block bitmap.
// void printfree() {
// 	for(int i=0;i<disk_nblocks(thedisk);i++) {
// 		printf("%02X",freeblock[i]);
// 	}
// }

// // set the bit indicating that block b is free.
// void markfree(int b) {
// 	int ix = b/8; // 8 bits per byte; this is the index of the byte to modify
// 	unsigned char mask[] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
// 	// OR in the bit that corresponds to the block we're talking about.
// 	freeblock[ix] |= mask[b%8];
// }

// // set the bit indicating that block b is used.
// void markused(int b) {
// 	int ix = b/8; // 8 bits per byte; this is the index of the byte to modify
// 	unsigned char mask[] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
// 	// AND the byte with the inverse of the bitmask to force the desired bit to 0
// 	freeblock[ix] &= ~mask[b%8];
// }

// // check to see if block b is free
// int isfree(int b) {
// 	int ix = b/8; // 8 bits per byte; this is the byte number
// 	unsigned char mask[] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
// 	// if bit is set, return nonzero; else return zero
// 	return ((freeblock[ix] & mask[b%8]) != 0);
// }

// // find a free block and return its index of a free block
// // Note: the superblock and inode blocks' bits need to be 0
// int getfreeblock() {
// 	//printf("Searching %d blocks for a free block\n",disk_nblocks(thedisk));
// 	for(int i=0; i<disk_nblocks(thedisk);i++)
// 		if (isfree(i)) {
// 			//printf("found free block at %d\n",i);
// 			return i;
// 		}
// 	printf("No free blocks found\n");
// 	return -1;
// }

// // return number of free blocks
// // This also expects the superblock and inode blocks to be marked unavailable
// unsigned int nfreeblocks() {
// 	unsigned int n=0;
// 	for(int i=0;i<disk_nblocks(thedisk);i++)
// 		if (isfree(i)) n++;
// 	return n;
// }

// // return the disk block number for the blkno'th data block in the file owned by inode
// int getfblockindex(struct fs_inode *inode,unsigned int blkno) {
// 	assert(blkno < (POINTERS_PER_BLOCK + POINTERS_PER_INODE));
// 	if (blkno < POINTERS_PER_INODE)  {
// 		assert(inode->direct[blkno] != 0);
// 		assert(!isfree(inode->direct[blkno]));
// 		return inode->direct[blkno];
// 	} else {
// 		// read indirect block
// 		union fs_block ib;
// 		assert(!isfree(inode->indirect));
// 		disk_read(thedisk,inode->indirect,ib.data);
// 		// read data pointed to by pointer within the block (make sure to offset index)
// 		int dbno = ib.pointers[blkno-POINTERS_PER_INODE];
// 		assert(dbno != 0);
// 		assert(!isfree(dbno));
// 		return dbno;
// 	}
// }

// // given an inode and a block number,
// // it gets the block and puts the data in the buffer pointed to by data.
// // as a BONUS, it puts the disk block number into the variable pointed to by bkidx (if bkidx is not NULL)
// void getfblock(struct fs_inode *inode,unsigned char *data, unsigned int fblkno,unsigned int *bkidx) {
// 	int dblkno = getfblockindex(inode, fblkno);
// 	disk_read(thedisk,dblkno,data);
// 	if (bkidx) *bkidx=dblkno;
// }


int fs_format()
{
	if (mounted == true) {
		printf("format: fs already mounted\n");
		return 0;
	}

	int nblocks = disk_nblocks(thedisk);
	int ninodeblocks = ceil(nblocks / 10.0);

	union fs_block b;

	b.super.magic = FS_MAGIC;
	b.super.nblocks = nblocks;
	b.super.ninodeblocks = ninodeblocks;
	b.super.ninodes = ninodeblocks * INODES_PER_BLOCK;

	disk_write(thedisk, 0, b.data);

	// zero out inode blocks
	memset(b.data, 0, BLOCK_SIZE);

	for (int block = 1; block <= ninodeblocks; block++) {
		disk_write(thedisk, block, b.data);
	}

	return 1;
}

/* Scans a mounted filesystem and reports on how the inodes and blocks are organized */
void fs_debug()
{
	union fs_block block;
	union fs_block indirect_block;
	struct fs_inode inode;
	struct tm *ptm;
	int inode_number;

	disk_read(thedisk,0,block.data);
	int ninodeblocks = block.super.ninodeblocks;

	printf("superblock:\n");
	printf("    %d blocks\n",block.super.nblocks);
	printf("    %d inode blocks\n",ninodeblocks);
	printf("    %d inodes\n",block.super.ninodes);

	for (int inode_block_number = 0; inode_block_number < ninodeblocks; inode_block_number++) 
	{
		disk_read(thedisk, inode_block_number + 1, block.data);

		for (int offset = 1; offset <= INODES_PER_BLOCK; offset++)
		{
			inode_number = inode_block_number*INODES_PER_BLOCK + offset;

			inode = block.inode[offset];
			if (inode.isvalid != 1) continue;

			printf("inode %d:\n", inode_number);
			printf("    valid: %d\n", inode.isvalid);

			printf("    size: %d bytes\n", inode.size);
			ptm = localtime(&inode.ctime);
			printf("    created: %d/%d/%d %02d:%02d:%02d\n", ptm->tm_mon + 1, ptm->tm_mday, 
				ptm->tm_year + 1900, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
			printf("    direct blocks: ");
			for (int i = 0; i < POINTERS_PER_INODE; i++) {
				if (inode.direct[i] == 0) continue;
				printf("%d ", inode.direct[i]);
			}
			printf("\n");

			// read indirect block
			if (inode.indirect == 0) continue;
			disk_read(thedisk, inode.indirect, indirect_block.data);

			printf("    indirect block: %d\n", inode.indirect);
			printf("    indirect data blocks: ");
			for (int i = 0; i < POINTERS_PER_BLOCK; i++) {
				if (indirect_block.pointers[i] == 0) continue;
				printf("%d ", indirect_block.pointers[i]);
			}
			printf("\n");
		}
	}
}

int fs_mount()
{
	if (mounted == true) {
		printf("mount: fs already mounted\n");
		return 0;
	}

	union fs_block b;
	union fs_block indirect_block;
	struct fs_inode inode;

	disk_read(thedisk, 0, b.data);

	if (b.super.nblocks < 1) {
		printf("mount: no blocks\n");
		return 0;
	}
	
	if (b.super.ninodeblocks < 1) {
		printf("mount: no inode blocks\n");
		return 0;
	}

	// build free block bitmap
	int nblocks = disk_nblocks(thedisk);

	// exact number of bytes needed for bitmap
	unsigned int nfbb = (nblocks * sizeof(unsigned char)/8) + ((nblocks % 8) != 0) ? 1 : 0;
	
	if (freeblock) free(freeblock);
	freeblock = (unsigned char *) malloc(nfbb);
	if (freeblock == NULL) {
		perror("malloc"); 
		return 0; 
	}

	// initialize all entries to free
	int nfb = nblocks;
	for (int blkno = 0; blkno < nblocks; blkno++) { markfree(blkno); }

	// mark superblock and all inode blocks as used
	for (int blkno = 0; blkno <= b.super.ninodeblocks; blkno++) {
		markused(blkno);
		nfb--;
	}

	// loop over all inodes
	for (int inode_block_number = 0; inode_block_number < b.super.ninodeblocks; inode_block_number++) 
	{
		disk_read(thedisk, inode_block_number + 1, b.data);

		for (int offset = 1; offset <= INODES_PER_BLOCK; offset++)
		{
			inode = b.inode[offset-1];
			if (inode.isvalid != 1) continue;
			
			// mark all blocks "pointed to" by direct pointers as used 
			for (int i = 0; i < POINTERS_PER_INODE; i++) {
				if (inode.direct[i] == 0) continue;
				markused(inode.direct[i]);
				nfb--;
			}

			// read indirect block
			if (inode.indirect == 0) continue;
			disk_read(thedisk, inode.indirect, indirect_block.data);
			markused(inode.indirect);
			nfb--;

			for (int i = 0; i < POINTERS_PER_BLOCK; i++) {
				if (indirect_block.pointers[i] == 0) continue;
				markused(indirect_block.pointers[i]);
				nfb--;
			}
		}
	}

	mounted = true;

	return 1;
}

int fs_create()
{
	if (mounted == false) {
		printf("create: fs not mounted\n");
		return 0;
	}

	union fs_block b;

	disk_read(thedisk, 0, b.data);

	int free_inode_blk_num = -1;	// 0 to ninodeblocks
	int free_off = -1;	// 0 to (INODES_PER_BLOCK - 1)

	for (int inode_block_number = 0; inode_block_number < b.super.ninodeblocks; inode_block_number++) 
	{
		if (free_off >= 0) break;	// free inode was found in nested loop

		disk_read(thedisk, inode_block_number + 1, b.data);

		for (int offset = 1; offset < INODES_PER_BLOCK; offset++)
		{
			if (b.inode[offset].isvalid == 1) continue;	// not free

			// free
			free_inode_blk_num = inode_block_number;
			free_off = offset;
			break;
		}
	}

	if (free_off < 0 || free_inode_blk_num < 0) {	
		printf("create: no free inode found\n");
		return 0;
	}

	// initialize new inode
	b.inode[free_off].isvalid = 1;
	b.inode[free_off].size = 0;
	b.inode[free_off].ctime = time(0);
	for (int i = 0; i < POINTERS_PER_INODE; i++) {
		b.inode[free_off].direct[i] = 0;
	}
	b.inode[free_off].indirect = 0;

	disk_write(thedisk, free_inode_blk_num + 1, b.data);

	int inode_number = free_inode_blk_num*INODES_PER_BLOCK + free_off;

	return inode_number;
}

int fs_delete( int inumber )
{
	if (mounted == false) {
		printf("delete: fs not mounted\n");
		return 0;
	}

	union fs_block b;
	union fs_block indirect_b;
	int blk = floor(inumber/INODES_PER_BLOCK) + 1;
	int off = inumber - (blk-1)*INODES_PER_BLOCK;

	disk_read(thedisk, blk, b.data);

	if (b.inode[off].isvalid != 1) {
		printf("delete: inode not created\n");
		return 0;
	}

	// clear inode
	b.inode[off].isvalid = 0;
	b.inode[off].size = 0;
	b.inode[off].ctime = 0;

	// mark each data block in file as free
	for (int i = 0; i < POINTERS_PER_INODE; i++) {
		if (b.inode[off].direct[i] == 0) continue;
		markfree(b.inode[off].direct[i]);
		b.inode[off].direct[i] = 0;
	}
	if (b.inode[off].indirect != 0) {
		disk_read(thedisk, b.inode[off].indirect, indirect_b.data);

		// clear indirect data blocks
		for (int i = 0; i < POINTERS_PER_BLOCK; i++) {
			if (indirect_b.pointers[i] == 0) continue;
			markfree(indirect_b.pointers[i]);
			indirect_b.pointers[i] = 0;
		}
		disk_write(thedisk, b.inode[off].indirect, indirect_b.data);
		markfree(b.inode[off].indirect);
		b.inode[off].indirect = 0;
	}

	disk_write(thedisk, blk, b.data);

	return 1;
}

int fs_getsize( int inumber )
{
	if (mounted == false) {
		printf("getsize: fs not mounted\n");
		return -1;
	}

	union fs_block b;
	int blk = floor(inumber/INODES_PER_BLOCK) + 1;
	int off = inumber - (blk-1)*INODES_PER_BLOCK;

	disk_read(thedisk, blk, b.data);

	if (b.inode[off].isvalid != 1) {
		printf("getsize: inode not created\n");
		return -1;
	}

	return b.inode[off].size;
}

int fs_read( int inumber, unsigned char *data, int length, int offset )
{
	if (mounted == false) {
		printf("read: fs not mounted\n");
		return 0;
	}

	union fs_block b;
	struct fs_inode inode;
	int blk = floor(inumber/INODES_PER_BLOCK) + 1;
	int off = inumber - (blk-1)*INODES_PER_BLOCK;
	int start_blk = floor(offset/BLOCK_SIZE);	// inode block number
	int start_off = offset - start_blk*BLOCK_SIZE;	// offset in inode block
	int bytes_read = 0;

	disk_read(thedisk, blk, b.data);
	inode = b.inode[off];

	if (inode.isvalid != 1) {
		printf("read: inode not created\n");
		return 0;
	}
	if (offset > inode.size) {
		return 0;
	}
	if (offset + length > inode.size) {
		length = inode.size - offset;
	}

	disk_read(thedisk, getfblockindex(&inode, start_blk), b.data);

	while (1) {
		if (b.data[start_off] != 0) {
			data[bytes_read] = b.data[start_off];
			bytes_read++;
		}
		start_off++;
		length--;

		// return if EOF reached or request fulfilled
		if ((b.data[start_off] == 0) || (length == 0)) { return bytes_read; }

		if (start_off >= BLOCK_SIZE) {	// move to next data block
			start_blk++;
			start_off = 0;
			disk_read(thedisk, getfblockindex(&inode, start_blk), b.data);
		}
	}
}

int fs_write( int inumber, const unsigned char *data, int length, int offset )
{
	if (mounted == false) {
		printf("read: fs not mounted\n");
		return 0;
	}

	union fs_block b;
	unsigned char new_block[BLOCK_SIZE];
	unsigned char indirect_data[BLOCK_SIZE];
	union fs_block indirect_block;
	struct fs_inode inode;	
	int blk = floor(inumber/INODES_PER_BLOCK) + 1;	// inode block
	int off = inumber - (blk-1)*INODES_PER_BLOCK;	// inode block offset
	int data_blk = floor(offset/BLOCK_SIZE);	// data block number
	int data_off = offset - data_blk*BLOCK_SIZE;	// offset in data block
	int length_remaining = length;
	int bytes_written = 0;
	int indirect_idx, new_idx;

	disk_read(thedisk, blk, b.data);
	inode = b.inode[off];
	if (inode.isvalid != 1) {
		printf("write: inode not created\n");
		return 0;
	}

	/* Write available direct blocks */
	for (int i = 0; i < POINTERS_PER_INODE; i++) {
		if (inode.direct[i] != 0) { // existing block with data
			new_idx = inode.direct[i];
		} else { // new allocated block
			// set direct pointer
			int free_idx = getfreeblock();
			if (free_idx < 1) {
				printf("write: disk is full\n");
				b.inode[off].size = bytes_written;
				disk_write(thedisk, blk, b.data);
				return bytes_written;
			}
			new_idx = free_idx;
			markused(free_idx);
			b.inode[off].direct[i] = free_idx;
			disk_write(thedisk, blk, b.data);
		}
		
		disk_read(thedisk, new_idx, new_block); // sets new_block to existing data or 0's

		// loop over data block, write to any empty bytes
		while (1) 
		{
			if (new_block[data_off] != 0) { // non-empty byte
				data_off++;
				continue; 
			} 

			new_block[data_off] = data[bytes_written];

			data_off++;
			bytes_written++;
			length_remaining--;

			if (length_remaining < 1) { // return if request fulfilled
				disk_write(thedisk, new_idx, new_block);

				b.inode[off].size = bytes_written;
				disk_write(thedisk, blk, b.data);

				return bytes_written; 
			}	
			if (data_off >= BLOCK_SIZE) { // break if out of space on direct block
				disk_write(thedisk, new_idx, new_block);
				data_off = 0;
				break; 
			}	
		}
	}

	/* Write available indirect blocks */
	if (inode.indirect != 0) {	// indirect block already exists
		indirect_idx = inode.indirect;
	} else { // create indirect block
		indirect_idx = getfreeblock();
		if (indirect_idx < 1) {
			printf("write: disk is full\n");
			b.inode[off].size = bytes_written;
			disk_write(thedisk, blk, b.data);
			return bytes_written;
		}
		markused(indirect_idx);
		b.inode[off].indirect = indirect_idx;
		disk_write(thedisk, blk, b.data);	// write pointer to new indirect block in inode
	}

	disk_read(thedisk, indirect_idx, indirect_block.data);

	// loop through indirect block
	for (int i = 0; i < POINTERS_PER_BLOCK; i++) 
	{	
		if (indirect_block.pointers[i] != 0) { // existing block with data
			new_idx = indirect_block.pointers[i];
		} else { // new allocated block
			// set indirect data pointer
			int free_idx = getfreeblock();
			if (free_idx < 1) {
				printf("write: disk is full\n");
				b.inode[off].size = bytes_written;
				disk_write(thedisk, blk, b.data);
				return bytes_written;
			}
			new_idx = free_idx;
			markused(free_idx);
			indirect_block.pointers[i] = free_idx;
			disk_write(thedisk, indirect_idx, indirect_block.data);
		}

		disk_read(thedisk, new_idx, indirect_data); 

		// loop over data block
		while (1) 
		{
			if (indirect_data[data_off] != 0) { // non-empty byte
				data_off++;
				continue; 
			} 

			indirect_data[data_off] = data[bytes_written];

			data_off++;
			bytes_written++;
			length_remaining--;

			if (length_remaining < 1) { // return if request fulfilled
				disk_write(thedisk, new_idx, indirect_data);

				b.inode[off].size = bytes_written;
				disk_write(thedisk, blk, b.data);

				return bytes_written; 
			}	
			if (data_off >= BLOCK_SIZE) { // break if out of space on data block
				disk_write(thedisk, new_idx, indirect_data);
				data_off = 0;
				break; 
			}	
		}
	}

	printf("write: exceeded maximum file size\n");
	b.inode[off].size = bytes_written;
	disk_write(thedisk, blk, b.data);
	return bytes_written;
}
