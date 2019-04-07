#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fs.h"


// #define DEBUG
int read = 0;
const char* IMAGE_NA = "image not found.";
const char* ERROR_INODE = "ERROR: bad inode.";
const char* ERROR_DIR_ADDR = "ERROR: bad direct address in inode.";
const char* ERROR_IND_ADDR = "ERROR: bad indirect address in inode.";
const char* ERROR_ROOT = "ERROR: root directory does not exist.";
const char* ERROR_FMT = "ERROR: directory not properly formatted.";
const char* ERROR_PARENT = "ERROR: parent directory mismatch.";
const char* ERROR_FREE = "ERROR: address used by inode but marked free in bitmap.";
const char* ERROR_BITMAP = "ERROR: bitmap marks block in use but it is not in use.";
const char* ERROR_MUL = "ERROR: direct address used more than once.";
const char* ERROR_IMUL = "ERROR: indirect address used more than once.";
const char* ERROR_INODE_MISS = "ERROR: inode marked use but not found in a directory.";
const char* ERROR_REFER_MISS = "ERROR: inode referred to in directory but marked free.";
const char* ERROR_FILE = "ERROR: bad reference count for file.";
const char* ERROR_DIRECTORY = "ERROR: directory appears more than once in file system.";

const uchar storage[8] = {0b00000001, 0b00000010, 0b00000100, 0b00001000,
    0b00010000, 0b00100000, 0b01000000, 0b10000000};

uchar bit_getter(uchar* bitmap, uint counter) 
{
    uint position = counter >> 3;
    uint diff = counter & 7;
    return (bitmap[position] & storage[diff]) >> diff;
}


void bit_setter(uchar* bitmap, uint counter) 
{
    uint position = counter >> 3;
    uint diff = counter & 7;
    bitmap[position] |= storage[diff];
}


uchar tester(uchar* bitmap, uint counter) 
{
    uint position = counter >> 3;
    uint diff = counter & 7;
    uchar old = (bitmap[position] & storage[diff]) >> diff;
    bitmap[position] |= storage[diff];
    return old;
}


int main(int argc, char* argv[]) 
{

	FILE* image = fopen(argv[1], "rb");
    
	if (argc == 1) 
	{
        fprintf(stderr, "%s\n", "Usage: xcheck <file_system_image>");
        exit(1);
    }
    
	if (image == NULL) 
	{
        fprintf(stderr, "%s\n", IMAGE_NA);
        exit(1);
    }
    
    uchar buffer[BSIZE];
    
    
    fseek(image, BSIZE, SEEK_CUR);
    
    struct superblock sbstruct;
    read = fread(buffer, 1, BSIZE, image);
    
	memcpy(&sbstruct, buffer, sizeof(struct superblock));
    
    
    uint inode_blks = (sbstruct.ninodes + IPB - 1) / IPB;
    
	struct dinode inode_tbl[inode_blks * IPB];
    
	for (uint i = 0; i < inode_blks; ++i) 
	{
        read = fread(buffer, 1, BSIZE, image);
        memcpy(&inode_tbl[IPB * i], buffer, BSIZE);
    }
    
    fseek(image, BSIZE, SEEK_CUR);
    
    uint bitmap_blks = (sbstruct.size + BPB - 1) / BPB;
    uchar bitmap[bitmap_blks * BSIZE];
    
	for (uint i = 0; i < bitmap_blks; ++i) 
	{
        read = fread(buffer, 1, BSIZE, image);
        memcpy(bitmap, buffer, BSIZE);
    }
    
    uchar bitmap_rec[bitmap_blks * BSIZE];
    memset(bitmap_rec, 0, sizeof(bitmap_rec));
    bit_setter(bitmap_rec, 0);
    bit_setter(bitmap_rec, 1);
    
	for (uint i = 0; i < inode_blks; ++i) 
	{
        bit_setter(bitmap_rec, i + 2);
    }
    
	bit_setter(bitmap_rec, inode_blks + 2);
    
	for (uint i = 0; i < bitmap_blks; ++i) 
	{
        bit_setter(bitmap_rec, i + inode_blks + 3);
    }
    uint data_region = bitmap_blks + inode_blks + 3;
    

    ushort ref_count[sbstruct.ninodes];
    memset(ref_count, 0, sizeof(ref_count));
    ref_count[1] = 1;
    ushort parent_map[sbstruct.ninodes];
    memset(parent_map, 0, sizeof(parent_map));
    ushort child_map[sbstruct.ninodes];
    memset(child_map, 0, sizeof(child_map));
    child_map[1] = 1;
    

    for (uint i = 0; i < sbstruct.ninodes; ++i) 
	{
        if (inode_tbl[i].type == 0) 
		{
            continue;
        } 
		else if (inode_tbl[i].type > T_DEV) 
		{
            fprintf(stderr, "%s\n", ERROR_INODE);
            exit(1);
        } 
		else 
		{
            if (i == 1 && inode_tbl[i].type != T_DIR) 
			{
                fprintf(stderr, "%s\n", ERROR_ROOT);
                exit(1);
            }
            
			for (uint j = 0; j < NDIRECT; ++j) 
			{
                if (inode_tbl[i].addrs[j] == 0) 
				{
                    // break;
                } 
				else if (inode_tbl[i].addrs[j] > sbstruct.size || inode_tbl[i].addrs[j] < data_region) 
				{
                    fprintf(stderr, "%s\n", ERROR_DIR_ADDR);
                    exit(1);
                } 
				else if (tester(bitmap_rec, inode_tbl[i].addrs[j]) != 0) 
				{
                    fprintf(stderr, "%s\n", ERROR_MUL);
                    exit(1);
                } 
				else if (inode_tbl[i].type == T_DIR) 
				{
                    fseek(image, inode_tbl[i].addrs[j] * BSIZE, SEEK_SET);
                    read = fread(buffer, 1, BSIZE, image);
                    struct dirent* dirent = (struct dirent*)buffer;
                    uint k = 0;
                    if (j == 0) 
					{
                        if (strcmp(dirent[0].name, ".") != 0 || strcmp(dirent[1].name, "..") != 0) 
						{
                            fprintf(stderr, "%s\n", ERROR_FMT);
                            exit(1);
                        }
                        if (i == 1 && dirent[1].inum != 1) 
						{
                            fprintf(stderr, "%s\n", ERROR_ROOT);
                            exit(1);
                        }
                        parent_map[i] = dirent[1].inum;
                        k = 2;
                    }
                    for (; k < DPB; ++k) 
					{
                        if (dirent[k].inum != 0) 
						{
                            ref_count[dirent[k].inum] += 1;
                            child_map[dirent[k].inum] = i;
                        }
                    }
                }
            }
            if (inode_tbl[i].addrs[NDIRECT] == 0) 
			{
				//do nada
            } 
			else if (inode_tbl[i].addrs[NDIRECT] > sbstruct.size|| inode_tbl[i].addrs[NDIRECT] < data_region) 
			{
				fprintf(stderr, "%s\n", ERROR_IND_ADDR);
                exit(1);
            } 
			else if (tester(bitmap_rec, inode_tbl[i].addrs[NDIRECT]) != 0) 
			{
                fprintf(stderr, "%s\n", ERROR_IMUL);
                exit(1);
            } 
			else 
			{
                uint indirect_blk[NINDIRECT];
                fseek(image, inode_tbl[i].addrs[NDIRECT] * BSIZE, SEEK_SET);
                read = fread(indirect_blk, 1, BSIZE, image);
                for (uint j = 0; j < NINDIRECT; ++j) 
				{
                    if (indirect_blk[j] == 0) 
					{
                        break;
                    } 
					else if (indirect_blk[j] > sbstruct.size|| indirect_blk[j] < data_region) 
					{
                        fprintf(stderr, "%s\n", ERROR_IND_ADDR);
                        exit(1);
                    } 
					else if (tester(bitmap_rec, indirect_blk[j]) != 0) 
					{
                        fprintf(stderr, "%s\n", ERROR_IMUL);
                        exit(1);
                    } 
					else if (inode_tbl[i].type == T_DIR) 
					{
                        fseek(image, inode_tbl[i].addrs[j] * BSIZE, SEEK_SET);
                        read = fread(buffer, 1, BSIZE, image);
                        struct dirent* dirent = (struct dirent*)buffer;
                        for (uint k = 0; k < DPB; ++k) 
						{
                            if (dirent[k].inum != 0) 
							{
                                ref_count[dirent[k].inum] += 1;
                                child_map[dirent[k].inum] = i;
                            }
                        }
                    }
                }
            }
        }
    }
    
    // Compare Data Bitmap
    for (uint i = 0; i < bitmap_blks * BSIZE; ++i) 
	{
        if (bitmap_rec[i] == bitmap[i]) 
		{
            continue;
        } 
		else if (bitmap_rec[i] > bitmap[i]) 
		{
            fprintf(stderr, "%s\n", ERROR_FREE);
            exit(1);
        } 
		else 
		{
            fprintf(stderr, "%s\n", ERROR_BITMAP);
            exit(1);
        }
    }
    
    // Enumerate Inodes - Second Pass
    for (uint i = 0; i < sbstruct.ninodes; ++i) 
	{
        if (inode_tbl[i].type == 0) 
		{
            if (ref_count[i] > 0) 
			{
                fprintf(stderr, "%s\n", ERROR_REFER_MISS);
                exit(1);
            }
        } 
		else 
		{
            if (ref_count[i] == 0) 
			{
                fprintf(stderr, "%s\n", ERROR_INODE_MISS);
                exit(1);
            }
            if (inode_tbl[i].type == T_FILE) 
			{
                if (inode_tbl[i].nlink != ref_count[i]) 
				{
                    fprintf(stderr, "%s\n", ERROR_FILE);
                    exit(1);
                }
            }
            else if (inode_tbl[i].type == T_DIR) 
			{
                if (ref_count[i] > 1) 
				{
                    fprintf(stderr, "%s\n", ERROR_DIRECTORY);
                    exit(1);
                }
                if (parent_map[i] != child_map[i]) 
				{
                    fprintf(stderr, "%s\n", ERROR_PARENT);
                    exit(1);
                }
            }
        }
    }
    
    fclose(image);
    return 0;
}

