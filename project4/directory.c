#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "API.h"

int dir_make(char *name)
{
  Dentry newDir;
  
  if(search_cur_dir(name) != -1)
    {
      printf("mkdir error: file/dir already exists\n");
      return -1;
    }
  // add a new file into the current directory entry
  int newInodeNum = get_inode();
  Inode tempNode = read_inode(newInodeNum);
  tempNode.type = directory;
  tempNode.directBlock[0] = get_block();
  tempNode.size = 1;
  write_inode(newInodeNum, tempNode);

  DirectoryEntry newEntry;
  for (int i = 0; i < sizeof(name); i ++)
    {
      newEntry.name[i] = name[i];
    }
  newEntry.inode = newInodeNum;
  curDir.dentry[curDir.numEntry] = newEntry;
  curDir.numEntry++;

  DirectoryEntry newRoot;
  newRoot.name[0] = '.';
  newRoot.name[1] = '\0';
  newRoot.inode = curDir.dentry[1].inode;
  newDir.dentry[0] = newRoot;
  newDir.numEntry = 1;

  DirectoryEntry newParent;
  newParent.name[0] = '.';
  newParent.name[1] = '.';
  newParent.name[2] = '\0';
  newParent.inode = curDir.dentry[0].inode;
  newDir.dentry[1] = newParent;
  newDir.numEntry = 2;

  Inode direct = read_inode(newEntry.inode);
  write_disk_block(direct.directBlock[0], (char*)&newDir);
  Inode current = read_inode(curDir.dentry[0].inode);  
  write_disk_block(current.directBlock[0], (char*)&curDir);
   
  return 0;
}

int dir_remove(char *name)
{
  int inodeNum;
  inodeNum = search_cur_dir(name);
  Inode read = read_inode(inodeNum);
    
  if (inodeNum == -1)
    {
      printf("Error: Directory not found\n");
      return -1;
    }
  if (read.type == file)
    {
      printf("Error: cannot delete file with this command\n");
      return -1;
    }
  if (strcmp(name, "..") == 0 || strcmp(name, ".") == 0)
    {
      printf("Error: cannot remove directory\n");
      return -1;
    }
       
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
	      if (read.linkCount > 1) {
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

int dir_change(char *name)
{
  if (search_cur_dir(name) == -1)
    {
      printf("Directory not found\n");
      return -1;
    }

  int newNum = search_cur_dir(name);
  Inode newNode = read_inode(newNum);
  Inode curNode = read_inode(curDirInode);

  write_disk_block(curNode.directBlock[0], (char*)&curDir);
  read_disk_block(newNode.directBlock[0], (char*)&curDir);
  curDirInode = newNum;

  return 0;
}


/* ===================================================== */

int ls()
{
  int i;
  int inodeNum;
  Inode targetInode;
  for(i = 0; i < curDir.numEntry; i++)
    {
      inodeNum = curDir.dentry[i].inode;
      targetInode = read_inode(inodeNum);
      if(targetInode.type == file) printf("type: file, ");
      else printf("type: dir, ");
      printf("name \"%s\", inode %d, size %d byte\n", curDir.dentry[i].name, curDir.dentry[i].inode, targetInode.size);
    }

  return 0;
}

