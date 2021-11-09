
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstring>
#include <stdlib.h>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <stack>
#include <deque>
#include <algorithm>
#include <getopt.h>
#include<climits>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <list>
using namespace std;
int verbosemode=0;
int current_time=0;
int scale=0; 
int ofs=0;
int quantum= 10000;
int maxPrio = 4;
ifstream rand_file;
vector<int> randVals;
vector <int> random_nums;
int tot_random_nums =0;
double process_io_counter =0;
double cpu_burst_all_process = 0;
double io_burst_all_process = 0;
double new_io_burst = 0;

//Process * current_running_process=NULL;
enum State {CREATED, READY, RUNNING, BLOCKED, DONE};
enum TransitionState {TRANS_TO_READY, TRANS_TO_RUN, TRANS_TO_BLOCK, TRANS_TO_DONE, TRANS_TO_PREEMPT};

class Process
{
    public:
        int arrTime, tot_cpuTime, cpu_burst, io_burst;
        int cb,ib;
        int pID;
        int staticPrio=-99;
        int dynamicPrio;
        int remainingCpuTime;
        int finishTime=-1;
        int total_time_spent_in_io = 0;
        int remaining_cpu_burst = 0;
        bool dp_enabled = false;
        int proc_timestamp=0;

        State current_state;

        void printprocc()
        {
            cout<<"PID->"<<pID<<" "
                <<"AT->"<<arrTime<<" "
                <<"TC->"<<tot_cpuTime<<" "
                <<"CB->"<<cpu_burst<<" "
                <<"IO->"<<io_burst<<endl;



        }

};
Process* ptr;
Process* CURR_RUN_PRO = nullptr;
Process* current_running_process = NULL;
vector <Process*> processes;
vector <Process*> :: const_iterator processIt;

int myrandom(int burst) { 
    //cout<<"inside my random"<<endl;
  if(ofs == tot_random_nums){
    ofs = 0;
  }
  return 1 + (random_nums[ofs++] % burst); 
}

class Event
{
    public:
        int eventId;
        int eventTimestamp;
        Process* event_proc;
        TransitionState trans_state;
        State eventState;
        Event(int timestamp, Process *proc, TransitionState tran)
        {
           
            eventTimestamp = timestamp;
            event_proc = proc;
            trans_state = tran;
        }
       Process* get_next_event_process()
       {
          return event_proc;
       }

};
Event* evt;
deque<Event*> eventQ;
class DES
{
    public:
     
        Event* get_event() 
        {
            
            if(eventQ.empty())
            {
                return nullptr;
            }
            Event* nwE = eventQ.front();
            eventQ.pop_front();
            return nwE;
        }
        int get_next_event_ts(Process *pro){
      int i;
      int ts ;
      deque<Event*>::iterator it = eventQ.begin();
      for(i=0; i < eventQ.size(); i++){
        if(eventQ.at(i)->event_proc->pID == pro->pID){
          ts = eventQ.at(i)->eventTimestamp;
        }
        ++it;
      }
      return ts;
    }
        void add_event(Event* event)
        {
        //    int i;
        //   deque<Event*>::iterator it = eventQ.begin();
        //   for(i=0; i < eventQ.size(); i++){
        //     // if(eventQ.at(i).timestamp > e.timestamp){
        //     //   break;
        //     // }
        //     if(eventQ.at(i)->eventTimestamp > event->eventTimestamp){
        //       break;
        //     }
        //     ++it;
        //   }
      
	      // eventQ.insert(it,event);

          deque <Event*> :: iterator eventIterator;
          for(eventIterator=eventQ.begin();eventIterator!=eventQ.end();eventIterator++)
          {
            if(event->eventTimestamp < (*eventIterator)->eventTimestamp)
            {
              break;
            }
          }
          eventQ.insert(eventIterator,event);
            // deque <Event*> :: iterator eventIt;
            // //eventQ.push_back(event);
            // //int qsize = eventQ.size();
            // for (eventIt=eventQ.begin();eventIt!=eventQ.end();eventIt++)
            // {
            //     if(event->eventTimestamp<(*eventIt)->eventTimestamp)
            //     {
                     
            //         break;
            //     }
            // }
            // eventQ.insert(eventIt,event);
        }
        // void remove_event()
        // {
        //     eventQ.pop_front();
        // }

        void remove_event(Process *pro) {
      int i;
      deque<Event*>::iterator it = eventQ.begin();
      for(i=0; i < eventQ.size(); i++){
        if(eventQ.at(i)->event_proc->pID == pro->pID){
          eventQ.erase(it);
        }
        ++it;
      }
      
    }
        void print_all_event(){
          //cout<<"EventQ is : \n";
          for (auto it = eventQ.begin(); it != eventQ.end(); ++it)
          {
              Event* e = *it;
             // cout<<"process :"<<e->event_proc->pID<<endl;
              //cout<<"event timestamp:"<<e->eventTimestamp<<endl;
              
              
          }
       // deque<Event>::iterator it1;
        // for(int i=0; i < eventQ.size(); i++){
        //   cout<<"Timestamp: "<<eventQ.at(i)->eventTimestamp<<" Process: "<<eventQ.at(i)->event_proc->pID<<endl;
        // }
        //cout<<endl;
        }
    
        
    

};
//DES mydes;
//DES* des_level;





class Scheduler
{
public:
    //int quantum = 10000; // set a large defualt quantum
    deque<Process *> runQ;
	virtual void add_process(Process* p){};
	virtual Process* get_next_process(){return nullptr;};
  virtual void print_runq(){};
  virtual void change_dyn_prio(Process *){};
virtual void test_preempt(Process*, int)=0;
virtual void check_preemption(Process *, Process *, int ){};
};

Scheduler* sched;
class FCFS: public Scheduler{
    //deque<Process*> runQ;
    public:
        void add_process(Process *proc) {
        runQ.push_back(proc);
    }
    Process* get_next_process() {
       if(runQ.empty()){
        return NULL;
      }
        Process * temp = runQ.front();
        runQ.pop_front();
        return temp;
    }
    void change_dyn_prio(Process *p){

    }
   void print_runq(){
    // cout<<"size : "<<runQ.size()<<endl;
   }

  void test_preempt(Process*, int)
  {

  }
  void check_preemption(Process *, Process *, int )
  {

  }
};
class LCFS: public Scheduler{
   // deque<Process*> runQ;
    public:
        void add_process(Process *proc) {
        runQ.push_back(proc);
    }
    Process* get_next_process() {
       if(runQ.empty()){
        return NULL;
      }
        Process * temp = runQ.back();
        runQ.pop_back();
        return temp;
    }
    void change_dyn_prio(Process *p){

    }
    void print_runq(){
     //cout<<"size : "<<runQ.size()<<endl;
   }
     void test_preempt(Process*, int)
  {

  }
  void check_preemption(Process *, Process *, int )
  {
    
  }
};

class SRTF: public Scheduler{
    
    public:
        void add_process(Process *proc) {
           runQ.push_back(proc);
        //runQ.push_back(proc);
    //     int i =0;
		//   while (i < runQ.size() && (proc->tot_cpuTime - proc->remainingCpuTime) >= (runQ[i]->tot_cpuTime - runQ[i]->remainingCpuTime)) 
    //       { 
		// 	    i++;
		//       }
		// runQ.insert(runQ.begin()+i,proc);
    }

     Process* get_next_process() {
      //   if(runQ.empty()){
      //   return NULL;
      // }
      //   Process * temp = runQ.front();
      //   runQ.pop_front();
      //   return temp;

      if(runQ.empty()){
        return NULL;
      }
      
      int min_remaining_time = INT_MAX;
      deque<Process*>::iterator it = runQ.begin();
      deque<Process*>::iterator min_it;
      Process *proc;

      for(int i=0; i < runQ.size(); i++){// use auto here 
        if(runQ.at(i)->remainingCpuTime < min_remaining_time){
          min_remaining_time = runQ.at(i)->remainingCpuTime;
          min_it = it;
          proc = runQ.at(i);
        }
        
        ++it;
      }

      runQ.erase(min_it);
      return proc;
    }
    void print_runq(){
     //cout<<"size : "<<runQ.size()<<endl;
   }
   void change_dyn_prio(Process *p){

    }
      void test_preempt(Process*, int)
  {

  }
  void check_preemption(Process *, Process *, int )
  {
    
  }

};
class RR : public Scheduler {
  public:
    void add_process(Process *p) {
		  runQ.push_back(p);
	  }
    Process * get_next_process(){
      if(runQ.empty()){
        return NULL;
      }
      Process * proc = runQ.front();
      runQ.pop_front();
      return proc;

    }
    void change_dyn_prio(Process *p){

    }
      void test_preempt(Process*, int)
  {

  }
  void check_preemption(Process *, Process *, int )
  {
    
  }
};


class PRIO : public Scheduler {
 public:
    // vector<deque<Process *>> activeQ;
    // vector<deque<Process *>> expiredQ;
    
    list<list<Process*>> AcQ;
    list<list<Process*>> ExQ;
    list<Process*>* acq;
    list<Process*>* exq;

    PRIO() {
      // activeQ = vector<deque<Process *>>(maxPrio);
      // expiredQ =  vector<deque<Process *>>(maxPrio);
       AcQ = list<list<Process *>>(maxPrio);
      ExQ =  list<list<Process *>>(maxPrio);
      acq = new list<Process*>[maxPrio];
      exq = new list<Process*>[maxPrio];
    }

    void add_process(Process *p) {
      
      int dyn_prio_index = p->dynamicPrio;
      list<Process*> pi;
      if (!p->dp_enabled){
     //   AcQ[dyn_prio_index].push_back(p);
     acq[dyn_prio_index].push_back(p);
     //AcQ.push_back(pi);
      } else {
        //ExQ[dyn_prio_index].push_back(p);
        exq[dyn_prio_index].push_back(p);
      }
      p->dp_enabled = false;
		  
	  }

    void change_dyn_prio(Process *p) {
      p->dynamicPrio --;
      if (p->dynamicPrio == -1) {
        p->dynamicPrio = p->staticPrio - 1;
        p->dp_enabled = true;
      } else {
        p->dp_enabled = false;
      }
    }

    void swapQ(){
    
      list<Process*>* temp;
      temp = acq;
      acq = exq;
      exq = temp;
    }

    bool isActiveQEmpty(){
      for(int i = 0 ; i < maxPrio ; i++){
        if(!acq[i].empty()){
          return false;
        }
      }
      return true;
    }
    bool isExpiredQEmpty(){
      for(int i = 0 ; i < maxPrio ; i++){
        if(!exq[i].empty()){
          return false;
        }
      }
      return true;
    }
    Process * get_next_process(){
      if(isActiveQEmpty() && isExpiredQEmpty()){
        return NULL;
      }
      if(isActiveQEmpty()){
        swapQ();
      }
    
      Process * proc;

      for(int i= maxPrio-1; i >=0; i--){
        if(!acq[i].empty()){
          proc = acq[i].front();
          acq[i].pop_front();
          break;
        }
      }
      
      return proc;

    }
      void test_preempt(Process*, int)
  {

  }
  void check_preemption(Process *, Process *, int )
  {
    
  }
};

class PREPRIO : public Scheduler {
  public:
    
    // vector<deque<Process *>> activeQ;
    // vector<deque<Process *>> expiredQ;
    list<list<Process*>> AcQ;
    list<list<Process*>> ExQ;
    list<Process*>* acq;
    list<Process*>* exq;

    PREPRIO() {
      // activeQ = vector<deque<Process *>>(maxPrio);
      // expiredQ =  vector<deque<Process *>>(maxPrio);
       AcQ = list<list<Process *>>(maxPrio);
      ExQ =  list<list<Process *>>(maxPrio);
      acq = new list<Process*>[maxPrio];
      exq = new list<Process*>[maxPrio];
    }

    void add_process(Process *p) {
      
      int dyn_prio_index = p->dynamicPrio;
      list<Process*> pi;
      if (!p->dp_enabled){
     //   AcQ[dyn_prio_index].push_back(p);
     acq[dyn_prio_index].push_back(p);
     //AcQ.push_back(pi);
      } else {
        //ExQ[dyn_prio_index].push_back(p);
        exq[dyn_prio_index].push_back(p);
      }
      p->dp_enabled = false;
		  
	  }

    void change_dyn_prio(Process *p) {
      p->dynamicPrio --;
      if (p->dynamicPrio == -1) {
        p->dynamicPrio = p->staticPrio - 1;
        p->dp_enabled = true;
      } else {
        p->dp_enabled = false;
      }
    }

    void swapQ(){
    
      list<Process*>* temp;
      temp = acq;
      acq = exq;
      exq = temp;
    }

    bool isActiveQEmpty(){
      for(int i = 0 ; i < maxPrio ; i++){
        if(!acq[i].empty()){
          return false;
        }
      }
      return true;
    }
    bool isExpiredQEmpty(){
      for(int i = 0 ; i < maxPrio ; i++){
        if(!exq[i].empty()){
          return false;
        }
      }
      return true;
    }
    Process * get_next_process(){
      if(isActiveQEmpty() && isExpiredQEmpty()){
        return NULL;
      }
      if(isActiveQEmpty()){
        swapQ();
      }
    
      Process * proc;

      for(int i= maxPrio-1; i >=0; i--){
        if(!acq[i].empty()){
          proc = acq[i].front();
          acq[i].pop_front();
          break;
        }
      }
      
      return proc;

    }
      void test_preempt(Process*, int)
  {

  }
  
  void check_preemption(Process *cur_running_pro, Process *currentProcess, int cur_time) {
    DES* des = new DES();
    int pro_next_ts = 0;
    if (cur_running_pro != nullptr) {
      pro_next_ts = des->get_next_event_ts(cur_running_pro);
      if (currentProcess->dynamicPrio > cur_running_pro->dynamicPrio) {
        if (pro_next_ts > cur_time) {

          des->remove_event(cur_running_pro);
          Event *e; 
          // e->p = cur_running_pro;
          // e->timestamp = cur_time;
          // e->trans = TRANS_TO_PREEMPT;
          e = new Event(cur_time, cur_running_pro,TRANS_TO_PREEMPT);
          //mydes.put_event(e);
          des->add_event(e);

          cur_running_pro->remainingCpuTime += (pro_next_ts - cur_time);
          if (cur_running_pro->remaining_cpu_burst <= 0) {
            cur_running_pro->remaining_cpu_burst = (pro_next_ts - cur_time);
          }
          else {
            cur_running_pro->remaining_cpu_burst += (pro_next_ts - cur_time);
          }
          
        }
      }
    }
  }
};

void read_randomfile(string randomfilename){ 
    //cout<<"inside random file"<<endl;
  rand_file.open(randomfilename.c_str());
  string line;
  char *ptr;
  getline(rand_file, line);
  int rand_num = stoi(line);
  tot_random_nums = rand_num;
  for(int i = 1; i <= rand_num; i++){
    getline(rand_file, line);
    int rand = stoi(line);
    random_nums.push_back(rand);
  }

}
void readfile(string filename, DES* des_level)
{
  //DES* des_level;
    int proc_id =0;
    int AT,TC,CB,IB=0;
    ifstream input;
    //input.open("C:/Users/TRISHA/Documents/C++/input1");
    input.open(filename);
    string line;
    //cout<<"inside readfile"<<endl;
    while (getline(input, line)){
        istringstream tokens(line);
        tokens >> AT >> TC >> CB >> IB;
        // cout<<AT<<endl;
        // cout<<TC<<endl;
        // cout<<CB<<endl;
        // cout<<IB<<endl;
        ptr = new Process();
        ptr->arrTime = AT;
       // cout<<"ptr arr time "<<ptr->arrTime<<endl;
        ptr->tot_cpuTime = TC;
        ptr->remainingCpuTime = TC;
        ptr->cpu_burst = CB;
        ptr->io_burst = IB;
        ptr->pID = proc_id;
        //cout<<"proc id :"<< ptr->pID<<endl;
        //ptr->remainingCpuTime = ptr->tot_cpuTime;
        //remaining cpu tim = cpu tot time
        proc_id++;
         //cout<<"proc id :"<< ptr->pID<<endl;
       ptr->staticPrio = myrandom(maxPrio);
        //cout<<"stat prio :"<< ptr->staticPrio<<endl;
        ptr->dynamicPrio = ptr->staticPrio-1;
        //evt = new Event();
        processes.push_back(ptr);
        Event *e; 
        // e->event_proc = ptr;
        // e->eventTimestamp = ptr->arrTime;
        // e->trans_state = TRANS_TO_READY;
        e = new Event(ptr->arrTime, ptr, TRANS_TO_READY);
        //cout<<"event ID :"<<e->event_proc->pID<<"event time "<<e->eventTimestamp<<endl;
        //mydes.add_event(e);
        des_level = new DES();
       des_level->add_event(e);
        
    }
    input.close();
    

}

void calculateIOBlock()
{

}

void simulation(DES* des_level){
   // cout<<"----inside simulation----"<<endl;
  Event *event;
  bool call_sched = false;
 //Process* current_running_process = NULL;
  int start_time = 0;
  int end_time = 0;
  //cout<<"Inside simulation"<<endl;
  int timeInPrevState;

  //while(event = mydes.get_event()){
    while(event = des_level->get_event()){
    Process *pro = event->event_proc;
    current_time = event->eventTimestamp;
    //timeInPrevState = current_time - pro->state_ts;
    //cout<<"getting event from event Q event process id: "<<pro->pID<<endl;
    //cout<<"\n\ngetting event from event Q event process id: "<<pro->pid<<endl;
    //cout<<"get event trans is : "<<event->trans<<endl;

    switch(event->trans_state){
      case TRANS_TO_READY: 
        {
          sched->check_preemption(current_running_process, pro,  current_time);
            //cout<<"In ready"<<endl;
          pro->current_state = READY;

          call_sched = true;
          sched->add_process(pro);
         sched->print_runq();
          break;
        }
      case TRANS_TO_RUN:
        {
            //cout<<"In run"<<endl;
          int current_process_cpu_burst = pro->cpu_burst;
          int time_run;
          if(pro->remaining_cpu_burst > 0){
            time_run = pro->remaining_cpu_burst;
          } else {
            time_run = myrandom(current_process_cpu_burst);
          }
          int process_remaining_cpu_time = pro->remainingCpuTime;
          //cout<<"cb: "<<time_run<<endl;

          if(quantum < time_run)
          {
            if(quantum < process_remaining_cpu_time)
            {
                 int new_time = current_time+quantum;
            pro->remainingCpuTime -= quantum;
            pro->remaining_cpu_burst =time_run- quantum;
            //cout<<"remaining cpu burst for "<<pro->pid<<" "<<pro->remaining_cpu_burst<<endl;
            Event * e ;
            
            e = new Event(new_time, pro, TRANS_TO_PREEMPT);
           // mydes.add_event(e);
            des_level->add_event(e);
            }
          }
          if(time_run < process_remaining_cpu_time)
          {
            if(time_run <= quantum)
            {
                 int new_time = current_time + time_run;
            pro->remainingCpuTime -= time_run;
            pro->remaining_cpu_burst -= time_run;
            Event * e;
             
            
             e = new Event(new_time, pro, TRANS_TO_BLOCK);
            //mydes.add_event(e);
            des_level->add_event(e);
            }
          }
          if(process_remaining_cpu_time <= time_run )
          {
            if(process_remaining_cpu_time <= quantum)
            {
            int new_time = current_time + process_remaining_cpu_time;
            pro->remainingCpuTime = 0;
            
            Event *e;
         
            e = new Event(new_time, pro, TRANS_TO_DONE);
            //mydes.add_event(e);
           des_level->add_event(e);
            }
          }





          // if(process_remaining_cpu_time <= time_run && process_remaining_cpu_time <= quantum){
          // //if(process_remaining_cpu_time <= time_run)  {
          //   int new_time = current_time + process_remaining_cpu_time;
          //   pro->remainingCpuTime = 0;
            
          //   Event *e;
         
          //   e = new Event(new_time, pro, TRANS_TO_DONE);
          //   //mydes.add_event(e);
          //  des_level->add_event(e);
            
          // } else if(time_run < process_remaining_cpu_time && time_run <= quantum){
          // //else{  
          //   int new_time = current_time + time_run;
          //   pro->remainingCpuTime -= time_run;
          //   pro->remaining_cpu_burst -= time_run;
          //   Event * e;
             
            
          //    e = new Event(new_time, pro, TRANS_TO_BLOCK);
          //   //mydes.add_event(e);
          //   des_level->add_event(e);

          // } else if(quantum < time_run && quantum < process_remaining_cpu_time){
          //   //cout<<"here here here"<<endl;
          //   int new_time = current_time+quantum;
          //   pro->remainingCpuTime -= quantum;
          //   pro->remaining_cpu_burst =time_run- quantum;
          //   //cout<<"remaining cpu burst for "<<pro->pid<<" "<<pro->remaining_cpu_burst<<endl;
          //   Event * e ;
            
          //   e = new Event(new_time, pro, TRANS_TO_PREEMPT);
          //  // mydes.add_event(e);
          //   des_level->add_event(e);

          // } 
          break;
        }
      case TRANS_TO_PREEMPT:
        {
            //cout<<"In preempt"<<endl;
            sched->change_dyn_prio(pro);
          call_sched = true;
          sched->add_process(pro);
          current_running_process = NULL;
          pro->current_state = READY;
          break;
        }
      case TRANS_TO_BLOCK:
        {
          //cout<<"In block"<<endl;
          int current_process_io_burst = pro->io_burst;
          int actual_io_burst = myrandom(current_process_io_burst);
          pro->total_time_spent_in_io += actual_io_burst;
          int new_time = current_time + actual_io_burst;
          start_time = current_time;
          new_io_burst = actual_io_burst;
          if(start_time < end_time)
          {
            //cout<< "inside the start loop --------"<<endl;
              int overlap = end_time- start_time;
              if(overlap< new_io_burst)
              {
               // cout<< "inside the overlap loop --------"<<endl;
                process_io_counter = process_io_counter+ new_io_burst - overlap;
                end_time = start_time+ new_io_burst;
              }
          }
          else
          {
            process_io_counter = process_io_counter + new_io_burst;
            end_time = start_time + new_io_burst;
          }
         

         
        pro->dynamicPrio = pro->staticPrio - 1;
          Event *e1 ;
       
         e1 = new Event(new_time, pro, TRANS_TO_READY);
          //mydes.add_event(e1);

          des_level->add_event(e1);

     

          current_running_process = NULL;
          call_sched = true;
          pro->current_state = BLOCKED;
          break;
        }
      case TRANS_TO_DONE:
        {
          //  cout<<"In done"<<endl;
          pro->finishTime = current_time;
          current_running_process = NULL;
          call_sched = true;
          pro->current_state = DONE;
          break;
        } 

    }

    delete event;
    event = NULL;
    //cout<<"Scheduler runQ size : "<<scheduler->runQ.size()<<endl;
    if(call_sched){
      if(!eventQ.empty() && eventQ.front()->eventTimestamp == current_time){
        continue;
      }
      call_sched = false;
      if(current_running_process == nullptr){
        current_running_process = sched->get_next_process();
        if(current_running_process == nullptr) 
          continue;
        //cout<<"current running process pid is:"<<current_running_process->pid<<endl;
        Event *e ;
       
         e = new Event(current_time, current_running_process, TRANS_TO_RUN);
       // mydes.add_event(e);
       des_level->add_event(e);
        current_running_process->current_state = RUNNING;
          
      } 
    }
  }
  //cout<<"---Outside simulation----"<<endl;
}

void printSummary()
{
    int last_event_finish_time = -1;
    int overall_total_cpu_time = 0;
    int overall_total_io_time = 0;
    int overall_total_tat = 0;
    int overall_cpu_wait_time = 0;
    int count = 0;
    double io_util = 0;
    for(processIt=processes.begin(); processIt!= processes.end();processIt++)
    {
        //processes[i]->printprocc();
        Process* p = *processIt;
        
        int tat = p->finishTime - p->arrTime;
        int cpu_waiting_time = tat - p->tot_cpuTime - p->total_time_spent_in_io;

        if(p->finishTime > last_event_finish_time){
            last_event_finish_time = p->finishTime;
        }
        overall_total_cpu_time += p->tot_cpuTime;
        overall_total_io_time += p->total_time_spent_in_io;
        overall_total_tat += tat;
        overall_cpu_wait_time += cpu_waiting_time;
        count++;
        
        printf("%04d: %4d %4d %4d %4d %1d | %5d %5d %5d %5d\n", p->pID, p->arrTime, p->tot_cpuTime, p->cpu_burst, p->io_burst, p->staticPrio, p->finishTime, tat, p->total_time_spent_in_io, cpu_waiting_time);

        
    }

        double cpu_utilization = (overall_total_cpu_time/(double)last_event_finish_time)*100;
        double io_utilization = (overall_total_io_time/(double)last_event_finish_time)*100;
        double avg_tat = overall_total_tat / (double)count;
        double avg_cpu_wait_time = overall_cpu_wait_time / (double)count;
        double throughput = (count/(double)last_event_finish_time) * 100;
        io_util = (process_io_counter/(double)last_event_finish_time)*100;
        printf("SUM: %d %.2lf %.2lf %.2lf %.2lf %.3lf\n",last_event_finish_time,cpu_utilization,io_util,avg_tat,avg_cpu_wait_time,throughput);
       // cout<<"process io counter----"<< process_io_counter<<endl;

}

int main(int argc, char* argv[])
{
  
     //int q = 10000;
      
    string schedulingAlgo="";
    string num;
    int c = 0;

      opterr = 0;
      while ((c = getopt (argc, argv, "vteps:")) != -1)
        switch (c)
          {
          case 'v':
            verbosemode = 1;
            break;
          case 's':
            schedulingAlgo = string(optarg);
            
            break;
            default:
            break;
          }

          
  
            for (int i=0;i<schedulingAlgo.length();i++)
            {
              if (schedulingAlgo.at(i)=='F')
              {
                 cout<<"FCFS"<<endl;
                 sched= new FCFS();

              }
              if (schedulingAlgo.at(i)=='L')
              {
                 cout<<"LCFS"<<endl;
                 sched= new LCFS();

              }
              if (schedulingAlgo.at(i)=='S')
              {
                 cout<<"SRTF"<<endl;
                 sched= new SRTF();

              }
              if (schedulingAlgo.at(i)=='R')
              {
                num = schedulingAlgo.substr(0,1);
                sscanf(schedulingAlgo.substr(1).c_str(),"%d",&quantum);
                //printf("quantum:%d \n", quantum);
                cout<<"RR"<<" "<<to_string(quantum)<<endl;
                sched= new RR();

              }
              if (schedulingAlgo.at(i)=='P')
              {
                num = schedulingAlgo.substr(0,1);
                sscanf(schedulingAlgo.substr(1).c_str(),"%d:%d",&quantum, &maxPrio);
               // printf("quantum:%d \n", quantum);
                //printf("max prio:%d \n", maxPrio);
                cout<<"PRIO"<<" "<<to_string(quantum)<<endl;
                sched= new PRIO();

              }
               if (schedulingAlgo.at(i)=='E')
              {
                num = schedulingAlgo.substr(0,1);
                sscanf(schedulingAlgo.substr(1).c_str(),"%d:%d",&quantum, &maxPrio);
                //printf("quantum:%d \n", quantum);
                //printf("max prio:%d \n", maxPrio);
                cout<<"PREPRIO"<<" "<<to_string(quantum)<<endl;
                sched= new PREPRIO();

              }
             
            }
            
    
    string filename =  string(argv[argc-2]);
    //cout<<"filename  "<<filename<<endl;

    string randfile =  string(argv[argc-1]); 
    //cout<<"randfile  "<<randfile<<endl;

   

    DES* des_level = new DES();
   
    read_randomfile(randfile);
    readfile(filename, des_level);
  
    //  cout<<"process size is -> "<< processes.size()<<endl;
    //   mydes.print_all_event();
    simulation(des_level);
    printSummary();
    
    
}


//trans-run simu chnge
