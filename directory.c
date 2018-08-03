#include "directory.h"
#include "inode.h"
#include "diskimg.h"
#include "file.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

#define NUM_DIR_ENTRIES (DISKIMG_SECTOR_SIZE/sizeof(struct direntv6))


int directory_findname(struct unixfilesystem *fs, const char *name,
		       int dirinumber, struct direntv6 *dirEnt) {
  
  /* getting the number of blocks in this directory through the inode layer */
  struct inode in;
  
  if ((inode_iget(fs, dirinumber, &in) < 0)){
  	fprintf(stderr, "Can't read directory inode %d or inode isn't for a directory \n", dirinumber); 
      return -1;	
  }
  int size = inode_getsize(&in);
  int numBlocks = (size + DISKIMG_SECTOR_SIZE - 1) / DISKIMG_SECTOR_SIZE;
  
  /* looping through all blocks in a directory inode searching for the name */
  int i;
  struct direntv6 dirBlock [NUM_DIR_ENTRIES];
  for (i = 0; i < numBlocks; i++) {
      int validBytes = file_getblock(fs, dirinumber, i , dirBlock);
      if(validBytes < 0){
      		fprintf(stderr, "Can't read block number %d in directory inumber  %d \n", i, dirinumber); 
      	return -1; 
      }	
      struct direntv6 temp;
      int valid_dir_entries = (validBytes/sizeof(struct direntv6));
      int j;
      for(j = 0; j < valid_dir_entries; j++){
      	temp = dirBlock[j];
      	if (strcmp(temp.d_name,name) == 0){
      		*dirEnt = temp;
      		return 0;
      	}
      }
  } 
  /* if name not found report error and and return -1 */
  fprintf(stderr, "directory_lookupname(name=%s dirinumber=%d) unsuccessful \n", name, dirinumber); 
  return -1;
}

