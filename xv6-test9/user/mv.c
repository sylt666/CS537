#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"
#include "fs.h"

#define BUF_SIZE 256

void ls(char *path);
char* fmtname(char *path);

int main(int argc, char *argv[])
{
	if (argc !=3)
	{
		printf(1, "please input the command as [mv src_file dest_file]\n");
		exit();
	}
	
	int fd_src = open(argv[1], O_RDONLY);
	if (fd_src == -1)
	{
		printf(1, "open source file failed\n");
		exit();
	}
	
	struct stat st;
	fstat(fd_src, &st);
	if (st.type == T_DIR)
	{
		printf(1, "source file is a directory, the files in that directory is:\n");
		ls(argv[1]);
		printf(1, "the program can't open the file in that directory after list them.\n");
		printf(1, "So, I'm sorry that you have to move them one by one.\n");
		exit();
	}

	char com[128] = {};
	strcpy(com, argv[2]);
	int len1 = strlen(argv[1]);
	int len2 = strlen(argv[2]);
	if (argv[2][len2-1] == '/')
	{
		int i = len1 - 1;
		for (; i >= 0; i--)
			if (argv[1][i] == '/')
				break;
		i++;
		strcpy(&com[len2], &argv[1][i]);
	}
	
	int fd_dest = open(com, O_WRONLY|O_CREATE);
	if (fd_dest == -1)
	{
		printf(1, "create dest file failed\n");
		exit();
	}
	
	char buf[BUF_SIZE] = {};
	int len = 0;
	while((len = read(fd_src, buf, BUF_SIZE)) > 0)
		write(fd_dest, buf, len);
	
	close(fd_src);
	close(fd_dest);
	
	if(unlink(argv[1]) < 0)
		printf(1, "delete source file failed\n");
	
	exit();
}

char* fmtname(char *path)
{
	static char buf[DIRSIZ+1];
	char *p;
	
	for(p=path+strlen(path); p >= path && *p != '/'; p--)
		;
	p++;
	
	if(strlen(p) >= DIRSIZ)
		return p;
	memmove(buf, p, strlen(p));
	memset(buf+strlen(p), ' ', DIRSIZ-strlen(p));
	return buf;
}

void ls(char *path)
{
	char buf[512], *p;
	int fd;
	struct dirent de;
	struct stat st;
	
	if((fd = open(path, 0)) < 0)
	{
		printf(2, "cannot open %s\n", path);
		return;
	}
	
	if(fstat(fd, &st) < 0)
	{
		printf(2, "cannot stat %s\n", path);
		close(fd);
		return;
	}
	
	switch(st.type)
	{
		case T_FILE:
		printf(1, "name = %s, type = file, size = %d\n", fmtname(path), st.size);
		break;
		
		case T_DIR:
		if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf)
		{
			printf(1, "path too long\n");
			break;
		}
		strcpy(buf, path);
		p = buf+strlen(buf);
		*p++ = '/';
		
		while(read(fd, &de, sizeof(de)) == sizeof(de))
		{
			if(de.inum == 0)
				continue;
			memmove(p, de.name, DIRSIZ);
			p[DIRSIZ] = 0;
			if(stat(buf, &st) < 0)
			{
				printf(1, "cannot stat %s\n", buf);
				continue;
			}
			printf(1, "name = %s, type = directory, size = %d\n", fmtname(buf), st.size);
		}
		break;
	}
	close(fd);
}