#include "types.h"
#include "stat.h"
#include "user.h"

#define USERTOP 0xA0000
#define PGSIZE 4096

void
testPassed(void)
{
  printf(1, "....Passed\n");
}

void
testFailed(void)
{
  printf(1, "....FAILED\n");
}

void expectedVersusActualNumeric(char* name, int expected, int actual)
{
  printf(1, "      %s expected: %d, Actual: %d\n", name, expected, actual);
}

void
whenRequestingSharedMemory_ValidAddressIsReturned(void)
{
  printf(1, "Test: whenRequestingSharedMemory_ValidAddressIsReturned...");
  char* sharedPage = shmem_access(0);
  char* highestPage =       (char*)(USERTOP - PGSIZE);
  char* secondHighestPage = (char*)(USERTOP - 2*PGSIZE);
  char* thirdHighestPage =  (char*)(USERTOP - 3*PGSIZE);
  char* fourthHighestPage = (char*)(USERTOP - 4*PGSIZE);

  if(sharedPage == highestPage ||
     sharedPage == secondHighestPage ||
     sharedPage == thirdHighestPage ||
     sharedPage == fourthHighestPage) {
    testPassed();
  } else {
    testFailed();
  }
}

void
afterRequestingSharedMemory_countReturns1()
{
  printf(1, "Test: afterRequestingSharedMemory_countReturns1...");
  char* sharedPage = shmem_access(0);
  int count = shmem_count(0);

  if(count == 1) {
    testPassed();
  } else {
    testFailed();
    expectedVersusActualNumeric("'count'", 1, count);
  }

  // silence the error about unused variable
  sharedPage = sharedPage + 0;
}

void
whenProcessExits_SharedPageIsFreed()
{
  printf(1, "Test: whenProcessExits_SharedPageIsFreed...");
  // printf(1, "forking...");
  int pid = fork();

  //printf(1, "waiting for processes");
  if(pid == 0){
    //printf(1, "child process");
    // in child
    char* sharedPage = shmem_access(0);
    //printf(1, "just accessed page");
    sharedPage[0] = 42;
    //printf(1, "just tried to set a value");
    exit();
  } else {
    // in parent
    wait();
    char* parentsSharedPage = shmem_access(0);
    if(parentsSharedPage[0] != 42){
      testPassed();
    } else {
      // should be garbage value after being freed, but it's still 42
      testFailed();
      expectedVersusActualNumeric("'parentsSharedPage[0]'", 1, parentsSharedPage[0]);
    }
  }
}

void
whenSharingAPageBetween2Processes_countReturns2()
{
  printf(1, "Test: whenSharingAPageBetween2Processes_countReturns2...");

  char* sharedPage = shmem_access(0);
  sharedPage = sharedPage + 0;  // silence unused variable error

  int pid = fork();

  if(pid == 0){
    // in child
    char* childsSharedPage = shmem_access(0);
    childsSharedPage = childsSharedPage + 0;  // silence unused variable error

    int count = shmem_count(0);
    if(count != 2){
      testFailed();
      expectedVersusActualNumeric("'count'", 2, count);
    }

    exit();
  } else{
    // in parent
    wait(); // wait for child to exit
    int parentsCount = shmem_count(0);
    if(parentsCount != 1){
      testFailed();
      expectedVersusActualNumeric("'parentsCount'", 1, parentsCount);
    }
  }

  testPassed();
}

void
whenProcessExists_countReturns0()
{
  printf(1, "Test: whenProcessExists_countReturns0...");

  int pid = fork();

  if(pid == 0){
    // in child
    char* sharedPage = shmem_access(0);
    sharedPage = sharedPage + 0;  // silence unused variable error
    exit();
  } else {
    // in parent
    wait();
    int count = shmem_count(0);

    if(count != 0){
      testFailed();
      expectedVersusActualNumeric("'count'", 0, count);
    } else {
      testPassed();
    }

  }
}

void
beforeRequestingSharedMemory_countReturns0()
{
  printf(1, "Test: beforeRequestingSharedMemory_countReturns0...");

  int count = shmem_count(0);

  if(count != 0){
    testFailed();
    expectedVersusActualNumeric("'count'", 0, count);
  } else {
    testPassed();
  }
}

void
requestSharedMemoryFromAll4Pages()
{

  printf(1, "Test: requestSharedMemoryFromAll4Pages...\n");

  char* sharedPage0 = shmem_access(0);
  sharedPage0 = sharedPage0 + 0;
  char* sharedPage1 = shmem_access(1);
  sharedPage1 = sharedPage1 + 0;
  char* sharedPage2 = shmem_access(2);
  sharedPage2 = sharedPage2 + 0;
  char* sharedPage3 = shmem_access(3);
  sharedPage3 = sharedPage3 + 0;

  int pid = fork();

  if(pid == 0){

      char* childSharedPage0 = shmem_access(0);
      childSharedPage0 = childSharedPage0 + 0;
      char* childSharedPage1 = shmem_access(1);
      childSharedPage1 = childSharedPage1 + 0;
      char* childSharedPage2 = shmem_access(2);
      childSharedPage2 = childSharedPage2 + 0;
      char* childSharedPage3 = shmem_access(3);
      childSharedPage3 = childSharedPage3 + 0;

      int count0 = shmem_count(0);
      int count1 = shmem_count(1);
      int count2 = shmem_count(2);
      int count3 = shmem_count(3);

      if (count0 != 2){
          testFailed();
          expectedVersusActualNumeric("Expected 'count0' to be 2", 2, count0);
      } else if (count1 != 2) {
          testFailed();
          expectedVersusActualNumeric("Expected 'count1' to be 2", 2, count1);
      } else if (count2 != 2) {
          testFailed();
          expectedVersusActualNumeric("Expected 'count2' to be 2", 2, count2);
      } else if (count3 != 2) {
          testFailed();
          expectedVersusActualNumeric("Expected 'count3' to be 2", 2, count3);
      } else {
          testPassed();
      }

      exit();

  } else {
      wait();
  }

}

void
whenSharingAPage_ParentSeesChangesMadeByChild(int page_number)
{
  printf(1, "Test: whenSharingAPage_ParentSeesChangesMadeByChild for page %d...", page_number);
  char* sharedPage = shmem_access(page_number);
  sharedPage[0] = 42;

  int pid = fork();

  if(pid == 0){
    // in child
    char* childsSharedPage = shmem_access(page_number);
    childsSharedPage[0] = childsSharedPage[0] + 1;
    exit();
  } else {
    // in parent
    wait(); // wait for child to terminate
    if(sharedPage[0] == 43){
      testPassed();
    } else {
      testFailed();
      expectedVersusActualNumeric("'sharedPage[0]'", 43, sharedPage[0]);
    }
  }
}

void
shmemAccessReturnsNullForInvalidPages()
{

  printf(1, "Test: illegal args to shmem_access should return NULL\n");

  if (shmem_access(-1) != NULL){
    printf(1, "shmem_access to -1 shoud return NULL");
    testFailed();
  } else if (shmem_access(4) != NULL) {
    printf(1, "shmem_access to 4 shoud return NULL");
    testFailed();
  } else  {
      testPassed();
  }

}

void
shmemCountReturnsNegative1ForInvalidPages()
{
    printf(1, "Test: illegal args to shmem_count should return -1\n");

    if ((int) shmem_count(-1) != -1){
      printf(1, "shmem_access to -1 shoud return -1");
      testFailed();
  } else if ((int) shmem_count(4) != -1) {
      printf(1, "shmem_access to 4 shoud return -1");
      testFailed();
    } else  {
        testPassed();
    }

}


int
main(void)
{
  int pid;

  // we fork then run each test in a child process to keep the main process
  // free of any shared memory
  pid = fork();
  if(pid == 0){
    whenRequestingSharedMemory_ValidAddressIsReturned();
    exit();
  }
  wait();

  pid = fork();
  if(pid == 0){
    afterRequestingSharedMemory_countReturns1();
    exit();
  }
  wait();

  // pid = fork();
  // if(pid == 0){
  //   whenSharingAPage_ParentSeesChangesMadeByChild();
  //   exit();
  // }
  // wait();

  pid = fork();
  if(pid == 0){
    whenSharingAPageBetween2Processes_countReturns2();
    exit();
  }
  wait();

  pid = fork();
  if(pid == 0){
    whenProcessExits_SharedPageIsFreed();
    exit();
  }
  wait();

  pid = fork();
  if(pid == 0){
    whenProcessExists_countReturns0();
    exit();
  }
  wait();

  pid = fork();
  if(pid == 0){
    beforeRequestingSharedMemory_countReturns0();
    exit();
  }
  wait();

  pid = fork();
  if(pid == 0){
      requestSharedMemoryFromAll4Pages();
      exit();
  }
  wait();

  pid = fork();
  if(pid == 0) {
      int i;
      for (i = 0; i < 4; i++){
        whenSharingAPage_ParentSeesChangesMadeByChild(i);
      }
      exit();
  }
  wait();

  pid = fork();
  if(pid == 0) {
      shmemAccessReturnsNullForInvalidPages();
      exit();
  }
  wait();


  pid = fork();
  if(pid == 0) {
      shmemCountReturnsNegative1ForInvalidPages();
      exit();
  }
  wait();


  exit();
}
