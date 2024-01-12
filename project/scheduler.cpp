#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <iomanip>
#include <cmath>
#include "queue_array.h"
using namespace std;

//PCB struct for keeping track of values specific to each process
struct PcbTable {
  int pid;
  int priority;
  int value;
  int start_time;
  int run_time;
  int cpu_time;
  int quantum;
};
void reporter();
char chr;
int i, max = 100;
int mcpipe2[2], num, totTime = 0;
int runningProcess = -1;
int turnaround = 0;
int finishedProc = 0;
PcbTable processes[100];
//States represented in QueueArrays
QueueArray <int> blockedState(3);
QueueArray <int> blockedState1(3);
QueueArray <int> blockedState2(3);
QueueArray <int> readyState(4);
int main(int argc, char *argv[]) {

  //convert the parameters into the address for the pipe
  mcpipe2[0] = atoi(argv[1]);
  mcpipe2[1] = atoi(argv[2]);

  //read a character from the pipe
  read(mcpipe2[0], &chr, sizeof(char));

  while (chr != 'T') {
    if (chr == 'S') {
      //read a number from the pipe
      int pid, value, run_time;
      read(mcpipe2[0], (int *)&pid, sizeof(pid));
      read(mcpipe2[0], (int *)&value, sizeof(value));
      read(mcpipe2[0], (int *)&run_time, sizeof(run_time));
      processes[pid].pid = pid;
      processes[pid].run_time = run_time;
      processes[pid].priority = 0;
      processes[pid].start_time = totTime;
      processes[pid].cpu_time = 0;
      processes[pid].value = value;
      processes[pid].quantum = 1;
      readyState.Enqueue(pid, processes[pid].priority);
    } else if (chr == 'B') {
      int rid;
      read(mcpipe2[0], (int *)&rid, sizeof(rid));
      //Need to make sure priority doesn't become negative
      if (processes[runningProcess].priority > 0) {
         processes[runningProcess].priority -= 1;
         processes[runningProcess].quantum = pow(2, processes[runningProcess].priority);
      }
      if (rid == 0) {
         blockedState.Enqueue(runningProcess, processes[runningProcess].priority);
      } else if (rid == 1) {
         blockedState1.Enqueue(runningProcess, processes[runningProcess].priority);
      } else if (rid == 2) {
         blockedState2.Enqueue(runningProcess, processes[runningProcess].priority);
      }
      //Since we're blocking the running process, we need to update it
      runningProcess = readyState.Dequeue();
    } else if (chr == 'U') {
      int rid;
      int dequeued;
      read(mcpipe2[0], (int *)&rid, sizeof(rid));
      if (rid == 0) {
         dequeued = blockedState.Dequeue();
      } else if (rid == 1) {
         dequeued = blockedState1.Dequeue();
      } else if (rid == 2) {
        dequeued = blockedState2.Dequeue();
      }
      //Makes sure we're not dequeing a pid that doesn't exist
      if (dequeued != 0) {
        readyState.Enqueue(dequeued, processes[dequeued].priority);
      }
    } else if (chr == 'Q') {
      totTime++;
      processes[runningProcess].quantum -= 1;
      processes[runningProcess].run_time -= 1;
      processes[runningProcess].cpu_time++;
      //Checks to see if a process finishes, if so update new running one
      if (processes[runningProcess].run_time < 1) {
        turnaround += totTime - processes[runningProcess].start_time;
        finishedProc++;
        runningProcess = readyState.Dequeue();
      }
      //Need to update quantum and priority if quantum dips runs out
      else if (processes[runningProcess].quantum < 1) {
         //Fixes the PID 9 problem where process goes beyond 3
         if (processes[runningProcess].priority < 3) {
          processes[runningProcess].priority += 1;
         }
         processes[runningProcess].quantum = pow(2, processes[runningProcess].priority);
         readyState.Enqueue(runningProcess, processes[runningProcess].priority);
         runningProcess = readyState.Dequeue();
      }
    } else if (chr == 'C') {
      char cmd;
      int num;
      read(mcpipe2[0], (int *)&num, sizeof(num));
      read(mcpipe2[0], (char *)&cmd, sizeof(cmd));
      totTime++;
      processes[runningProcess].quantum -= 1;
      processes[runningProcess].run_time -= 1;
      processes[runningProcess].cpu_time++;
      if (cmd == 'A') {
        processes[runningProcess].value += num;
      } else if (cmd == 'S') {
        processes[runningProcess].value -= num;
      } else if (cmd == 'M') {
        processes[runningProcess].value *= num;
      } else if (cmd == 'D') {
        processes[runningProcess].value /= num;
      }
      //Checks to see if a process completes
      if (processes[runningProcess].run_time < 1) {
        turnaround += totTime - processes[runningProcess].start_time;
        finishedProc++;
        runningProcess = readyState.Dequeue();
      //Same quantum logic as above.
      } else if (processes[runningProcess].quantum < 1) {
        processes[runningProcess].priority += 1;
        processes[runningProcess].quantum = pow(2, processes[runningProcess].priority);
        readyState.Enqueue(runningProcess, processes[runningProcess].priority);
        runningProcess = readyState.Dequeue();
      }
    } else if (chr == 'P') {
      //New fork that spawns the reporter
      int c1 = fork();
      if (c1 == 0) {
        reporter();
        exit(-1);
      } else if (c1 < 0) {
        cout << "Forked Failed!" << endl;
      }
    }
    //If running process hasn't been set, set it
    if (runningProcess == -1) {
      runningProcess = readyState.Dequeue();
    }
    //read a character from the pipe
    read(mcpipe2[0], &chr, sizeof(char));
  }
  double avg = 0.0;
  //Need to typecast so we get a reasonable output
  avg = double(turnaround) / double(finishedProc);
  cout << "The average Turnaround time: " << avg << endl;
  //close down the pipe
  close(mcpipe2[0]);
  close(mcpipe2[1]);
  //return with a 2, so the parent receive the status message of 2
  // note the number can not greater then 255.
  return 2;
}
void reporter() {
  cout << "*****************************************************" << endl;
  cout << "The current system state is as follows:" << endl;
  cout << "*****************************************************" << endl;
  cout << " " << endl;
  cout << "CURRENT TIME: " << totTime << endl;
  cout << " " << endl;
  cout << "RUNNING PROCESS: " << endl;
  cout << "PID  Priority Value  Start Time  Total CPU time" << endl;
  cout << setw(2)  << processes[runningProcess].pid << "  "
       << setw(4)  << processes[runningProcess].priority << "  "
       << setw(7)  << processes[runningProcess].value << "  "
       << setw(11) << processes[runningProcess].start_time << "  "
       << setw(8)  << processes[runningProcess].cpu_time << endl;
  cout << "BLOCKED PROCESSES: " << endl;
  int* q0;
  int* q1;
  int* q2;
  if (blockedState.Qsize(0) > 0 || blockedState.Qsize(1) > 0 || blockedState.Qsize(2) > 0) {
    cout << "Queue of processes blocked for resource 0: " << endl;
    cout << "PID  Priority Value  Start Time  Total CPU time" << endl;
    q0 = blockedState.Qstate(0);
    q1 = blockedState.Qstate(1);
    q2 = blockedState.Qstate(2);
    for (int i = 0; i < blockedState.Qsize(0) && blockedState.Qsize(0) > 0; i++) {
      cout << setw(2)  << processes[q0[i]].pid << "  "
           << setw(4)  << processes[q0[i]].priority << "  "
           << setw(7)  << processes[q0[i]].value << "  "
           << setw(11) << processes[q0[i]].start_time << "  "
           << setw(8)  << processes[q0[i]].cpu_time << endl;
    }
    for (int i = 0; i < blockedState.Qsize(1) && blockedState.Qsize(1) > 0; i++) {
      cout << setw(2)  << processes[q1[i]].pid << "  "
           << setw(4)  << processes[q1[i]].priority << "  "
           << setw(7)  << processes[q1[i]].value << "  "
           << setw(11) << processes[q1[i]].start_time << "  "
           << setw(8)  << processes[q1[i]].cpu_time << endl;
    }
    for (int i = 0; i < blockedState.Qsize(2) && blockedState.Qsize(2) > 0; i++) {
      cout << setw(2)  << processes[q2[i]].pid << "  "
           << setw(4)  << processes[q2[i]].priority << "  "
           << setw(7)  << processes[q2[i]].value << "  "
           << setw(11) << processes[q2[i]].start_time << "  "
           << setw(8)  << processes[q2[i]].cpu_time << endl;
    }
  } else {
    cout << "Queue of processes blocked for resource 0 is empty" << endl;
  }
  if (blockedState1.Qsize(0) > 0 || blockedState1.Qsize(1) > 0 || blockedState1.Qsize(2) > 0) {
    cout << "Queue of processes blocked for resource 1: " << endl;
    cout << "PID  Priority Value  Start Time  Total CPU time" << endl;
    q0 = blockedState1.Qstate(0);
    q1 = blockedState1.Qstate(1);
    q2 = blockedState1.Qstate(2);
    for (int i = 0; i < blockedState1.Qsize(0) && blockedState1.Qsize(0) > 0; i++) {
      cout << setw(2)  << processes[q0[i]].pid << "  "
           << setw(4)  << processes[q0[i]].priority << "  "
           << setw(7)  << processes[q0[i]].value << "  "
           << setw(11) << processes[q0[i]].start_time << "  "
           << setw(8)  << processes[q0[i]].cpu_time << endl;
    }
    for (int i = 0; i < blockedState1.Qsize(1) && blockedState1.Qsize(1) > 0; i++) {
      cout << setw(2)  << processes[q1[i]].pid << "  "
           << setw(4)  << processes[q1[i]].priority << "  "
           << setw(7)  << processes[q1[i]].value << "  "
           << setw(11) << processes[q1[i]].start_time << "  "
           << setw(8)  << processes[q1[i]].cpu_time << endl;
    }
    for (int i = 0; i < blockedState1.Qsize(2) && blockedState1.Qsize(2) > 0; i++) {
      cout << setw(2)  << processes[q2[i]].pid << "  "
           << setw(4)  << processes[q2[i]].priority << "  "
           << setw(7)  << processes[q2[i]].value << "  "
           << setw(11) << processes[q2[i]].start_time << "  "
           << setw(8)  << processes[q2[i]].cpu_time << endl;
    }
  } else {
    cout << "Queue of processes blocked for resource 1 is empty" << endl;
  }
  if (blockedState2.Qsize(0) > 0 || blockedState2.Qsize(1) > 0 || blockedState2.Qsize(2) > 0) {
    cout << "Queue of processes blocked for resource 2: " << endl;
    cout << "PID  Priority Value  Start Time  Total CPU time" << endl;
    q0 = blockedState2.Qstate(0);
    q1 = blockedState2.Qstate(1);
    q2 = blockedState2.Qstate(2);
    for (int i = 0; i < blockedState2.Qsize(0) && blockedState2.Qsize(0) > 0; i++) {
      cout << setw(2)  << processes[q0[i]].pid << "  "
           << setw(4)  << processes[q0[i]].priority << "  "
           << setw(7)  << processes[q0[i]].value << "  "
           << setw(11) << processes[q0[i]].start_time << "  "
           << setw(8)  << processes[q0[i]].cpu_time << endl;
    }
    for (int i = 0; i < blockedState2.Qsize(1) && blockedState2.Qsize(1) > 0; i++) {
      cout << setw(2)  << processes[q1[i]].pid << "  "
           << setw(4)  << processes[q1[i]].priority << "  "
           << setw(7)  << processes[q1[i]].value << "  "
           << setw(11) << processes[q1[i]].start_time << "  "
           << setw(8)  << processes[q1[i]].cpu_time << endl;
    }
    for (int i = 0; i < blockedState2.Qsize(2) && blockedState2.Qsize(2) > 0; i++) {
      cout << setw(2)  << processes[q2[i]].pid << "  "
           << setw(4)  << processes[q2[i]].priority << "  "
           << setw(7)  << processes[q2[i]].value << "  "
           << setw(11) << processes[q2[i]].start_time << "  "
           << setw(8)  << processes[q2[i]].cpu_time << endl;
    }
  } else {
    cout << "Queue of processes blocked for resource 2 is empty" << endl;
  }
  cout << "PROCESSES READY TO EXECUTE:" << endl;
  if (readyState.Qsize(0) > 0) {
    q0 = readyState.Qstate(0);
    cout << "Queue of processes with priority 0: " << endl;
    cout << "PID  Priority Value  Start Time  Total CPU time" << endl;
    for (int i = 0; i < readyState.Qsize(0); i++) {
      cout << setw(2)  << processes[q0[i]].pid << "  "
           << setw(4)  << processes[q0[i]].priority << "  "
           << setw(7)  << processes[q0[i]].value << "  "
           << setw(11) << processes[q0[i]].start_time << "  "
           << setw(8)  << processes[q0[i]].cpu_time << endl;
    }
  } else {
    cout << "Queue of processes with priority 0 is empty" << endl;
  }
  if (readyState.Qsize(1) > 0) {
    q0 = readyState.Qstate(1);
    cout << "Queue of processes with priority 1: " << endl;
    cout << "PID  Priority Value  Start Time  Total CPU time" << endl;
    for (int i = 0; i < readyState.Qsize(1); i++) {
      cout << setw(2)  << processes[q0[i]].pid << "  "
           << setw(4)  << processes[q0[i]].priority << "  "
           << setw(7)  << processes[q0[i]].value << "  "
           << setw(11) << processes[q0[i]].start_time << "  "
           << setw(8)  << processes[q0[i]].cpu_time << endl;
    }
  } else {
    cout << "Queue of processes with priority 1 is empty" << endl;
  }
  if (readyState.Qsize(2) > 0) {
    q0 = readyState.Qstate(2);
    cout << "Queue of processes with priority 2: " << endl;
    cout << "PID  Priority Value  Start Time  Total CPU time" << endl;
    for (int i = 0; i < readyState.Qsize(2); i++) {
      cout << setw(2)  << processes[q0[i]].pid << "  "
           << setw(4)  << processes[q0[i]].priority << "  "
           << setw(7)  << processes[q0[i]].value << "  "
           << setw(11) << processes[q0[i]].start_time << "  "
           << setw(8)  << processes[q0[i]].cpu_time << endl;
    }
  } else {
    cout << "Queue of processes with priority 2 is empty" << endl;
  }
  if (readyState.Qsize(3) > 0) {
    q0 = readyState.Qstate(3);
    cout << "Queue of processes with priority 3: " << endl;
    cout << "PID  Priority Value  Start Time  Total CPU time" << endl;
    for (int i = 0; i < readyState.Qsize(3); i++) {
      cout << setw(2)  << processes[q0[i]].pid << "  "
           << setw(4)  << processes[q0[i]].priority << "  "
           << setw(7)  << processes[q0[i]].value << "  "
           << setw(11) << processes[q0[i]].start_time << "  "
           << setw(8)  << processes[q0[i]].cpu_time << endl;
    }
  } else {
    cout << "Queue of processes with priority 3 is empty" << endl;
  }
}
