#include "mapreduce.h"
#include <stdbool.h>
#include <pthread.h>
#include <sys/sysinfo.h>
#include <string.h>
#include <sys/uio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <semaphore.h>
#define SIZE 100000000
#define STR_SIZE 100


pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t locks[60];



int reducers;
Partitioner partit;
Mapper mapp;
Reducer reducee;





typedef struct reduce_data
{
    int part_num;
}reduce_data;

//type for ordering the files
typedef struct map_data
{
    int fileNum;
    char** filenames;
}map_data;

//type for each node
typedef struct hashnode
{
    char * keys;
    char * val;
}hashnode;


//the partitions sent to the reducers
hashnode*** partitions;

//partition size counter
int *cntr;

//getter position
int *pos;

// second index of each partition
int* index_2;

//used by emit to add node to the partition array
void insert(hashnode * new_node, unsigned long part_num)
{
         partitions[part_num][cntr[part_num]] = new_node;
         cntr[part_num]++;
}

//emit used in the mapping
void MR_Emit(char *key, char *value)
{
	//partition number
	unsigned long part_num = partit(key, reducers);

	//new node that will be added
	hashnode* new_node = (hashnode*) malloc(sizeof(hashnode*));
	new_node->keys = (char*)malloc(100);
	strcpy(new_node->keys, key);
	new_node->val = (char*)malloc(100);
	strcpy(new_node->val, value);


	//lock
        pthread_mutex_lock(&locks[part_num]);

	//insert the new node into the corresponding partition array
	insert(new_node,part_num);

	//unlock
        pthread_mutex_unlock(&locks[part_num]);


}

int compare_struct(const void* a, const void* b)
{
           hashnode* x = *(hashnode**)a;
           hashnode* y = *(hashnode**)b;
           return strcmp(x->keys, y->keys);
}

char * get_next_index(char* key, int partition_number){
	
	if (pos[partition_number] >= cntr[partition_number]){
		return NULL;
	}
	if (strcmp(partitions[partition_number][pos[partition_number]]->keys, key) == 0){
		int tmp = pos[partition_number];
		pos[partition_number]++;
		return partitions[partition_number][tmp]->val;	
	}
	return NULL;
}

char * get_first_index(char * key, int partition_number){
	
	if (pos[partition_number] >= cntr[partition_number]){
		return NULL;
	}
	if (pos[partition_number] < index_2[partition_number]){
		int tmp = pos[partition_number];
		pos[partition_number]++;
		return partitions[partition_number][tmp]->val;
	}
	return NULL;
}

void * reduce_thr(void * data){
	reduce_data * dat = (reduce_data*) data;
	int partition_number = dat->part_num;
	
	if (cntr[partition_number] == 0){
		return NULL;
	}
	qsort((void*)&partitions[partition_number][0], cntr[partition_number],sizeof(hashnode*),compare_struct);

	char* pass_key = malloc(STR_SIZE);
	strcpy(pass_key, partitions[partition_number][0]->keys);
	int next = 1;
	while((strcmp(partitions[partition_number][next]->keys, pass_key) == 0)){
		next++;
	}
	
	index_2[partition_number] = next;
	reducee(pass_key, get_first_index, partition_number);
	
	for(int i = next; i < cntr[partition_number]; i++){

		if(strcmp(partitions[partition_number][i]->keys, pass_key) != 0){
			strcpy(pass_key, partitions[partition_number][i]->keys);
			reducee(pass_key, get_next_index, partition_number);		
		}	
	}
	
	return NULL;
	
}

//default part
unsigned long MR_DefaultHashPartition(char *key, int num_partitions) {
    unsigned long hash = 5381;
    int c;
    while ((c = *key++) != '\0')
        hash = hash * 33 + c;
    return hash % num_partitions;
}


int file_index = 0;

//thread method
void * map_thr(void * data)
{
        map_data * datas = (map_data*) data;

        int index;
        while(file_index < datas->fileNum)
        {


            pthread_mutex_lock(&lock);
            index = file_index;
            file_index++;
            pthread_mutex_unlock(&lock);


		char *file = (char*) malloc(100);


		strcpy(file,datas->filenames[index]);
	printf("hello");
            mapp(file);

        }
	


        return NULL;
}


//where everything goes on
void MR_Run(int argc, char *argv[], Mapper map, int num_mappers, Reducer reduce, int num_reducers, Partitioner partition)
{
 
      int args = argc - 1;






      FILE *fp [args];
      char** filename = malloc(sizeof(char*)*args);
      long total_file_size = 0;
      
      for(int i = 0;i<args;i++)
      {
           filename[i] = malloc(100);
      }

      for(int i = 1; i < argc; i++)
      {
            strcpy(filename[i - 1], argv[i]);
      }


      long *file_size = malloc(sizeof(int) * args);

      for(int i = 0; i < args; i++)
      {
          fp[i] = fopen(argv[i + 1], "r");
          fseek(fp[i], 0L, SEEK_END);
          file_size[i] = ftell(fp[i]);
          fseek(fp[i], 0L, SEEK_SET);
          total_file_size += file_size[i];
      }

	
      for(int i = 0; i < args; i++)
      {
          for(int j = 0; j < args - i -1; j++)
          {
              if(file_size[j] > file_size[j+1])
              {
                  
                  long temp = file_size[j];
                  file_size[j] = file_size[j+1];
                  file_size[j+1] = temp;
                  
                  char*  tmp = malloc(100);
                  strcpy(tmp, filename[j]);
                  strcpy(filename[j], filename[j+1]);
                  strcpy(filename[j+1], tmp);
                  free(tmp);
              }
          }
      }

	reducee = reduce;
	mapp = map;
      partit = partition;
      reducers = num_reducers;
	index_2 = malloc(num_reducers * sizeof(int));
	cntr = malloc(reducers * sizeof(int));
	for(int i = 0;i < reducers;i++)
	{
		cntr[i] = 0;
		index_2[i] = 1;
	}
	pos = malloc(reducers * sizeof(int));

	partitions = (hashnode***) malloc(sizeof(hashnode**) * reducers);
	
      	for(int i = 0;i < reducers;i++)
      	{
		//emit lock declaration
		pthread_mutex_init(&locks[i], NULL);	

		//partition array initalization
		partitions[i] = (hashnode**) malloc(sizeof(hashnode*) * SIZE);
	
		//intialize the partition counters
		cntr[i] = 0;

	}

      	map_data* data =(map_data*) malloc(sizeof(map_data));

      	data->fileNum = args;

	data->filenames = (char**) malloc(sizeof(char*)*args);
      	for (int i = 0; i < args; i++)
      	{
		data->filenames[i] = (char*) malloc(100); 
		strcpy(data->filenames[i], filename[i]);

      	}



      // create threads for mapping
      pthread_t mappers[num_mappers];
      pthread_t threads;
      int save;


      for (int i = 0; i < num_mappers; i++)
      {
          save = pthread_create(&threads, NULL, map_thr, data);
          if (save == 0)
          {
              mappers[i] = threads;
          }
      }

	for (int i = 0; i < num_mappers; i++)
	{
        	   pthread_join(mappers[i],NULL);
	}



	 // create threads for reducing
      reduce_data dat[num_reducers];
      for(int i = 0; i < num_reducers; i++)
      {
          dat[i].part_num = i;
      }


      pthread_t reducers[num_reducers];
      for(int i = 0; i < num_reducers; i++)
      {
          save = pthread_create(&threads, NULL, reduce_thr, &dat[i]);
          if(save == 0)
          {
              reducers[i] = threads;
          }
      }

      for(int i = 0 ; i < num_reducers; i++)
      {
          pthread_join(reducers[i], NULL);
      }






    
    	for(int i = 0;i < num_reducers;i++)
	{
		for(int j = 0; j < cntr[i];j++)
		{
			free(partitions[i][j]->keys);
			free(partitions[i][j]->val);
			free(partitions[i][j]);
		}
		free(partitions[i]);
	}

}

