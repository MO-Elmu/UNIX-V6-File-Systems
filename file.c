#include <stdio.h>
#include <assert.h>

#include "file.h"
#include "inode.h"
#include "diskimg.h"

// remove the placeholder implementation and replace with your own
int file_getblock(struct unixfilesystem *fs, int inumber, int blockNum, void *buf) {
  struct inode in;
  if (inode_iget(fs, inumber, &in) < 0) {
    fprintf(stderr,"Can't read inode %d \n", inumber);
    return -1;
  }
  int disk_blkNo = inode_indexlookup(fs, &in, blockNum);
  
  if (disk_blkNo < 0){
    fprintf(stderr,"Can't read disk block number %d\n", disk_blkNo);
    return -1;
  }
  int bytes_read = diskimg_readsector(fs->dfd, disk_blkNo, buf);
  int size = inode_getsize(&in);
  int numBlocks = (size + DISKIMG_SECTOR_SIZE - 1) / DISKIMG_SECTOR_SIZE;

  /* Returning only the valid bytes thant belong to the file and ignoring any junk in the last block */

  if((blockNum == (numBlocks-1)) && ((size/DISKIMG_SECTOR_SIZE) < numBlocks)){
      return (size%DISKIMG_SECTOR_SIZE);
  }
  return bytes_read;
}

