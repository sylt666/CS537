#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include "mapreduce.h"

//#define hash_sz 20
#define p_threshold 0.8
#define load_factor 1

typedef struct Item{
   char *key;
   struct Vnode* vn;
   struct Item *next;     
}Item;

typedef struct Vnode{
   char* value;
   struct Vnode *next;     
}Vnode;

typedef struct Backet_info{
   struct Item *head;
   struct Item *tail;     
}Backet_info;

typedef struct Partition_info{
   int size;
   int current;     
}Partition_info;

typedef struct Key_iterator{
	int curridx;
	char *key;
	struct Vnode* vn;     
}Key_iterator;

typedef struct Map_warpper_args{
	Mapper m;
	char *filename;
}Map_warpper_args;

typedef struct Reduce_wrapper_args{
	Reducer r;
	Getter get_func;
	int partition_number;
}Reduce_wrapper_args;

/*
typedef struct Reducer_Args{
   char *key;   
   Getter get_func;
   int partition_number;
}Reducer_Args;
*/


//global variables
Item **hash_array;
Backet_info *backet_info_array;
Item **partition_array;
Partition_info *partition_array_info;
Key_iterator *iterator_info;

//concurrent variables
//pthread_cond_t hash_cv = PTHREAD_COND_INITIALIZER;
pthread_mutex_t hash_lock = PTHREAD_MUTEX_INITIALIZER;

int num_partitions;
int num_keys;
int hash_sz = 41;
int partition_sz = 41;
int num_rehash;

void rehash();
void print_hashtable();
unsigned long MR_DefaultHashPartition(char *key, int num_partitions);

//Hash table implementation

//hash function
unsigned long
hash(char *str, int sz)
{
    unsigned long hash = 5381;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash % sz;
}


void MR_Emit(char *key, char *value){


	pthread_mutex_lock(&hash_lock); 
	//get hash code
	int hashcode = (int)hash((char*)key, hash_sz);
	//printf("hashcode: %d\n", hashcode);
	
	if(hash_array[hashcode] == NULL){
		//no list exist, create a new head
		//create node for insertion
		
		
	  	Item *item = (Item*) malloc(sizeof(Item));
		Vnode *vnode = (Vnode*)malloc(sizeof(Vnode));
		vnode->value = strdup(value);
		vnode->next = NULL;
		item->vn = vnode;
		item->key = strdup(key);
		item->next = NULL;
		
		backet_info_array[hashcode].head = item;
		backet_info_array[hashcode].tail = item;
		hash_array[hashcode] = item;
		num_keys++;
  }
	else{
		//collision
		Item *curr = hash_array[hashcode];
		int key_exist = 0;
		while(curr != NULL){
			//if(num_rehash == 1){
	    	//	printf("insert collision\n");
	    	//}
			//if the key already exist
			//insert into head of vn list
			if(strcmp(curr->key, key) == 0){
				Vnode *vnode = (Vnode*)malloc(sizeof(Vnode));
				vnode->value = strdup(value);
				vnode->next = curr->vn;
				curr->vn = vnode;
				key_exist = 1;
				break;
			}
			else{
				curr = curr->next;
			}
		}
		if(key_exist == 0){
			//key does not exist, append on tail of the backet
			//create node for insertion
			Item *item = (Item*) malloc(sizeof(Item));
			Vnode *vnode = (Vnode*)malloc(sizeof(Vnode));
			vnode->value = strdup(value);
			vnode->next = NULL;
			item->vn = vnode;
			item->key = strdup(key);
			item->next = NULL;
			backet_info_array[hashcode].tail->next = item;
			backet_info_array[hashcode].tail = item;
			num_keys++;
		}
	}
	//printf("num_keys: %d\n", num_keys);
	//printf("left: %d, right: %d\n", num_keys, hash_sz * load_factor);
	if(num_keys > hash_sz * load_factor){
	//rehash needed

	    num_rehash += 1;
	    //printf("rehash # %d\n", num_rehash);
	    

		rehash();


		//printf("end of rehash # %d\n", num_rehash);
			    

		
	}
	pthread_mutex_unlock(&hash_lock); 
  
}

void rehash(){
	int old_hash_sz = hash_sz;
	hash_sz = hash_sz * 2 + 1;
	
	//printf("old_hash_sz: %d, hash_sz : %d\n", old_hash_sz, hash_sz);
	
	//allocate space for new hash table
	Item **temp_hash_array = malloc(hash_sz * sizeof(Item *));
	Backet_info *temp_backet_info_array = malloc(hash_sz * sizeof(Backet_info));
	
	if( temp_hash_array == NULL || temp_backet_info_array == NULL)
	{
		printf("rehash malloc fail\n");
		exit(1);
	}
	
	for(int i = 0; i < hash_sz; i++){
		temp_hash_array[i] = NULL;
		*temp_backet_info_array = (Backet_info){NULL, NULL};
	}
		
	//traverse the hash table
	//insert every node in old table to new table 
	for(int i = 0; i < old_hash_sz; i++){
		Item *curr = hash_array[i];
		while(curr != NULL){
		 	
		 	
		 	//forward head pointer of the old list
		 	backet_info_array[i].head = curr->next;
			int hashcode = (int)hash((char*)curr->key, hash_sz);
			
			//when slot in new table empty
			if(temp_hash_array[hashcode] == NULL){
				
				temp_hash_array[hashcode] = curr;

				temp_backet_info_array[hashcode].head = curr;
				temp_backet_info_array[hashcode].tail = curr;
				curr->next = NULL;
			}
			
			//when slot not empty
			else{
				temp_backet_info_array[hashcode].tail->next = curr;
				temp_backet_info_array[hashcode].tail = curr;
				curr->next = NULL;
			}
			curr = backet_info_array[i].head;
		}
	}

	//free old hash table
	free(hash_array);
	free(backet_info_array);
	backet_info_array = temp_backet_info_array;
	hash_array = temp_hash_array;
	
	//printf("end of rehash\n");
}



void hash_partition(Partitioner partition){
	//TODO: untested
	
	//allocate space
	
	printf("hash_partition\n");
	partition_array = (Item **) malloc(num_partitions * sizeof(Item *));	
	partition_array_info = malloc(num_partitions * sizeof(Partition_info));
	
	if (partition_array == NULL || partition_array_info == NULL){
		printf("hash_partition malloc fail\n");
		exit(1);
	}
	
	printf("hash_partition_1\n");
	for(int i = 0; i < num_partitions; i++)
	{
		partition_array[i] = (Item *) malloc(partition_sz * sizeof(Item));
		*partition_array[i] = (Item){NULL, NULL, NULL};
		partition_array_info[i] = (Partition_info){partition_sz, 0};
	}
	
	//traverse the hashtable
	for(int i = 0; i < hash_sz; i++){
		
		//printf("hash_partition_1.5 i = %d\n", i);
		Item *curr = hash_array[i];
		
		//for each item in the list of hashtable
		while(curr != NULL){
			//printf("key :%s, num_partitions: %d\n",curr->key, num_partitions);
			
			//locate the partition
			int p = (int)partition(curr->key, num_partitions);
			int cur_keys = partition_array_info[p].current;
			
			//append the item
			partition_array[p][cur_keys] = *curr;
			partition_array_info[p].current++;
			
			//printf("hash_partition_2\n");
			
			//expend partition if needed
			if(partition_array_info[p].current > p_threshold * partition_array_info[p].size){
				
				//printf("resize partition\n");
				
				partition_array[p] = realloc(partition_array[p], 2 * partition_array_info[p].size * sizeof(Item));
				partition_array_info[p].size *= 2;
			}
			
			curr = curr->next;
		}
	}
	
	
		
}

Item *search_partition(char *key, int partition){
	int current = partition_array_info[partition].current;
	for(int j = 0; j < current; j++){
		if(strcmp(key, partition_array[partition][j].key) == 0){
			Item *result = &partition_array[partition][j];
			return result;
		}
	}
	return NULL;
}


/**
helper functions for freeing
**/
//free hashtable
void free_hashtable(){
	for(int i = 0; i < hash_sz; i++){
		Item *curr = hash_array[i];
		
		while(curr != NULL){
		
			Item *tmp = curr;
			/*
			Vnode *v = curr->vn;
			
			while(v != NULL){
			
                Vnode *vtmp = v;
                v = v->next;
				free(vtmp);
			}
			*/
			curr = curr->next;
			free(tmp);
		}
	}
    free(hash_array);
    free(backet_info_array);
}

void free_partition_table(){
	
	for(int i = 0; i < num_partitions; i++){
		int current = partition_array_info[i].current;
		for(int j = 0; j < current; j++){
			
			Vnode *v = partition_array[i][j].vn;
			Vnode *vtmp = v;
			
			while(v != NULL){
				v = v->next;
				free(vtmp);
			}
		}
		free(partition_array[i]);
	}
	free(partition_array);
	free(partition_array_info);
	
}


/**
	Printer functions
**/
void print_hashtable(){

	//if(hash_array[0] == NULL){
		//printf("hashtable empty\n");
		//return;
	//}
	for(int i = 0; i < hash_sz; i++){
		Item *curr = hash_array[i];
		if(curr == NULL){
			printf("i = %d, NULL\n", i);
		}
		int j = 0;
		while(curr != NULL){
			for(int k = 0; k < j; k++){
				printf("  ");
			}
			j++;
			printf("i = %d, key = %s, value = ", i, curr->key);
			Vnode *v = curr->vn;
			while(v != NULL){
				printf("%s, ", v->value);
				v = v->next;
			}
			printf("\n");
			curr = curr->next;
		}
	}
}

void print_partition(){

	//if(partition_array[0] == NULL){
		//printf("partition table empty\n");
		//return;
	//}
	
	for(int i = 0; i < num_partitions; i++){
		int current = partition_array_info[i].current;
		for(int j = 0; j < current; j++){
			printf("partition= %d, %dth item, key = %s, value = ", i, j, partition_array[i][j].key);
			Vnode *v = partition_array[i][j].vn;
			
			//print values
			while(v != NULL){
				printf("%s, ", v->value);
				v = v->next;
			}
			printf("\n");
		}
	}

}


/**
	Helper function for qsort
**/
int struct_strcmp(const void *a, const void *b){
	Item *item_a = (Item *)a;
	Item *item_b = (Item *)b;
	return strcmp(item_a->key, item_b->key);
}

/**
	Default partition function
**/
unsigned long MR_DefaultHashPartition(char *key, int num_partitions) {
    unsigned long hash = 5381;
    int c;
    while ((c = *key++) != '\0')
        hash = hash * 33 + c;
    return hash % num_partitions;
}

char *get_next (char *key, int partition_number){
	
	//init iterator for the partition if needed
	if(iterator_info[partition_number].key == NULL && iterator_info[partition_number].vn == NULL){
		//search the partition and find the key
		Item *itm = search_partition(key, partition_number);
		iterator_info[partition_number].key = key;
		
		char *temp_value = strdup(itm->vn->value);
		iterator_info[partition_number].vn = itm->vn->next;
		iterator_info[partition_number].curridx = 0;
		return temp_value;
	}
	
	
	if(strcmp(iterator_info[partition_number].key, key) == 0){
		//go to next vn of this key
		if(iterator_info[partition_number].vn == NULL){
			return NULL;
		}
		char *temp_value = strdup(iterator_info[partition_number].vn->value);
		iterator_info[partition_number].vn = iterator_info[partition_number].vn->next;
		return temp_value;
	}
	else{
		//go to next key in this partition
		iterator_info[partition_number].curridx += 1;
		iterator_info[partition_number].vn = partition_array[partition_number][iterator_info[partition_number].curridx].vn;
		iterator_info[partition_number].key = partition_array[partition_number][iterator_info[partition_number].curridx].key;
		char *temp_value = strdup(iterator_info[partition_number].vn->value);
		iterator_info[partition_number].vn = iterator_info[partition_number].vn->next;
		return temp_value;
	}
}

/**
	Mapper and Reducer
**/

/*
void Map(char *file_name) {
    FILE *fp = fopen(file_name, "r");
    assert(fp != NULL);

    char *line = NULL;
    size_t size = 0;
    while (getline(&line, &size, fp) != -1) {
        char *token, *dummy = line;
        while ((token = strsep(&dummy, " \t\n\r")) != NULL) {
            MR_Emit(token, "1");
        }
    }
    free(line);
    fclose(fp);
}

void Reduce(char *key, Getter get_next, int partition_number) {
    int count = 0;
    char *value;
    while ((value = get_next(key, partition_number)) != NULL)
        count++;
    printf("%s %d\n", key, count);
}
*/

/**
	Mapper and Reducer wrapper
**/
void *Map_wrapper(void *arg){
	Map_warpper_args *mwa = (Map_warpper_args *)arg;
	Mapper m = mwa->m;
	char* file = mwa->filename;
	m(file);
	return 0;
}

void *Reduce_wrapper(void *arg){
	Reduce_wrapper_args *rwa = (Reduce_wrapper_args *)arg;
	Reducer r = rwa->r;
	Getter get_func = rwa->get_func;
	int partition_number = rwa->partition_number;
	
	//reduce the partition
	for(int i = 0; i < partition_array_info[partition_number].current; i++){
		r(partition_array[partition_number][i].key, get_func, partition_number);
	}
	return 0;
}



/**
	Mapreduce main
**/

void MR_Run(int argc, char *argv[], 
	    Mapper map, int num_mappers, 
	    Reducer reduce, int num_reducers, 
	    Partitioner partition){
	

	num_partitions = num_reducers;
	
	//init hash table
	hash_array = malloc(hash_sz * sizeof(Item *)); 
	backet_info_array = malloc(hash_sz * sizeof(Backet_info));
	for(int i = 0; i < hash_sz; i++){
		hash_array[i] = NULL;
		*backet_info_array = (Backet_info){NULL, NULL};
	}
	num_keys = 0;
	num_rehash = 0;
	//Mapping
	printf("argc = %d\n", argc);
	printf("argv[0] = %s\n", argv[0]);
	
	pthread_t mappers[argc - 1];
	
	Map_warpper_args *mwa_list = malloc((argc - 1) * sizeof(Map_warpper_args));
	
	for(int i = 1; i < argc; i++){
		
		//Map_warpper_args *mwa = (Map_warpper_args *) malloc(sizeof(Map_warpper_args));
		
		
		char *file = argv[i];
		mwa_list[i-1].m = map;
		mwa_list[i-1].filename = file;
		printf("thread # %d\n", i-1);
		pthread_create(&mappers[i - 1], NULL, Map_wrapper, (void *)(&mwa_list[i-1]));
	}
	
	for(int i = 1; i < argc; i++){
		printf("join thread # %d\n", i-1);
		pthread_join(mappers[i- 1], NULL);
	}
	
	
	
	
	printf("\n------------------------------main mapping hashtable------------------------------------\n");
	//print_hashtable();
	
		
	printf("\n------------------------------main partitiontable------------------------------------\n");
	//alloc space for partitions
	hash_partition(partition);

	//free_hashtable();
	//printf("\n------------------------------after free------------------------------------\n");
	//print_partition();
	

	//Sorting the partition table
	printf("\n------------------------------main sorting partition------------------------------------\n");
	for(int i = 0; i < num_partitions; i++){
		qsort((void *)partition_array[i], partition_array_info[i].current, sizeof(Item), struct_strcmp);
	}
	//print_partition();
	
	//init iterator
	iterator_info = malloc(num_partitions * sizeof(Key_iterator));
	for(int i = 0; i < num_partitions; i++){
		iterator_info[i] = (Key_iterator){0, NULL, NULL};
	}
	
	printf("\n------------------------------main reduce------------------------------------\n");

	//Reducing
	/*
	//single reducer
	for(int i = 0; i < num_partitions; i++){
		for(int j = 0; j < partition_array_info[i].current; j++){
			Reduce(partition_array[i][j].key, get_next, i);
		}
	}
	*/
	
	
	pthread_t reducers[num_reducers];
	
	Reduce_wrapper_args *rwa_list = malloc((num_partitions) * sizeof(Reduce_wrapper_args));
	
	for(int i = 0; i < num_partitions; i++){
		//Reduce_wrapper_args *rwa = (Reduce_wrapper_args *) malloc(sizeof(Reduce_wrapper_args));
		
		
		rwa_list[i].r = reduce;
		rwa_list[i].get_func = get_next;
		rwa_list[i].partition_number = i;
		pthread_create(&reducers[i], NULL, Reduce_wrapper, (void *)(&rwa_list[i]));
	}
	for(int i = 0; i < num_partitions; i++){
		pthread_join(reducers[i], NULL);
	}
	
	printf("\n------------------------------main reduce end------------------------------------\n");

	printf("\n------------------------------main free mwa_list------------------------------------\n");
	
	//free(mwa_list);
	
	printf("\n------------------------------main free rwa_list------------------------------------\n");
	//free(rwa_list);
	
	printf("\n------------------------------main free iterator_info------------------------------------\n");
	//free(iterator_info);
	
	printf("\n------------------------------main free partition_table------------------------------------\n");
	//free_partition_table();
	
	printf("\n------------------------------main free end------------------------------------\n");
	
	
	//print_hashtable();
	//print_partition();
	printf("num_keys: %d\n", num_keys);
	
}

/*
int main(int argc, char *argv[]) {
    MR_Run(argc, argv, Map, 4, Reduce, 4, MR_DefaultHashPartition);
}
*/
