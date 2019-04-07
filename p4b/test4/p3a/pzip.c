#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/uio.h>
#include <sys/mman.h>
#include <pthread.h>
#include <sys/sysinfo.h>


pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
char *final_char_list;
int *final_cnt_list;
int final_pos = -1;
int order=0;

typedef struct arg{
        char * section;
      	int num;
	int tot;
	int front;
	int size;
	int sectlen;
}arg;

void * zipper (void * data) {
    
    	int len = 0;
    	char save;
    	char now;
        arg * args = (arg*) data;
    	char *char_list = malloc(sizeof(char) * args->sectlen);
    	int *cnt_list = malloc(sizeof(int) * args->sectlen);

    	int pos = 0;
  
    	save = args->section[0];
	if(args->size == 0)
	{
		return NULL;
	}
    	for(int i = 0; i < args->size;i++) 
	{
        	now = args->section[i];
        
        	if (len == 0) 
		{
            		save = now;
            		++len;
        	}
	        else if (len > 0) 
		{
	            	if (now == save) 
			{
	                	++len;
	            	}
	            	else if (now != save) 
			{
	                	char_list[pos] = save;
	                	cnt_list[pos] = len;
	                	pos++;
	                
	                	len = 1;
	                	save = now;
        	    	}
	        }
        	
	}    	
    	
	char_list[pos] = save;
    	cnt_list[pos] = len;
    	pos++;
	
	

	pthread_mutex_lock(&lock);
	while (order != args->num) 
	{
        	pthread_cond_wait(&cond, &lock);
    	}	
	order++;
	int i=0;
	
	if(final_char_list[final_pos] == char_list[i])
	{
		final_cnt_list[final_pos] += cnt_list[i];
		i++;
		
	}
	
	int a = i;
	for(i=a;i<pos;i++)
	{
			final_pos++;
			final_char_list[final_pos] = char_list[i];
			final_cnt_list[final_pos] = cnt_list[i];
	}
	pthread_cond_broadcast(&cond);    	
	pthread_mutex_unlock(&lock);
	
	return NULL;
}


int main(int argc, char *argv[]){

	
	final_char_list = malloc(sizeof(char) * 999999999);
	final_cnt_list = malloc(sizeof(int) * 999999999);
	
//	final_char_list[0] = 'a';
//	final_cnt_list[0] = 0;
        char *f;
        size_t size;
        struct stat s;
        int fd;
        int status;

        if (argc < 2)
	{
                printf("my-zip: file1 [file2 ...]\n");
                exit(1);
        }
// open files using mmap, stores the char array in f
        for (int m = 1; m < argc; m++)
	{

                fd = open(argv[m],O_RDONLY);
                if (fd < 0) 
		{
                        printf("my-cat: cannot open file\n");
                        exit(1);
                }

                if (fstat(fd, &s) < 0)
		{
                        close(fd);
                        exit(1);
                }

                status = fstat(fd, &s);
                
		if (status < 0)
		{
                        close(fd);
                        exit(1);
                }
                
		size = s.st_size;
		if (size == 0){
			continue;
		}

                f = mmap(0, size, PROT_READ, MAP_PRIVATE, fd, 0);
                
		if (f == MAP_FAILED)
		{
                        close(fd);
                        exit(1);
                }

		int proc_num = get_nprocs();
		pthread_t threads[proc_num];
		arg args[proc_num];
		pthread_t tmp_p;
		int save;
		
		int sect_len;
		
		if(size%proc_num == 0)
		{
			sect_len = size/proc_num;
		}
		else
		{
			sect_len = size/proc_num + 1;
		}
//	printf("%d\n",sect_len);

		int front = 0;
	    	int back;

		for (int i = 0; i < proc_num; i++) 
		{
        	
		// FIXME might not work, try using index logic instead

			if(sect_len*(i+1)<size)
			{
				back = sect_len;
			}
			else
			{
				back = size - sect_len*i;
			}
        		if(back < 0)
			{
				back = 0;
			}	
		        args[i].section = (f+front);
        		args[i].num = i;
	        	args[i].tot = proc_num;
			args[i].front = front;
			args[i].size = back;
			args[i].sectlen = sect_len;
	        	front += back;
  //     			printf("\nfile number > %d thread number > %d\n",m,i); 
		        save =  pthread_create(&tmp_p, NULL, zipper, &(args[i]));
	        
			if (save == 0)
			{
	        		threads[i] = tmp_p;
			}
	    	}
		
		for (int i = 0; i < proc_num; i++) 
		{
	       		pthread_join(threads[i], NULL);
		}
	
		order = 0;	
		close(fd);
	}// new end of argc for loop

	
	char *write_file = malloc(5*(final_pos+1)*sizeof(char));

	
	int position=0;
       for(int i=0;i<=final_pos;i++)
       {		// save count
		
		*((int*)(write_file+position)) = final_cnt_list[i];
		position += 4;
		*(write_file+position) = final_char_list[i];
		position++;
	
       }
	fwrite(write_file, 5*(final_pos+1)*sizeof(char), 1, stdout);

}







