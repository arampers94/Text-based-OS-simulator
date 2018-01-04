/**
 * Author     : Amar Rampersaud
 * Created    : November 27, 2017
 * Title      : CSCI 340 Home Project
 * Description: Simulates some functions of an OS
 * Build      : g++ -o main main.cpp
 */

#include <iostream>
#include <string>
#include <sstream>
#include <list>
#include <vector>

using namespace std;

// used in struct functions and main
string cpu_state[2] = {"idle", "busy"};
string process_state[2] = {"running", "ready"};

struct Frame{
  int lru_bit, process_num, frame_num, page_num;

  Frame() : lru_bit(-1), process_num(-1), frame_num(-1), page_num(-1) {}

  Frame operator =(Frame &rhs){
    lru_bit = rhs.lru_bit;
    process_num = rhs.process_num;
    page_num = rhs.page_num;

    return *this;
  }
};

// Process pid begins at 1. Each new process pid is incremented by 1.
// Processes have 2 states - running and ready
struct PCB{
  int pid, priority, page_num;
  string state, requested_file;

  PCB() : pid(-1), priority(-1), page_num(-1), state("null"), requested_file("null") {}

  void clear(){
    pid = -1;
    priority = -1;
    state = "";
  }

};

struct RAM{
  unsigned int size;
  int max_frames;
  vector<Frame> frame_table;

  RAM() : size(0), max_frames(0) {}

  // Replace the least recently used entry in the frame table
  void ReplaceLRU (Frame &new_frame){
    int current = new_frame.lru_bit;
    int lru_index, temp;
    for (int i = 0; i < frame_table.size(); i++){
      temp = frame_table[i].lru_bit;
      if (temp < current){
        current = temp;
        lru_index = i;
      }
    }

    frame_table[lru_index] = new_frame;
    cout << "Frame " << lru_index << " replaced with new frame in memory.\n";
  }

  void ShowMemory (){
    for (int i = 0; i < 20; i++){
      cout << "-";
    }
    cout << endl;
    cout << "Current state of memory: \n";

    for (int i = 0; i < frame_table.size(); i++){
      cout << "Frame " << i << " is occupied by process " << frame_table[i].process_num << endl;
      cout << "Page stored: " << frame_table[i].page_num  << endl;
      cout << "Timestamp  : " << frame_table[i].lru_bit   << endl;
      cout << "----------\n";
    }

    for (int i = 0; i < 20; i++){
      cout << "-";
    }
    cout << endl;
  }

  bool IsFull (){
    return frame_table.size() == max_frames;
  }

  int FramesInMem(){
    return frame_table.size();
  }

};  // end RAM

// CPU has 2 states - idle and busy
// idle: no processes currently using CPU
// busy: a process is currently using CPU
struct CPU{
  PCB active_process;
  string state;
  vector<PCB> ready_queue;

  CPU() : state(cpu_state[0]), active_process(), ready_queue() {}

  void Print(){
    if (active_process.pid == -1){
      cout << "No processes currently using CPU.\n";
    } else{
      cout << "CPU-----------------\n";
      cout << "Process pid   : " << active_process.pid << endl;
      cout << "Priority level: " << active_process.priority << endl;
      cout << "Number of processes in the ready queue: " << ready_queue.size() << endl;
      cout << "Ready queue---------\n";
      for (int i = 0; i < ready_queue.size(); i++){
        cout << "Process pid: " << ready_queue[i].pid      << endl;
        cout << "Priority   : " << ready_queue[i].priority << endl;
        cout << "State      : " << ready_queue[i].state    << endl;
        cout << "--------------------\n";
      } // end for
    } // end if
  } // end print

  // Scans the CPU ready queue to allow the process with the next highest priority
  // to use the CPU. If ready queue is empty, message is printed to the screen.
  PCB NextReady(){
    PCB next;
    if (ready_queue.empty()){
      cout << "Ready queue is empty.\n";
      state = cpu_state[0]; // CPU becomes idle
      return next;
    }

    int highest = ready_queue[0].priority;
    // Determine what the highest priority is
    for (int i = 0; i < ready_queue.size(); i++){
      if (ready_queue[i].priority > highest){
        highest = ready_queue[i].priority;
      }
    }

    // Find the first process with the highest priority
    for (int i = 0; i < ready_queue.size(); i++){
      if (ready_queue[i].priority == highest){
        next = ready_queue[i];
        next.state = process_state[0];
        ready_queue.erase(ready_queue.begin() + i);
        break;
      }
    }

    if (next.pid == -1){    // if ready queue is was empty
      state = cpu_state[0]; // CPU becomes idle
    } else {
      state = cpu_state[1]; // otherwise CPU is busy
    }

    return next;
  } // end NextReady

  // Process currently using the CPU is terminated and removed from the system.
  void Terminate(RAM &ram){
    // Remove process from memory
    for (int i = 0; i < ram.frame_table.size(); i++){
      if (active_process.pid == ram.frame_table[i].process_num){
        ram.frame_table.erase(ram.frame_table.begin() + i);
        i--;
      }
    }

    // Allow the process with the next highest priority to use the CPU. Print a message
    // if no processes are available.
    if (ready_queue.empty() && active_process.pid != -1){
      cout << "Process " << active_process.pid << " terminated.\n";
      active_process.clear();
      state = cpu_state[0];
      cout << "CPU is now idle.\n";
    } else if (ready_queue.empty() && active_process.pid == -1){
      cout << "There are no active processes in the CPU.\n";
    } else{
      cout << "Process " << active_process.pid << " terminated.\n";
      active_process.clear();
      active_process = NextReady();
      cout << "Process " << active_process.pid << " is now using CPU.\n";
      state = cpu_state[1];
      vector<PCB>::iterator it;
      for (int i = 0; i < ready_queue.size(); i++){
        if (active_process.pid == ready_queue[i].pid){
          it = ready_queue.begin() + i;
          ready_queue.erase(it);
        }
      } //end for
    }
  }

  CPU operator =(PCB &process){
    active_process.pid = process.pid;
    active_process.priority = process.priority;
    active_process.state = process.state;

    return *this;
  }
};

struct DISK{
  PCB current_process;
  vector<PCB> disk_queue;
  string file_name;
  int disk_num;

  DISK() : file_name("") {}

  DISK operator =(DISK &rhs){
    current_process = rhs.current_process;
    disk_queue = rhs.disk_queue;
    file_name  = rhs.file_name;
    disk_num   = rhs.disk_num;

    return *this;
  }
};

// Prints information about process using disks and the I/O queue of each disk.
void PrintDiskInfo(vector<DISK> disks);


int main(){

  unsigned int RAM_size, total_hard_disks;
  int frame_size, total_frames;
  int timestamp = 1;
  int process_id = 1;
  vector<DISK> disks;
  CPU cpu;
  RAM ram;

  cout << "Enter the amount of RAM in this simulation: ";
  cin >> RAM_size;

  cout << "Enter the size of frames in this simulation: ";
  cin >> frame_size;

  cout << "Enter the number of hard disks in this simulation: ";
  cin >> total_hard_disks;

  // Adding hard disks to the system
  for (int i = 0; i < total_hard_disks; i++){
    DISK new_disk;
    new_disk.disk_num = i;
    disks.push_back(new_disk);
  }

  total_frames = RAM_size / frame_size;

  ram.size = RAM_size;
  ram.max_frames = total_frames;

  // List of commands
  for (int i = 0; i < 20; i++){
    cout << '-';
  }
  cout << "\nBeginning simulation\n\n";
  cout << "----------List of commands----------\n";
  cout << "A <priority>         : Creates new process and assigns it priority level <priority>.\n";
  cout << "t                    : Terminates process currently using CPU.\n";
  cout << "D <number> <filename>: Process currently using CPU requests hard disk number";
  cout << " <number> to read/write <filename>.\n";
  cout << "D <number>           : Hard disk <number> has finished work for one process.\n";
  cout << "m <address>          : Process currently using CPU requests memory operation for address <address>.\n";
  cout << "S r                  : Shows processes currently using CPU and processes waiting in ready queue.\n";
  cout << "S i                  : Shows processes currently using hard disks and processes waiting to use them.\n";
  cout << "S m                  : Shows state of memory.\n";
  cout << "------------------------------------\n";

  cin.ignore();   // ignore trailing newline character so getline can read user input
  string response = "initial";
  cout << "Enter a command: ";
  getline (cin, response);

  // Begin queries
  while (response != ""){
    stringstream ss (response);
    char first_char;
    char second_char;
    ss >> first_char;

    if (first_char == 't'){   // terminates process currently using CPU
      cpu.Terminate(ram);
    }

    else if (first_char == 'S'){  // show process using CPU and its ready queue
        ss >> second_char;
        if (second_char == 'r'){        // show processes currently running and waiting in ready queue
          cpu.Print();
        } else if (second_char == 'i'){ // show processes using disks and I/O queue for each
          PrintDiskInfo(disks);
        } else if (second_char == 'm'){ // show the state of memory
          if (ram.frame_table.empty()){
            cout << "There is nothing in memory.\n";
          } else{
            ram.ShowMemory();
          }
       }
    }

    else if (first_char == 'm'){   // m <address>
      int address;
      ss >> address;                    // virtual address
      int page = address / frame_size;  // physical page
      bool found = false;

      if (cpu.active_process.pid == -1){
        cout << "No processes currently using CPU.\n";
      } else{
        // Find the currently running process in memory and change its page num to page
        for (int i = 0; i < ram.frame_table.size(); i++){
          if (cpu.active_process.pid == ram.frame_table[i].process_num && cpu.active_process.page_num == page){
            found = true;
            ram.frame_table[i].lru_bit = timestamp;
            timestamp++;
            break;
          }
        }

        Frame new_frame;
        new_frame.process_num = cpu.active_process.pid;
        new_frame.page_num    = page;
        new_frame.frame_num   = ram.FramesInMem() + 1;

        if (found == false && !ram.IsFull()){
          new_frame.lru_bit = timestamp;
          timestamp++;
          ram.frame_table.push_back(new_frame);
        } else if (found == false && ram.IsFull()){
          new_frame.lru_bit = timestamp;
          timestamp++;
          ram.ReplaceLRU(new_frame);
        }
      }
    }

    else if (first_char == 'A'){   // a <priority>
        int level;
        PCB process;
        Frame new_frame;
        ss >> level;

        process.pid = process_id;
        process.page_num = 0;
        new_frame.page_num = 0;
        new_frame.process_num = process_id;
        process_id++;
        process.priority = level;
        new_frame.lru_bit = timestamp;
        timestamp++;

        if (ram.IsFull()){
          ram.ReplaceLRU (new_frame);
        } else{
          ram.frame_table.push_back(new_frame);
        }

        if (cpu.ready_queue.empty() && cpu.state == cpu_state[0]){
          process.state = process_state[0];
          cpu.active_process = process;
          cpu.state = cpu_state[1];
          cout << "Process " << process.pid << " is now using the CPU.\n";
        } else if (cpu.state == cpu_state[1]){
            if (cpu.active_process.priority < level){
              cpu.active_process.state = process_state[1];
              cpu.ready_queue.push_back(cpu.active_process);
              cout << "Process " << cpu.active_process.pid << " sent to ready queue.\n";
              process.state = process_state[0];
              cpu.active_process = process;
              cout << "Process " << cpu.active_process.pid << " is now using the CPU.\n";
            } else {
              process.state = process_state[1];
              cpu.ready_queue.push_back(process);
              cout << "Process added to CPU ready queue.\n";
            }
        }
    }

    else if (first_char == 'D'){  // D <number>
        int disk_num;
        ss >> disk_num;
        DISK curr_disk = disks[disk_num];
        int pid = curr_disk.current_process.pid;

          if (disk_num > disks.size() - 1){
            if (disks.size() == 1){
              cout << "There is only disk 0.\n";
            } else {
              cout << "There is no disk " << disk_num << ". ";
              cout << "Choose a disk from 0 to " << disks.size() - 1 << ".\n";
            }
          } else {
            if (pid == -1){
              cout << "There are no processes using disk " << disk_num << endl;
            } else {
              curr_disk.current_process.state = "ready";
              cout << "Process " << pid << " is finished using disk " << disk_num << ".\n";

              if (cpu.active_process.pid == -1){   //CPU is idle; let process use it
                curr_disk.current_process.state = process_state[0];
                cpu.active_process = curr_disk.current_process;
                cout << "It is now using the CPU.\n";
              } else if (cpu.active_process.priority < curr_disk.current_process.priority){ // Disk process priority is higher
                curr_disk.current_process.state = process_state[0];
                cpu.active_process.state = process_state[1];
                cpu.ready_queue.push_back(cpu.active_process);
                cpu.active_process = curr_disk.current_process;
                cout << "It is now using the CPU.\n";
              } else{  // CPU process priority is higher
                curr_disk.current_process.state = process_state[1];
                cpu.ready_queue.push_back(curr_disk.current_process);   // send process back to CPU ready queue
                cout << "It has been sent to the CPU ready queue.\n";
              }
              if (!curr_disk.disk_queue.empty()){   // allow next process in I/O queue to use disk
                curr_disk.current_process = curr_disk.disk_queue[0];    // fetch next process from I/O queue
                curr_disk.file_name = curr_disk.disk_queue[0].requested_file;
                curr_disk.disk_queue.erase(curr_disk.disk_queue.begin());   // FCFS - remove first process in queue
              } else{ // I/O queue is empty; disk becomes idle
                cout << "I/O queue empty. Disk " << disk_num << " is now idle.\n";
                curr_disk.current_process.clear();
                curr_disk.file_name = "";
              }
            }
          }
        }

        else if (first_char == 'd'){ // d <number> <filename>
          int disk_num;
          string filename;
          ss >> disk_num >> filename;
          DISK curr_disk = disks[disk_num];
          int pid = curr_disk.current_process.pid;

          if (cpu.active_process.pid == -1 && cpu.ready_queue.empty()){
            cout << "CPU ready queue is empty.\n";
          } else {
            if (disk_num > disks.size() - 1){
              if (disks.size() == 1){
                cout << "There is only disk 0.\n";
              } else {
                cout << "There is no disk " << disk_num << ". ";
                cout << "Choose a disk from 0 to " << disks.size() - 1 << ".\n";
              }
            } else if (curr_disk.current_process.pid != -1){ // disk is being used by another process
              cout << "That disk is currently busy. Adding process to disk queue.\n";
              cpu.active_process.requested_file = filename;
              curr_disk.disk_queue.push_back(cpu.active_process);
              if (!cpu.ready_queue.empty()){   // next highest priority process gets to use CPU
                cpu.active_process = cpu.NextReady();
                cout << "Process " << cpu.active_process.pid << " is now using CPU.\n";
              } else{   // CPU ready queue is empty
                cpu.active_process.clear();
                cpu.state = cpu_state[0];
                cout << "No processes in CPU ready queue. CPU is now idle." << endl;
              }
            } else{   // disk is not being used; process gets access to disk
                if (cpu.active_process.pid != -1){
                  curr_disk.current_process = cpu.active_process;
                  cpu.active_process.requested_file = filename;
                  curr_disk.file_name = cpu.active_process.requested_file;
                  cout << "Process " << curr_disk.current_process.pid << " is now using disk " << disk_num << ".\n";
                  cpu.active_process = cpu.NextReady();
                } else{
                  cout << "No processes available to use disk.\n";
                }

                if (cpu.active_process.pid != -1){
                  cout << "Process " << cpu.active_process.pid << " is now using CPU.\n";
                }
                if (cpu.state == cpu_state[0]){
              cout << "CPU is now idle.\n";
              }
            }
          }
          disks[disk_num] = curr_disk;
        }  // end d <number> <filename>

    else{
      cout << "Command not recognized. Please choose from the list of commands above." << endl;
    }

    cout << "Enter another command (or press enter to end simulation): ";
    getline (cin, response);
  } // end while

  for (int i = 0; i < 20; i++){
    cout << '-';
  }
  cout << "\nEnding simulation\n";

return 0;
}

void PrintDiskInfo(vector<DISK> disks){
  for (int i = 0; i < disks.size(); i++){
    if (disks[i].current_process.pid != -1){
      cout << "--------------------\n";
      cout << "Disk " << i << ": \n";
      cout << "Pid      : " << disks[i].current_process.pid << endl;
      cout << "File name: " << disks[i].file_name << endl;
      cout << "--------------------\n";
      cout << "--------------------\n";
      cout << "I/O queue:" << endl;
    } else {
      cout << "No processes are currently using disk " << i << endl;
    }
    for (int j = 0; j < disks[i].disk_queue.size(); j++){
      if (!disks[i].disk_queue.empty()){
        cout << "Process pid   : " << disks[i].disk_queue[j].pid << endl;
        cout << "Requested file: " << disks[i].disk_queue[j].requested_file << endl;
        cout << "--------------------\n";
      } else {
        cout << "I/O queue for disk " << i << " is empty.\n";
        break;
      }
    } // end inner loop
  } // end outer loop
} // end PrintDiskInfo
