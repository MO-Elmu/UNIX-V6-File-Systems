#include "pathname.h"
#include "directory.h"
#include "inode.h"
#include "diskimg.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

int pathname_lookup(struct unixfilesystem *fs, const char *pathname) {
   
   int inumber = ROOT_INUMBER;
   struct direntv6 result;
   size_t pathLength = strlen(pathname);
   char abs_path[pathLength+1];   // +1 to compensate for the null terminator \0  
   memcpy (abs_path,pathname,pathLength+1);
 
   //Make sure pathname is absolute path
   if(pathname[0] != 47){  // 47 is forward slash / in ASCII table
   	fprintf(stderr, "Error Path name must be an absolute path.\n");
   	return -1; 
   }
  //since this function only handle absolute path the starting inumber is always 1, and no need for recursion
  char *dirName;
  dirName = strtok(abs_path, "/");  //break the pathname to tokens equivalent to dir and file name using slash "/" as a delimiter
  while (dirName != NULL){
  	if(directory_findname(fs, dirName, inumber, &result) < 0){
  	     fprintf(stderr, "Error finding directory or file name %s. Returing -1.\n", dirName);
             return -1;
        }
        inumber = result.d_inumber;
        dirName = strtok(NULL, "/");	
  }
  return inumber;

}

