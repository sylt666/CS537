#include "types.h"
#include "stat.h"
#include "user.h"


int main(int argc, char *argv[]){

	printf(1, "read count =  %d\n", getreadcount());
	read(0,0,0);		
	printf(1, "read count =  %d\n", getreadcount());
	exit();


}
