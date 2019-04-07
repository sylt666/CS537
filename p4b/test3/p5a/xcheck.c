#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>
#include <stdbool.h>
#include "fs.h"
#define BLOCK_SIZE (512)
int nblocks;
int ninodes;
int size;
//below three are copy from kernel/stat.h
#define T_DIR  1   // Directory
#define T_FILE 2   // File
#define T_DEV  3   // Special device

// This method is modified from test file. if exp is false print msg to stderr and return rt.
#define fscheck(exp, msg, rt) if(exp){}else{\
fprintf(stderr, "%s\n", msg);\
exit(rt);\
}\


//below are copy global variable of mkfs.c
typedef struct Inode_refer{
  int inuse;
  int refer_count;
}Inode_refer;

int fsfd;
struct superblock * sb; // sb = super (sha) block(bi) 
struct dinode * curr_di; //current dinode
uint freeblock;
uint usedblocks;
uint bitblocks;
uint freeinode = 1;
uint root_inode;


//These are new global variable
void * fsp; //xv6 file system image pointer
int fd; // file descriptor


void badinputhandler(){
	fprintf(stderr, "%s\n", "Usage: xcheck <file_system_image>");
	exit(1);
}
void loadimage(off_t len){
	printf("Start mounting image...\n");

	fsp = mmap(NULL, (size_t)len, PROT_READ, MAP_SHARED,fd,0);
	//the image starts at lower address(pointed by fsp) and ends at higher address (fsp + size)
	fscheck(fsp != MAP_FAILED, "Error: MAP_FAILED. ", 0);
	printf("Success: Image mouting to address: %p\n", fsp);

}

int ifblockinuse(uint n){

	int nbytes = n / 8;
	int nbits = n % 8;
	uint mask = 1 << nbits;
	int bitmapblock = BBLOCK(n, ninodes);
	void *bitmap = fsp + BLOCK_SIZE * bitmapblock;
	int cur = *(int *)(bitmap + nbytes);
	uint cond = ((uint)cur & mask) >> nbits;
	if (cond == 1) return 1;
	else return 0;

}

void checkinode(){
	//int nlink;
	//uint  addr;	// stores address of current direct blocks
	//void * block; // stores address of indirect blocks
	printf("-------Start checking nodes.-------\n");
  
  //array for checking block usage
  int block_checker[size];
  for(int i = 0; i < size; i++){
  	block_checker[i] = 0;
  }
  
  //array for checking inode usage
  Inode_refer inode_checker[ninodes];
  for(int i = 0; i < ninodes; i++){
    inode_checker[i].inuse = 0;
    inode_checker[i].refer_count = 0;
  }
  
  
  
	for(int i = 0; i < ninodes; i++){
		
		if(i == 0){
      inode_checker[i].inuse = 1;
      continue;
    }
    
    curr_di = fsp + BLOCK_SIZE * 2 + sizeof(struct dinode) * i;
    
    if(i == 1){
        //DONE 3. Root directory exists, its inode number is 1, and the parent of the root directory is itself.
		//root dir
        fscheck(curr_di->size != 0 && curr_di->type == 1 && curr_di -> addrs[0] != 0, "ERROR: root directory does not exist.", 1);
		struct dirent * root_parent = fsp + curr_di -> addrs[0] * BLOCK_SIZE + sizeof(struct dirent);
		//printf("root dirent inumber at inode number: %d is: %d\n", i, root_parent -> inum);
		fscheck(root_parent -> inum == 1, "ERROR: root directory does not exist.", 1);
	}
    

    
    //DONE 1. Each inode is either unallocated or one of the valid types (T_FILE, T_DIR, T_DEV). If not, print ERROR: bad inode.
    //if type is 0, not in use
    if(curr_di -> type == 0) continue;
    inode_checker[i].inuse = 1;
    
	printf("Checking inode number %d/%d at address: %p type: %d nlink: %d, file size: %u\n", i, ninodes, curr_di, curr_di -> type, curr_di -> nlink, curr_di->size);
    fscheck(curr_di->type == T_DIR || curr_di->type == T_FILE || curr_di->type == T_DEV, "ERROR: bad inode.", 1);
		
    
	if(curr_di -> type == 1){
  		//DONE 4. Each directory contains . and .. entries, and the . entry points to the directory itself.
  		// addrs[0] dirent 0 stores . and dirent 1 stores ..
  		struct dirent * curr_dirent = fsp + curr_di -> addrs[0] * BLOCK_SIZE;
  		//printf("dirent inumber at inode number: %d is: %d\n", i, curr_dirent -> inum);
  		fscheck(curr_dirent -> inum == i, "ERROR: directory not properly formatted.", 1);
	}
   
    //traverse the address of the inode
		for (int j = 0; j <= NDIRECT; j++){
   
			uint addr = (uint)curr_di->addrs[j];
			//block is not in use
      if(addr == 0) break; 
      

      
      if(j != NDIRECT){
        //2. For in-use inodes, each address that is used by inode is valid (points to a valid datablock address within the image). 
        fscheck(addr >= 4 + (ninodes/IPB) && addr < size, "ERROR: bad direct address in inode.", 1);
        
        //7. For in-use inodes, each direct address in use is only used once.
        fscheck(block_checker[addr] == 0, "ERROR: direct address used more than once.", 1);
        block_checker[addr] = 1;
        //TODO: bitmap check
         
        
		//5. For in-use inodes, each address in use is also marked in use in the bitmap.
		//int bitmapblock = BBLOCK(addr, ninodes);
        //void *bit = fsp + BLOCK_SIZE * bitmapblock;
        //int cond = ifblockinuse(addr);
        //printf("bit: %x\n", bit);
  		//fscheck(cond == 1, "ERROR: address used by inode but marked free in bitmap.", 1);
      }
      else{ 
        //2. For in-use inodes, each address that is used by inode is valid (points to a valid datablock address within the image). 
        fscheck(addr >= 4 + (ninodes/IPB) && addr < size, "ERROR: bad indirect address in inode.", 1);
        
        //7. For in-use inodes, each direct address in use is only used once.
        fscheck(block_checker[addr] == 0, "ERROR: indirect address used more than once.", 1);
        block_checker[addr] = 1;
         //indirect address
        //k = 0 - 127
        for(int k = 0; k < BLOCK_SIZE / sizeof(uint); k++){
          //level 2 addr
          uint indir_addr_lv2 = ((uint *)(fsp + BLOCK_SIZE * addr))[k];
          
          if(indir_addr_lv2 == 0) break;
          
          //2. For in-use inodes, each address that is used by inode is valid (points to a valid datablock address within the image). 
          fscheck(indir_addr_lv2 >= 4 + (ninodes/IPB) && indir_addr_lv2 < size, "ERROR: bad indirect address in inode.", 1);
          
          //8. For in-use inodes, each indirect address in use is only used once
          fscheck(block_checker[indir_addr_lv2] == 0, "ERROR: indirect address used more than once.", 1);
          block_checker[indir_addr_lv2] = 1;
        }
         
      }

		}
				
			
		
	}
	printf("second iteration\n");
  for(int i = 0; i < ninodes; i++){
  	curr_di = fsp + BLOCK_SIZE * 2 + sizeof(struct dinode) * i;
    
    //traverse all direct node
 		if(curr_di -> type == 1){
        for(int j = 0; j <= NDIRECT; j++){
     			uint addr = (uint)curr_di->addrs[j];
    			//block is not in use
          if(addr == 0) break; 
          
          if(j != NDIRECT){
            //traverse all the dirents of direct addr
            for(int k = 0; k < BLOCK_SIZE / sizeof(struct dirent); k++){
    			struct dirent * curr_dirent = fsp + addr * BLOCK_SIZE + k * sizeof(struct dirent);
            	if(curr_dirent == 0) break;
            	
              //10. For each inode number that is referred to in a valid directory, it is actually marked in use.
    		  fscheck(inode_checker[curr_dirent->inum].inuse == 1, "ERROR: inode referred to in directory but marked free.", 1);
              //printf("node %d direct referenced inode %d\n", i, curr_dirent->inum);
             
              if(!(j == 0 && k < 2)){
              	inode_checker[curr_dirent->inum].refer_count += 1;
              }
            }
          }
          else{
            //indirect address
            //k = 0 - 127
            for(int k = 0; k < BLOCK_SIZE / sizeof(uint); k++){
              //level 2 addr
              uint indir_addr_lv2 = ((uint *)(fsp + BLOCK_SIZE * addr))[k];
              if(indir_addr_lv2 == 0) break;
              for(int m = 0; m < BLOCK_SIZE / sizeof(struct dirent); m++){
                struct dirent * curr_dirent = fsp + indir_addr_lv2 * BLOCK_SIZE + m * sizeof(struct dirent);
                if(curr_dirent == 0) break;
                //10. For each inode number that is referred to in a valid directory, it is actually marked in use.
                //printf("node %d indirect referenced inode %d\n", i, curr_dirent->inum);
                fscheck(inode_checker[curr_dirent->inum].inuse == 1, "ERROR: inode referred to in directory but marked free.", 1);
                inode_checker[curr_dirent->inum].refer_count += 1;
              } 
            }
          } 
        }
	  }
    
  }
  
  printf("third iter\n");
  for(int i = 1; i < ninodes; i++){
    //printf("%d\n", i);
    curr_di = fsp + BLOCK_SIZE * 2 + sizeof(struct dinode) * i;
    //if(curr_di->type == 0) break;
    
    printf("node %d, type = %d, nlink = %d, inuse = %d, referc = %d\n", i, curr_di->type, curr_di->nlink, inode_checker[i].inuse, inode_checker[i].refer_count);
    
    if(i == 1) continue;
    
    //12 No extra links allowed for directories (each directory only appears in one other directory).
    if(curr_di->type == 1){
    	fscheck(inode_checker[i].refer_count == 1, "ERROR: directory appears more than once in file system.", 1);
    }
    
    
    //11 Reference counts (number of links) for regular files match the number of times file is referred to in directories 
    if(curr_di->type == 2){
    	fscheck(inode_checker[i].refer_count == curr_di->nlink, "ERROR: bad reference count for file.", 1);
    }
    
    
    if(inode_checker[i].inuse == 1){
      fscheck(inode_checker[i].refer_count >= 1, "ERROR: inode marked use but not found in a directory.", 1);
    }
    else if(inode_checker[i].inuse == 0){
      fscheck(inode_checker[i].refer_count == 0, "ERROR: inode referred to in directory but marked free.", 1);
    }
    else{
      printf("?????????????\n");
    }
  }
	//free(inode_checker);
	//free(block_checker);
	for(int i = 4 + (ninodes/IPB); i < size; i++){
		printf("block %d, inuse = %d", i, block_checker[i]);
		if(block_checker[i] == 1){
			int cond = ifblockinuse(i);
			printf(", cond = %d\n", cond);
			fscheck(cond == 1, "ERROR: address used by inode but marked free in bitmap.", 1);
		}
		if(block_checker[i] == 0){
			int cond = ifblockinuse(i);
			printf(", cond = %d\n", cond);
			fscheck(cond == 0, "ERROR: bitmap marks block in use but it is not in use.", 1);
		}
	}

	
	printf("--------Done checking nodes.-------\n");
}

int main(int argc, char *argv[])
{
	if(argc != 2) badinputhandler();
	
	printf("Reciving file name: %s\n", argv[1]);
	fd = open(argv[1], O_RDONLY);
	fscheck(fd != -1, "image not found.", 1);
	
	struct stat st;
	int r = fstat(fd, &st); // obtain file information
	fscheck(r != -1, "Error: obtain file size failed.", 0);

	off_t len = st.st_size; // obtain file length
	printf("The image size is: %lu\nFile descriptor: %d\n", len, fd);
	loadimage(len);
	
	// main program
	//first read in the super block

	sb = fsp + BLOCK_SIZE;
	printf("Super block at address: %p\n", sb);
	nblocks = sb->nblocks;
	size = sb->size;
	ninodes = sb->ninodes;
	printf("Information in superblock:\nNumber of blocks: %d\nNumber of Inodes: %d\nSize: %d\n", nblocks, ninodes, size);
	printf("Number of Inodes per block is: %lu\n", IPB);
	checkinode();

	munmap(fsp, len);
	printf("TEST PASSED!\n");
	return 0;
}
