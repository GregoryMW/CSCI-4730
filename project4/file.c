#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "API.h"

int file_cat(char *name)
{
  int inodeNum, i, size;
  char str_buffer[512];
  char * str;
  bool needOF = false;

  //get inode
  inodeNum = search_cur_dir(name);
  size = read_inode(inodeNum).size;
   
  if(size > 7680)
    {
      needOF = true;
    }

  //check if valid input
  if(inodeNum < 0)
    {
      printf("cat error: file not found\n");
      return -1;
    }
  if(read_inode(inodeNum).type == directory)
    {
      printf("cat error: cannot read directory\n");
      return -1;
    }

  //allocate str
  str = (char *) malloc( sizeof(char) * (size+1) );
  str[ size ] = '\0';
  for( i = 0; i < read_inode(inodeNum).blockCount; i++ ){
    int block;
    block = read_inode(inodeNum).directBlock[i];

    read_disk_block( block, str_buffer );

    if( size >= BLOCK_SIZE )
      {
	memcpy( str+i*BLOCK_SIZE, str_buffer, BLOCK_SIZE );
	size -= BLOCK_SIZE;
      }
    else
      {
	memcpy( str+i*BLOCK_SIZE, str_buffer, size );
      }
  }
   
  if(needOF)
    { 
      int indirectBlockMap[128];
      read_disk_block(read_inode(inodeNum).indirectBlock, (char*)indirectBlockMap);

      for(i = 15; i < read_inode(inodeNum).blockCount; i++)
	{
	  read_disk_block(indirectBlockMap[i-15], str_buffer);
	  memcpy( str+(i*BLOCK_SIZE), str_buffer, size );
						
	} 
    }
  printf("%s\n", str);		

  free(str);

  //return success
  return 0;
}
  

int file_remove(char *name)
{
  int inodeNum;
  inodeNum = search_cur_dir(name);
  Inode read = read_inode(inodeNum);
    
  if (inodeNum < 0)
    {
      printf("Error: File not found\n");
      return -1;
    }
  if (read.type == directory)
    {
      printf("Error: cannot delete directory with this command\n");
      return -1;
    }
    
  int block;
  if (read.blockCount > 1)
    {
      for(int i = 0; i < read.blockCount; i++)
	{
	  read.directBlock[i] = 0;
	}
    }
  else
    {
      read.directBlock[0] = 0;
    }
    
  for(int i = 0; i < curDir.numEntry; i++)
    {
      if (curDir.dentry[i].inode == inodeNum)
	{
	  if (strcmp(curDir.dentry[i].name, name) == 0)
	    {
	      curDir.dentry[i].inode = -1;
	      if (curDir.dentry[i+1].inode == -1)
		{
		  curDir.numEntry--;
		}
	      else
		{
		  for (int j = i; j < curDir.numEntry - 1; j++)
		    {
		      curDir.dentry[j].inode = curDir.dentry[j+1].inode;
		      strcpy(curDir.dentry[j].name, curDir.dentry[j+1].name);
		    }
		  curDir.numEntry--;
		} 
	      if (read.linkCount > 1)
		{
		  read.linkCount--;
		}
	    }
	}
    }
  
  for (int i = 0; i < read.blockCount; i ++)
    {
      free_block(read.directBlock[i]);
    }
  free_inode(inodeNum);
    
  return 0;
}

int hard_link(char *src, char *dest)
{
  //get iNodeNum of destination file
  int check = search_cur_dir(dest);

  //Check if dest file exists
  if(check >= 0)
    {
      printf("Error: The destination file already exists\n");
      return -1;

    }

  //Check directory has enough space for additional file
  if(curDir.numEntry + 1 > MAX_DIR_ENTRY)
    {
      printf("Error: The directory is full\n");
      return -1;
    }

  int src_inodeNum = search_cur_dir(src);
  strncpy(curDir.dentry[curDir.numEntry].name, dest, strlen(dest));
  curDir.dentry[curDir.numEntry].name[strlen(dest)] = '\0';
  curDir.dentry[curDir.numEntry].inode = src_inodeNum;
  Inode temp = read_inode(search_cur_dir(src));
  temp.linkCount++;
  write_inode(search_cur_dir(src), temp);
  curDir.numEntry++;

  //change dest inode to src inode
  int destNum = search_cur_dir(dest);

  printf("Hard link generated: %s\n", dest);

  return 0;
}

int file_copy(char *src, char *dest)
{
  int block, i;
  int src_num = search_cur_dir(src);
  if(src_num == -1)
    {
      printf("Source file does not exist.\n");
      return 0;
    }
  int dest_num = search_cur_dir(dest); 
  if(dest_num != -1)
    {
      printf("Dest file already exists.\n");
      return 0;
    }
  
  Inode srcNode = read_inode(src_num);
  dest_num = get_inode();
  Inode destNode;
  destNode.type = srcNode.type;
  destNode.size = srcNode.size;
  destNode.blockCount = srcNode.blockCount;
  destNode.linkCount = 1;
 
  // add a new file into the current directory entry
  strncpy(curDir.dentry[curDir.numEntry].name, dest, strlen(dest));
  curDir.dentry[curDir.numEntry].name[strlen(dest)] = '\0';
  curDir.dentry[curDir.numEntry].inode = dest_num;
  curDir.numEntry++;
   
  int size = srcNode.size;
  int numBlock = size / BLOCK_SIZE;
  char *tmp = (char*) malloc(sizeof(int) * size+1);
  // get data blocks
  for(i = 0; i < 15; i++)
    {
      if (i >= numBlock) break;
      block = get_block();
      if(block == -1) {
	printf("File_copy error: get_block failed\n");
	return -1;
      }
      //set direct block
      destNode.directBlock[i] = block;

      write_disk_block(block, tmp+(i*BLOCK_SIZE));
    }

  if(size > 7680) {
    // get an indirect block
    block = get_block();
    if(block == -1) {
      printf("File_copy error: get_block failed\n");
      return -1;
    }
        
    int indirectBlockMapOld[128];
     
    read_disk_block(read_inode(src_num).indirectBlock, (char*)indirectBlockMapOld);
        
        

    destNode.indirectBlock = block;
    int indirectBlockMap[128];
    char *tmp2 = (char*) malloc(sizeof(int) * size- 7680);

    for(i = 15; i < read_inode(src_num).blockCount; i++)
      {
	block = get_block();
	if(block == -1) {
	  printf("File_create error: get_block failed\n");
	  return -1;
	}
	//set direct block
	read_disk_block(indirectBlockMapOld[i-15], tmp2);
	indirectBlockMap[i-15] = block;
	write_disk_block(block, tmp2);
      }
    write_disk_block(destNode.indirectBlock, (char*)indirectBlockMap);
  }
  write_inode(dest_num, destNode);

  printf("File copy succeed: from %s to %s\n",src, dest);
  return 0;
}


/* =============================================================*/

int file_create(char *name, int size)
{
  int i;
  int block, inodeNum, numBlock;

  if(size <= 0 || size > 73216){
    printf("File create failed: file size error\n");
    return -1;
  }

  inodeNum = search_cur_dir(name); 
  if(inodeNum >= 0) {
    printf("File create failed:  %s exist.\n", name);
    return -1;
  }

  if(curDir.numEntry + 1 > MAX_DIR_ENTRY) {
    printf("File create failed: directory is full!\n");
    return -1;
  }

  if(superBlock.freeInodeCount < 1) {
    printf("File create failed: inode is full!\n");
    return -1;
  }

  numBlock = size / BLOCK_SIZE;
  if(size % BLOCK_SIZE > 0) numBlock++;

  if(size > 7680) {
    if(numBlock+1 > superBlock.freeBlockCount)
      {
	printf("File create failed: data block is full!\n");
	return -1;
      }
  } else {
    if(numBlock > superBlock.freeBlockCount) {
      printf("File create failed: data block is full!\n");
      return -1;
    }
  }

  char *tmp = (char*) malloc(sizeof(int) * size+1);

  rand_string(tmp, size);
  printf("File contents:\n%s\n", tmp);

  // get inode and fill it
  inodeNum = get_inode();
  if(inodeNum < 0) {
    printf("File_create error: not enough inode.\n");
    return -1;
  }
		
  Inode newInode;

  newInode.type = file;
  newInode.size = size;
  newInode.blockCount = numBlock;
  newInode.linkCount = 1;

  // add a new file into the current directory entry
  strncpy(curDir.dentry[curDir.numEntry].name, name, strlen(name));
  curDir.dentry[curDir.numEntry].name[strlen(name)] = '\0';
  curDir.dentry[curDir.numEntry].inode = inodeNum;
  curDir.numEntry++;

  // get data blocks
  for(i = 0; i < 15; i++)
    {
      if (i >= numBlock) break;
      block = get_block();
      if(block == -1) {
	printf("File_create error: get_block failed\n");
	return -1;
      }
      //set direct block
      newInode.directBlock[i] = block;

      write_disk_block(block, tmp+(i*BLOCK_SIZE));
    }

  if(size > 7680) {
    // get an indirect block
    block = get_block();
    if(block == -1) {
      printf("File_create error: get_block failed\n");
      return -1;
    }

    newInode.indirectBlock = block;
    int indirectBlockMap[128];

    for(i = 15; i < numBlock; i++)
      {
	block = get_block();
	if(block == -1) {
	  printf("File_create error: get_block failed\n");
	  return -1;
	}
	//set direct block
	indirectBlockMap[i-15] = block;
	write_disk_block(block, tmp+(i*BLOCK_SIZE));
      }
    write_disk_block(newInode.indirectBlock, (char*)indirectBlockMap);
  }

  write_inode(inodeNum, newInode);
  printf("File created. name: %s, inode: %d, size: %d\n", name, inodeNum, size);

  free(tmp);
  return 0;
}

int file_stat(char *name)
{
  char timebuf[28];
  Inode targetInode;
  int inodeNum;
		
  inodeNum = search_cur_dir(name);
  if(inodeNum < 0) {
    printf("file cat error: file is not exist.\n");
    return -1;
  }
		
  targetInode = read_inode(inodeNum);
  printf("Inode = %d\n", inodeNum);
  if(targetInode.type == file) printf("type = file\n");
  else printf("type = directory\n");
  printf("size = %d\n", targetInode.size);
  printf("linkCount = %d\n", targetInode.linkCount);
  printf("num of block = %d\n", targetInode.blockCount);
}


