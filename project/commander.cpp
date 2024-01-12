#include <cstdio>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cstdlib>
#include <iostream>
#include <fstream>
using namespace std;

/* master.cpp file */
#define READ_END 0
#define WRITE_END 1

int main() {
  int max = 100, i, c1, stat;
  ifstream fd;

  //pipe variable
  int mcpipe1[2], status;
  //to pass the pipe address to the child process
  char mc0[10], mc1[10], chr;

  //create the pipe
  if (c1 = pipe(mcpipe1)) /* Create a pipe for master and a child process */ {
    perror("unable to create the pipe");
    exit(1);
  }

  //convert the addresses into a character so it can be passed as parameter to the child processes
  sprintf(mc0, "%d", mcpipe1[READ_END]);
  sprintf(mc1, "%d", mcpipe1[WRITE_END]);

  //fork the child process
  if ((c1 = fork()) == -1) /* Create the first child process */ {
    perror("unable to fork child");
    exit(1);
  }
  else if (c1 == 0) {
    /* This is the first child process */
    cout <<"Child pid = "<<getpid()<<endl;

    //exec and pass the parameters to the child
    execl("scheduler", "scheduler", mc0, mc1, NULL);
    exit(1);
  }
  //this code is what the master now executes
  cout <<"master pid = "<<getpid()<<endl;

  //close the read side of the pipe, since we don't read from the child
  close(mcpipe1[READ_END]);

  //read in a single character
  cin >> chr;
  while (chr!='T') {
    cout<<"Master: writing "<<chr<<endl;
    //write to the child process
    write(mcpipe1[WRITE_END], (char *)&chr, sizeof(char));
    if (chr == 'S') {
      int pid, value, run_time;
      cin >> pid;
      cin >> value;
      cin >> run_time;
      //write to the child process
      write(mcpipe1[WRITE_END], (int *)&pid, sizeof(int));
      write(mcpipe1[WRITE_END], (int *)&value, sizeof(int));
      write(mcpipe1[WRITE_END], (int *)&run_time, sizeof(int));
    } else if (chr == 'B') {
      int rid;
      cin >> rid;
      write(mcpipe1[WRITE_END], (int *)&rid, sizeof(int));
    } else if (chr == 'U') {
      int rid;
      cin >> rid;
      write(mcpipe1[WRITE_END], (int *)&rid, sizeof(int));
    } else if (chr == 'C') {
      char cmd;
      int num;
      cin >> cmd;
      cin >> num;
      write(mcpipe1[WRITE_END], (int *)&num, sizeof(int));
      write(mcpipe1[WRITE_END], (int *)&cmd, sizeof(char));
    }
    cin >>chr; //Reading next line

    //sleep for 1 second
    sleep(2);
  }
  write(mcpipe1[WRITE_END], (char *)&chr, sizeof(char));
  cout <<"finished reading commands\n";
  close(mcpipe1[WRITE_END]);
  wait(&status);
  cout << "Child status is "<<WEXITSTATUS(status)<<endl;
  return 0;
}
