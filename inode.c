#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "inode.h"
#include "diskimg.h"
#include "ino.h"

#define INODES_PER_BLOCK  (DISKIMG_SECTOR_SIZE/sizeof(struct inode))

/* This represent the first block number that will fall in i_addr[7] and will be stored in Doubly indirect fashion */ 
#define FIRST_DOUBLY_INDIRECT_BLOCK  1792    // (# of singly indirect table entries) 7 * 256 (#of block numbers can be stored in a block) = 1792



/*Function: gets the inode number and returns 0 and store the inode in struct pointed to by struct inode *inp
            returns -1 in case of failure */

int inode_iget(struct unixfilesystem *fs, int inumber, struct inode *inp) {
	
  int fd = fs->dfd;    // file descriptor of the image file
  int blk_no;          // Sector/Block number where this inode (with this inumber) reside
  if ((inumber % INODES_PER_BLOCK) == 0) {blk_no = (inumber/INODES_PER_BLOCK)+1;} //if the inode # is multiple of 16 then it falls in this block number (inode#/16)+1
  else {blk_no =  ((inumber/INODES_PER_BLOCK) + INODE_START_SECTOR);}                // if not it falls in block number (inode# /16)+2 
  struct inode in_blk [INODES_PER_BLOCK];                   
  if (diskimg_readsector(fd, blk_no, in_blk) != DISKIMG_SECTOR_SIZE) {
  	fprintf(stderr, "Warning: memory block contains inode has size (%zu) != SECTOR_SIZE\n",
            sizeof(struct filsys));
        return -1;
  }
  int in_index = inumber - ((blk_no - INODE_START_SECTOR) * INODES_PER_BLOCK) - 1 ;
  *inp = in_blk[in_index];
   return 0;
}

/*Function: gets the inode and the file block Number and returns the actual block number in the file system */


int inode_indexlookup(struct unixfilesystem *fs, struct inode *inp, int blockNum) {
  
  uint16_t blkNo_buff[DISKIMG_SECTOR_SIZE/sizeof(uint16_t)];
  	
  int size = inode_getsize(inp);
  int inode_blkTable_length = (sizeof(inp->i_addr)/sizeof(inp->i_addr[0]));
  int numBlocks = (size + DISKIMG_SECTOR_SIZE - 1) / DISKIMG_SECTOR_SIZE;

  /* Simple checks to make sure the passed blockNum is within file range */
  
  if ((blockNum > (numBlocks-1)) || (blockNum < 0)) {
  	fprintf(stderr, "Error: File block number %d is out of range \n", blockNum);	
  	return -1;
  }
  
  /* This code handle small files up to 8 blocks with no indirection */
   
  if ((inp->i_mode & ILARG) == 0){
  	if (blockNum >= inode_blkTable_length) {
  		fprintf(stderr, "Error: File block number %d is out of range \n", blockNum);	
  	        return -1;
        }
  		
  	return inp->i_addr[blockNum];	
  }
  // if the file is big and inode file blocks table is indirect execute the below algotithm
  
  
  
  /* First will handle the case if the supplied blockNum falls in an index with double inderiction i_addr[7] */
  
  if (blockNum >= FIRST_DOUBLY_INDIRECT_BLOCK){
      uint16_t temp_buff[DISKIMG_SECTOR_SIZE/sizeof(uint16_t)];
      
      /*arithmatic gives the block number to the 2nd level of indirection */
      int index = (blockNum - FIRST_DOUBLY_INDIRECT_BLOCK) / (DISKIMG_SECTOR_SIZE/sizeof(uint16_t)); 

      /*arithmatic gives index that stores the absolute file sys block in the 2nd indirection */
      int fileBlk_index = (blockNum - FIRST_DOUBLY_INDIRECT_BLOCK) - index * (DISKIMG_SECTOR_SIZE/sizeof(uint16_t)); 
      
      /* Resolving First indirection */
      if (diskimg_readsector(fs->dfd, inp->i_addr[7], temp_buff) != DISKIMG_SECTOR_SIZE){
      	fprintf(stderr, "Error reading 1st indirect block number table\n");
      	return -1;
      }
      /* Resolving Second indirection */
      int newBlkNo = temp_buff[index];
      if (diskimg_readsector(fs->dfd, newBlkNo, blkNo_buff) != DISKIMG_SECTOR_SIZE){
      	fprintf(stderr, "Error reading 2nd indirect block number table\n");
      	return -1;
      }
      
  return  blkNo_buff[fileBlk_index];  	
  	
  }
  
  
  /* Handles the case when blockNum is in the range of single indirection */
  
  /* This gives the index in the inode table as a starting point in locating the file blockNum */
   int index_in = blockNum / (DISKIMG_SECTOR_SIZE/sizeof(uint16_t)) ;   // 256 num of blocks numbers stored in a disk block 512/2, index inode(in)
  
   int relative_index = blockNum - index_in*(DISKIMG_SECTOR_SIZE/sizeof(uint16_t));  //DISKIMG_SECTOR_SIZE/2 = 256 blk numbers stored in a disk block 
  
  if (diskimg_readsector(fs->dfd, inp->i_addr[index_in], blkNo_buff) != DISKIMG_SECTOR_SIZE){
  	fprintf(stderr, "Error reading inode block number table\n");
  	return -1;
  }
 return blkNo_buff[relative_index]; 

}

int inode_getsize(struct inode *inp) {
  return ((inp->i_size0 << 16) | inp->i_size1); 
}

