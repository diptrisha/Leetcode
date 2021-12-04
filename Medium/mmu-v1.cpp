#include <iostream>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <fstream>
#include <stdio.h>
#include <getopt.h>
#include <bits/stdc++.h>

using namespace std;
#define TAU 49 
#define NRU_SIZE = 4;
//bool options
bool O_option = false;
bool P_option = false; //pagetable option
bool F_option = false; //frame table option
bool S_option = false; //summary line
bool x_option = false; //prints the current page table after each instructions (see example outputs) and this should help you significantly to track down bugs and transitions (remember you write the print function only once
bool y_option = false; //like ‘x’ but prints the page table of all processes instead.
bool f_option = false; //prints the frame table after each instruction.
bool a_option = false; //prints additional “aging” information during victim_selecton and after each instruction for complex algorithms

//other globals
const int pte_num = 64; //page table entries
//int num_frames = 128;   //num frames, max = 128

int algChoice;          //choice of scheduler
int rand_nums;          //total random nums in file
int ins_counter = 0;    //instruction counter
int choose_printing;
int current_proc_number = 0;

unsigned long long total_cost = 0;
unsigned long long total_cost2 = 0;
unsigned long long total_inst_count = 0;
unsigned long long total_context_switches = 0;
unsigned long long total_proc_exits = 0;
unsigned long long rwaccesses = 0;
int num_frames;
deque<int> randoms;
int rand_num;
int frames_max_size = 128; 
int rofs = 0;
ifstream rand_file;
vector<int> randVals;
vector <int> random_nums;
void finalOutput();
// int get_next_random()
// {
//     if (rofs == rand_num)
//     {
//         rofs = 0;
//     }
//     //cout<<randoms.size()<<rand_nums<<endl;
//     int ans = random_nums[rofs] % num_frames;
//     ++rofs;
//     return ans;
// }
int offset=0;
int get_next_random(int burst)
{
    if (offset == rand_nums)
    {
        offset = 0;
    }
    //cout<<randoms.size()<<rand_nums<<endl;
    //int ans = randoms[offset++] % burst;
    // ++rofs;
    return randoms[offset++] % burst;
}
struct inst
{
    char instr;
    int addr;
};
vector<inst> instructions;
class Instruction
{
    public:
        char command;
        int page;
};

vector<Instruction> instructionList;
// struct PTE
// {
//     // unsigned int present : 1; // present/valid bit
//     // unsigned int write_protect : 1;
//     // unsigned int modified : 1;
//     // unsigned int referenced : 1;
//     // unsigned int pagedout : 1;
//     // unsigned int frame_number : 7;
//     // unsigned int filemap : 1;
//     // unsigned int hole : 1;

//     // void setPTE()
//     // {
//     //     present = 0;
//     //     write_protect = 0;
//     //     modified = 0;
//     //     referenced = 0;
//     //     pagedout = 0;
//     //     frame_number = 0;
//     //     filemap = 0;
//     //     hole = 0;
//     // }
// };
class PTE
{
    public:
        unsigned int present : 1; // present/valid bit
        unsigned int write_protect : 1;
        unsigned int modified : 1;
        unsigned int referenced : 1;
        unsigned int pagedout : 1;
        unsigned int frame_number : 7;
        unsigned int filemap : 1;
        unsigned int hole : 1;

        void setPTE()
        {
            present = 0;
            write_protect = 0;
            modified = 0;
            referenced = 0;
            pagedout = 0;
            frame_number = 0;
            filemap = 0;
            hole = 0;
        }
        

};
// struct FTE
// {
//     int frameNum;
//     int procID;
//     int vpage;
//     unsigned long age;
//     bool victimed;
//     unsigned long tau;
// };
// deque<FTE> frameTable;
// deque<FTE *> freeFrames;
class FTE
{
    public:
        int frameNum;
        int vpage;
        int procID;
        bool victim_frame;
        unsigned long age;
        unsigned long tau;

        void setFrame(int vpageno, int pID,  unsigned long age, unsigned long tau)
        {
            //frameNum = fno;
            vpage = vpageno;
            procID = pID;
            //victim_frame = vFrame;
            age = age;
            tau = tau;

        }
};
deque<FTE> frameTable;
deque<FTE *> freeFrames;
//queue<Frame*> frame_tbl;
// Frame *frame_tbl;
// queue<Frame*> free_pool_frames;
int usedFramesCt = 0;

struct VMA
{
    unsigned int start_vpage : 7;
    unsigned int end_vpage : 7;
    unsigned int write_protected : 1;
    unsigned int file_mapped : 1;
};
class VirtualMemoryAddress
{
    public:
        int starting_virtual_page;
        int ending_virtual_page;
        int write_protected;
        int file_mapped;

        // VirtualMemoryAddress(int svp, int evp, int wp, int fm)
        // {
        //     starting_virtual_page = svp;
        //     ending_virtual_page = evp;
        //     write_protected = wp;
        //     file_mapped = fm;
        // }
        void setVMA(int svp, int evp, int wp, int fm)
        {
            starting_virtual_page = svp;
            ending_virtual_page = evp;
            write_protected = wp;
            file_mapped = fm;
        }

};
class Process
{
public:
    vector<VirtualMemoryAddress*> virtualMemoryAreasList;
    vector<PTE> pageTable;
    int pid;
    unsigned long long unmaps;
    unsigned long long maps;
    unsigned long long pageins;
    unsigned long long pageouts;
    unsigned long long fins;
    unsigned long long fouts;
    unsigned long long zeros;
    unsigned long long segv;
    unsigned long long segprot;

    void setProcess()
    {
        for (int i = 0; i < pte_num; i++)
        {
            PTE e;      //empty page table entry
            e.setPTE(); //initialize with empty vals
            e.frame_number = i;
            pageTable.push_back(e);
        }

        pid = unmaps = maps = pageins = pageouts = fins = fouts = zeros = segv = segprot = 0;
    }
    void print()
    {
         printf("PROC[%d]: U=%llu M=%llu I=%llu O=%llu FI=%llu FO=%llu Z=%llu SV=%llu SP=%llu\n", pid,
                   unmaps, maps, pageins, pageouts, fins, fouts, zeros, segv, segprot);

    }
};
Process* ptr;
vector<Process*> proc_list;
vector<Process> processes;
Process *current_process;
class BasePager
{
public:
    void frameTableinit()
    {
        for (int i = 0; i < num_frames; i++) //instantiated the frame table
        {
            FTE f;
            f.frameNum = i;
            f.procID = -1;
            f.vpage = -1;
            f.victim_frame = false;
            f.age = 0;
            f.tau = 0;
            frameTable.push_back(f);
        }

        for (int i = 0; i < num_frames; i++) //instantiated free frame pointers to the frame table
        {
            FTE *f;
            f = &frameTable[i];
            freeFrames.push_back(f);
        }
    }
    FTE *get_frame()
    {
        FTE *f;
        if (freeFrames.size() != 0)
        {
            f = freeFrames.front();
            freeFrames.pop_front();
            usedFramesCt++;
        }
        else
        {
            //need to get victim frame

            f = getVictimFrame();
        }

        return f;
    }
    virtual FTE *getVictimFrame() = 0;
};

// class FIFO : public BasePager
// {
// public:
//     FTE *fileptr;
//      int hand;
//      FIFO(){

//         fileptr = &frameTable.front(); 
//         int hand = 0;}

//      FTE * getVictimFrame()
//      {
//           FTE* victim = fileptr;
//           hand = hand+1;
//           if(hand == frameTable.size())
//           {
//               hand =0;
//               fileptr = &frameTable.front();
//           }
//           //hand = (hand+1)%num_frames;
//           fileptr = &frameTable[hand];
//           victim->victim_frame = true;
//           // cout<<"VICTIM FRAME"<<endl;
//           return victim;
//      }

// };

class FIFO : public BasePager //done
{
public:
    FTE *pointer;
    int size;
    int ct;

    FIFO()
    {
        ct = 0;
        size = frameTable.size();
        pointer = &frameTable.front();
    }
    FTE *getVictimFrame()
    {

        FTE *victim = pointer;
        ct++;
        if (ct == size)
        {
            ct = 0;
            pointer = &frameTable.front();
        }
        pointer = &frameTable[ct];
        victim->victim_frame = true;
        return victim;
    }
};

class Random : public BasePager 
{
public:
    Random()
    {
        
    }
    FTE *getVictimFrame()
    {
        int vf = get_next_random(num_frames);
        frameTable[vf].victim_frame = true;
        FTE *f = &frameTable[vf];
        return f;
    }
};

class Clock : public BasePager
{
public:
    FTE *pointer;// give meaningful names 
    FTE *ans;
    int size;
  
    int hand;

    Clock()
    {
 
        hand = 0;
        size = frameTable.size();// remove this
        pointer = &frameTable[0];
    }
    // bool checkIfReferenced(PTE *correspondingPage)
    // {
    //     if(correspondingPage->referenced == 1)
    //         return true;
    //     return false;
    // }
    // FTE *setNotRef(FTE *ans, int ct, FTE *ptr)
    // {
    //   //  cout<<"inside setnotref"<<endl;
    //         ans->victim_frame = true;
    //         ct++;
    //         if (ct == size)
    //             ct = 0;
    //         ptr = &frameTable[ct];
    //         return ans;
    // }
    // int incre(int count)
    // {
    //     return count+1;
    // }
    FTE *getVictimFrame()
    {
        //PTE *correspondingPage = &processes[pointer->procID].pageTable[pointer->vpage];
        PTE *correspondingPage = &proc_list[pointer->procID]->pageTable[pointer->vpage];
       // bool refFlag = checkIfReferenced(&proc_list[pointer->procID]->pageTable[pointer->vpage]);
        while (correspondingPage->referenced == 1)
        {
            correspondingPage->referenced = 0;
            hand = (hand+1)% size;
            pointer = &frameTable[hand];
        
            correspondingPage = &proc_list[pointer->procID]->pageTable[pointer->vpage];
        }
            
            ans = &frameTable[correspondingPage->frame_number];
            //setNotRef(ans, ct, pointer);
            ans->victim_frame = true;
           
            hand = (hand+1)% size;
            pointer = &frameTable[hand];

        return ans;
    }
};

class NRU : public BasePager
{
public:
    int handpos;
    int nruClass[4];
    //int NRU_bit_class[NRU_SIZE];
    FTE *victim;
    int last;
    NRU()
    {
        handpos = 0;
        last = 0;
        victim = NULL;
      
    }
    int calc_classBits(int class_mod, int class_ref)
    {
        class_ref = 2 * class_ref;
        int sum =  class_mod + class_ref;
        return sum;
    }
    void resetBits()
    {
      
       nruClass[0] = -1;
       nruClass[1] = -1;
       nruClass[2] = -1;
       nruClass[3] = -1;
    }
    FTE *getVictimFrame()
    {
        bool resetframe = true;
        resetBits();
        bool victimSet = false;
        
        //victim calc
        for (int y = 0; y < frameTable.size(); y++)
        {
            
            FTE *current_frame = &frameTable[(y + handpos) % frameTable.size()];
         
          
            PTE *current_page = &proc_list[current_frame->procID]->pageTable[current_frame->vpage];
            int cID = calc_classBits(current_page->modified,current_page->referenced);
           
            if((nruClass[cID] == -1) && (cID !=0))
            {
                nruClass[cID] = current_frame->frameNum;
            }
            else if ((nruClass[cID] == -1) && (cID ==0))
            {
                victim = current_frame;
                    victim->victim_frame = true;
                    victimSet = true;
                    resetframe = false;
                    handpos = ((y + handpos) % frameTable.size() + 1) % frameTable.size();
                    break;
            }
            
        }

        if(resetframe)
        {
            for (int i = 0; i < 4; i++)
            {
                if(nruClass[i] == -1)
                {
                    continue;
                }
               else
                {
                    victim = &frameTable[nruClass[i]];
                    victim->victim_frame = true;
                    victimSet = true;
                    handpos = (victim->frameNum + 1) % frameTable.size();
                    break;
                }
            }
        }
        // if (!victimSet)
        // {
        //     for (int i = 0; i < 4; i++)
        //     {
        //         if (nruClass[i] != -1)
        //         {
        //             victim = &frameTable[nruClass[i]];
        //             victim->victim_frame = true;
        //             victimSet = true;
        //             handpos = (victim->frameNum + 1) % frameTable.size();
        //             break;
        //         }
        //     }
        // }

        if (ins_counter - last >= 50)
        {
            for (int i = 0; i < frameTable.size(); i++)
            {
                FTE *f = &frameTable[i];
                if (f->procID != -1)
                
                    proc_list[f->procID]->pageTable[f->vpage].referenced = 0;
            }
            last = ins_counter;
        }

        return victim;
    }
};

class Aging : public BasePager
{
public:
    int handpos;
    FTE *victim;
    
    Aging()
    {
        handpos = 0; //initial
    }
    int getMaxNoOfFrames()
    {
        int MX_NO_FRAMES = frameTable.size();
        return MX_NO_FRAMES;
    }
    unsigned long calcAge(unsigned long a)
    {
        return a>> 1;
    }
    FTE *getVictimFrame()
    {
        victim = &frameTable[handpos];

        //s1 lowest age
        for (int i = 0; i < getMaxNoOfFrames(); i++)
        {
            int id = (i + handpos) % getMaxNoOfFrames();
            FTE *f = &frameTable[id];
            f->age = calcAge(f->age);
           
            if (proc_list[f->procID]->pageTable[f->vpage].referenced == 1)
            {
                f->age = (f->age | 0x80000000);
               
                proc_list[f->procID]->pageTable[f->vpage].referenced = 0;
            }

            if (f->age < victim->age) //maintain ord
            {
                victim = f;
            }
        }

        handpos = (victim->frameNum + 1) % getMaxNoOfFrames();
        victim->victim_frame = true;
        return victim;
    }
};

class Working_Set : public BasePager
{
public:
    FTE *currentFTE;
    PTE *currentPTE;

    int hand;
    

    Working_Set()
    {
        hand = 0;
 
    }
    int getMaxNoOfFrames()
    {
        int MX_NO_FRAMES = frameTable.size();
        return MX_NO_FRAMES;
    }
    
    FTE *getVictimFrame()
    {   
        
        int minimum_time = 0;
        
        FTE *lastFrame = nullptr;
        FTE *currFrame = &frameTable[hand];
        for (int i = 0; i < getMaxNoOfFrames(); i++)
        {
            int curFrameID = (i + hand) % getMaxNoOfFrames();
            currentFTE = &frameTable[(i + hand) % getMaxNoOfFrames()];
          
            currentPTE = &proc_list[currentFTE->procID]->pageTable[currentFTE->vpage];
            int age = ins_counter - 1 - currentFTE->tau;
            //int age = -1;
            if(!currentPTE->referenced)
            {
                if((age< (TAU+1)) && (age > minimum_time))
                {
                    
                        minimum_time = age;
                       
                        lastFrame = currentFTE;
                    
                }
                if (age >= (TAU+1))
                {
                
                    lastFrame = currentFTE;
                    
                    lastFrame->victim_frame = true;
                
                    hand = (lastFrame->frameNum + 1) % getMaxNoOfFrames();
                    return lastFrame;
                }

            }
            if (currentPTE->referenced ==1)
            {
                currentPTE->referenced = 0;
                currentFTE->tau = ins_counter - 1; 
            }
           
        }
        
        if(lastFrame == nullptr)
        {
            lastFrame = currFrame;
        }
        hand = (lastFrame->frameNum + 1) % getMaxNoOfFrames();
        lastFrame->victim_frame = true;
       
        return lastFrame;
    }
    
};
// class Working_Set : public BasePager
// {
// public:
//     FTE *currentFTE;
//     PTE *currentPTE;
//     FTE *ans;
//     int hand;
//     int size;
//     int threshold;
//     int min;

//     Working_Set()
//     {
//         hand = 0;
//         size = frameTable.size();
//         threshold = 50;
//         ans = NULL;
//         min = -1000;
//     }
//     FTE *getVictimFrame()
//     {   
//         min = -1000;
//         ans = NULL;
//         //ans = &frameTable[hand];
//         for (int i = 0; i < frameTable.size(); i++)
//         {
//             int curFrameID = (i + hand) % frameTable.size();
//             currentFTE = &frameTable[curFrameID];
//             //currentPTE = &processes[currentFTE->procID].pageTable[currentFTE->vpage];
//             currentPTE = &proc_list[currentFTE->procID]->pageTable[currentFTE->vpage];
//             int age = ins_counter - 1 - currentFTE->tau;
//             if (currentPTE->referenced)
//             {
//                 currentPTE->referenced = 0;
//                 currentFTE->tau = ins_counter - 1; //wout working with chk
//             }
//             else
//             {
//                 if (age >= 50)
//                 {
//                     ans = currentFTE;
//                     ans->victim_frame = true;
//                     //ans->tau = ins_counter;
//                     hand = (ans->frameNum + 1) % frameTable.size();
//                     return ans;
//                 }
//                 else
//                 {
//                     if (age > min)
//                     {
//                         min = age;
//                         ans = currentFTE;
//                     }
//                 }
//             }
//         }
//         if(ans == NULL)
//         {
//             ans = &frameTable[hand];
//         }
//         hand = (ans->frameNum + 1) % frameTable.size();
//         ans->victim_frame = true;
//         //ans->tau = ins_counter;
//         return ans;
//     }
// };

BasePager *pager;

// int reader(string input_file, string random_file)
// {
//     string s;
//     int num_proc;
//     int num_vmas;
//     int start_page;
//     int end_page;
//     int write_protected;
//     int file_mapped;
//     ifstream in(input_file);
//     if (!in)
//     {
//         cout << "Cannot open input file.\n";
//         exit(0);
//     }
//     else
//     {
//         getline(in, s);
//         while (s[0] == '#')
//             getline(in, s);
//         num_proc = atoi(&s[0]);
//         getline(in, s);
//         while (s[0] == '#')
//             getline(in, s);

//         for (int i = 0; i < num_proc; i++) //procRead
//         {
//             Process p;
//             p.setProcess();
//             p.pid = i;
//             num_vmas = atoi(&s[0]);
//             getline(in, s);
//             for (int j = 0; j < num_vmas; j++)
//             {
//                 char *pch;
//                 pch = strtok(&s[0], " ");
//                 start_page = atoi(pch);
//                 pch = strtok(NULL, " ");
//                 end_page = atoi(pch);
//                 pch = strtok(NULL, " ");
//                 write_protected = atoi(pch);
//                 pch = strtok(NULL, " ");
//                 file_mapped = atoi(pch);
//                 VMA v;
//                 v.start_vpage = start_page;
//                 v.end_vpage = end_page;
//                 v.write_protected = write_protected;
//                 v.file_mapped = file_mapped;
//                 p.virtualMemoryAreasList.push_back(v);
//                 getline(in, s);
//                 while (s[0] == '#')
//                     getline(in, s);
//             }

//             // for (int y = 0; y < p.virtualMemoryAreasList.size(); y++)
//             // {
//             //     cout << "sdfsd";
//             //     VMA temp = p.virtualMemoryAreasList[y];
//             //     for (int x = temp.start_vpage; x <= temp.end_vpage; x++)
//             //     {
//             //         p.pageTable[x].hole = 0;
//             //     }
//             // }

//             processes.push_back(p);
//         }

//         if (s[0] == '#')
//             getline(in, s);
//         else
//             while (s[0] != '#' && in)
//             {
//                 inst ins;
//                 char *pch;
//                 pch = strtok(&s[0], " ");
//                 ins.instr = *pch;
//                 pch = strtok(NULL, " ");
//                 ins.addr = atoi(pch);
//                 instructions.push_back(ins);
//                 getline(in, s);
//             }
//     }
//     in.close();
//     ifstream in2(random_file);
//     if (!in2)
//     {
//         cout << "cannot open rand file";
//         exit(0);
//     }
//     else
//     {
//         getline(in2, s);
//         rand_nums = atoi(&s[0]);
//         while (getline(in2, s))
//         {
//             randoms.push_back(atoi(&s[0]));
//         }
//     }
// }

// void frameTableinit()
// {
//     for (int i = 0; i < num_frames; i++) //instantiated the frame table
//     {
//         FTE f;
//         f.frameNum = i;
//         f.procID = -1;
//         f.vpage = -1;
//         f.victimed = false;
//         f.age = 0;
//         f.tau = 0;
//         frameTable.push_back(f);
//     }

//     for (int i = 0; i < num_frames; i++) //instantiated free frame pointers to the frame table
//     {
//         FTE *f;
//         f = &frameTable[i];
//         freeFrames.push_back(f);
//     }
// }
// void allocateFrameTable()
//  {
//      cout<<"no of frame"<<num_frames<<endl;
//     //FrameTable = new Frame[mx_frames];
//     frame_tbl = new Frame[num_frames];
//     for(int i=0;i<num_frames;i++)
//     {
       
//         frame_tbl[i].frame_no = i;
//         frame_tbl[i].proc_id = -1;
//         frame_tbl[i].virtual_page_no = -1;
//         frame_tbl[i].age = 0;
//         frame_tbl[i].tau = 0;
//         frame_tbl[i].victim_frame = false;
//         free_pool_frames.push(&frame_tbl[i]);
        

//         // Frame *frame;
//         // frame->frame_no = i;
//         // frame->proc_id = -1;
//         // frame->virtual_page_no = -1;
//         // frame->age = 0;
//         // frame->tau = 0;
//         // frame->victim_frame = false;
//         // frame_tbl.push(frame);
        
        
//        // free_pool_frames.push(frame_tbl.);

//     }
//     // for(int i=0; i<mx_frames; i++)
//     // {
//     //     FrameTable[i] = *(new Frame);
//     //     FrameTable[i].frameNo = i;
//     //     FrameTable[i].process = NULL;
//     //     FrameTable[i].vpage = NULL;
//     //     FreeList.push(&FrameTable[i]);
//     // }
//  }

void checker()
{
    for(int i =0; i< proc_list.size();i++)
    {
        Process *p = proc_list[i];
        cout<< "Proc id "<<p->pid<<endl;
         for (int j = 0; j < p->virtualMemoryAreasList.size(); j++)
         {
            cout<<"start page "<<p->virtualMemoryAreasList[j]->starting_virtual_page<<endl;
            cout<<"end page "<<p->virtualMemoryAreasList[j]->ending_virtual_page<<endl;
            cout<<"write protected "<<p->virtualMemoryAreasList[j]->write_protected<<endl;
            cout<<"file mapped "<<p->virtualMemoryAreasList[j]->file_mapped<<endl;
         }
         cout<<endl;
    }
    // for (int i = 0; i < processes.size(); i++)
    // {
    //     Process x = processes[i];
    //     cout << x.pid << " " << endl;

    //     for (int j = 0; j < x.virtualMemoryAreasList.size(); j++)
    //     {
    //         cout << x.virtualMemoryAreasList[j].start_vpage << " " << x.virtualMemoryAreasList[j].end_vpage << " " << x.virtualMemoryAreasList[j].write_protected << " " << x.virtualMemoryAreasList[j].file_mapped << endl;
    //     }

    //     for (int k = 0; k < x.pageTable.size(); k++)
    //     {
    //         cout << x.pageTable[k].frame_number << " : " << x.pageTable[k].hole << endl;
    //     }

    //     cout << endl;
    //     cout << endl;
    // }
}

void printPageTable(int pid)
{
   // Process x = processes[pid];
    Process *x = proc_list.at(pid);
    cout << "PT[" << pid << "]:";
    for (int i = 0; i < pte_num; i++)
    {
        if (x->pageTable[i].present == 1)
        {
            cout << " "<<i << ":";

            if (x->pageTable[i].referenced == 1)
                cout << "R";
            else
                cout << "-";
            if (x->pageTable[i].modified == 1)
                cout << "M";
            else
                cout << "-";
            if (x->pageTable[i].pagedout == 1)
                cout << "S";
            else
                cout << "-";
        }
        else
        {
            if (x->pageTable[i].pagedout == 1)
                cout << " #";
            else
                cout << " *";
        }
    }
    cout << endl;
}

void printFrameTable()
{
    cout << "FT:";
    for (int i = 0; i < frameTable.size(); i++)
    {
        if (frameTable[i].procID != -1)
        {
            cout <<" " <<frameTable[i].procID << ":" << frameTable[i].vpage ;
        }
        else
        {
            cout << " *";
        }
    }
    cout << endl;
}

// FTE *get_frame()
// {
//     FTE *f;
//     if (freeFrames.size() != 0)
//     {
//         f = freeFrames.front();
//         freeFrames.pop_front();
//         usedFramesCt++;
//     }
//     else
//     {
//         //need to get victim frame

//         f = pager->getVictimFrame();
//     }

//     return f;
// }
void setFTEFree(FTE *ff)
{
    ff->age = 0;
    ff->procID = -1;
    ff->victim_frame = false;
    ff->vpage = -1;
    ff->tau = 0;
    freeFrames.push_back(ff);

}
void setPTEBits(PTE *p)
{
    p->present = 0;
    p->referenced = 0;
    p->pagedout = 0;

}
bool segmentationFaultHandle(PTE *pte, Process* curr_proc, int cur_vpage, bool SEGV_Error)
{
    //bool SEGV_Error = true;
            for (int k = 0; k < curr_proc->virtualMemoryAreasList.size(); k++)
                {
                    if (cur_vpage >= curr_proc->virtualMemoryAreasList[k]->starting_virtual_page && cur_vpage <= curr_proc->virtualMemoryAreasList[k]->ending_virtual_page)
                    {
                        
                        SEGV_Error = false;
                        pte->filemap = curr_proc->virtualMemoryAreasList[k]->file_mapped;
                        pte->write_protect = curr_proc->virtualMemoryAreasList[k]->write_protected;
                        pte->hole = 1;
                        break;
                    }
                }
            

           return SEGV_Error;
           
}
void victimFrameSelect(FTE *newframe)
{
        if (newframe->victim_frame) //this means we are dealing with a victimed frame not fresh frame
            {
                int reverse_map_vpage = newframe->vpage;
                int reverse_map_procid = newframe->procID;

                //PTE *p = &processes[reverse_map_procid].pageTable[reverse_map_vpage];
                PTE *p = &proc_list[reverse_map_procid]->pageTable[reverse_map_vpage];

                p->present = 0;
               // processes[reverse_map_procid].unmaps++;
                proc_list[reverse_map_procid]->unmaps++;

                //unmap over
                total_cost2 += 400;
                if (O_option)
                    cout << " UNMAP " << reverse_map_procid << ":" << reverse_map_vpage << endl;


                if(p->modified &&  !p->filemap)
                {
                         proc_list[reverse_map_procid]->pageouts++;
                        p->modified = false;
                        p->pagedout = true;
                        total_cost2 += 2700;
                        if (O_option)
                            cout << " OUT" << endl;
                        p->frame_number = 0;

                }
                else if(p->modified && p->filemap)
                {
                    proc_list[reverse_map_procid]->fouts++;
                        p->modified = false;
                        total_cost2 += 2400;
                        if (O_option)
                            cout << " FOUT" << endl;
                        p->frame_number =0;
                }
       
            }


}
void printConstants(int c){
    switch(c)
    {
        case 1:
                cout << " SEGV" << endl;
                break;
        case 2:
                cout << " IN" << endl;
                break;
        case 3:
                cout << " ZERO" << endl;
                break;
        case 4:
                cout << " FIN" << endl;
                break;
        case 5:
                cout << " SEGPROT" << endl;
                break;
        case 6:
                cout << " MAP ";
                break;
        case 7:
                cout << " UNMAP ";
                break;

        default:
                break;
    }

}
void interpretReadWriteExitContextSwitchIns(vector<Instruction> &instr_li)
{
    switch (algChoice) //creating correct pager class
    {
    case 0:
        pager = new FIFO();
        //cout << " \n created";
        break;
    case 1:
        pager = new Random();
        break;
    case 2:
        pager = new Clock();
        break;
    case 3:
        pager = new NRU();
        break;
    case 4:
        pager = new Aging();
        break;
    case 5:
        pager = new Working_Set();
        break;
    default:
        break;
    }

    //simulation
    //for (int i = 0; i < instructionList.size(); i++)
    for (int i = 0; i < instr_li.size(); i++)
    {

        char current_instr = instr_li[i].command;
        int curr_vPageNo = instr_li[i].page;
        // cout<<"curr ins"<< current_instr<<endl;
        // cout<<"curr addr"<<curr_vPageNo<<endl;
        Instruction currentIns = instructionList.at(i);
        ins_counter++;
        if (O_option)
        {
            cout << i << ": ==> " << current_instr<< " " << curr_vPageNo << endl;
        }
        //handle e an c. e left todo

        //if (currentIns.command == 'c')
        if (current_instr == 'c')
        {
            //current_proc_number = curr_vPageNo;
            //current_process = &processes[current_proc_number];
            current_process = proc_list[curr_vPageNo];
            //cout<<current_process->pid;
            total_context_switches++;
            total_cost2 += 130;
            continue;
        }

      
        //handle r and w
     if (current_instr == 'r' || current_instr == 'w')
     {
         int cur_vpage = curr_vPageNo;
        PTE *pte = &current_process->pageTable[cur_vpage];
      

        total_cost2++;

        if (!pte->present) //if faulting
        {
            bool SEGV_Error = true;
           

            if (segmentationFaultHandle(pte, current_process, cur_vpage, SEGV_Error))
            {
                //segmentation fault

                if (O_option)
                {
                   // cout << " SEGV" << endl;
                    choose_printing = 1;
                    printConstants(choose_printing);
                    total_cost2 += 340;
                }
                current_process->segv++;
                //moveon
                continue;
            }
           

            FTE *newframe = pager->get_frame();
            victimFrameSelect(newframe);
            
            //bot bak

            if(pte->filemap==0)
            {
                if (pte->pagedout)
                {
                    total_cost2 += 3100;
                    current_process->pageins++;
                    if (O_option)
                        //cout << " IN" << endl;
                        choose_printing = 2;
                        printConstants(choose_printing);
                    
                }
                else
                {
                    if (O_option)
                        //cout << " ZERO" << endl;
                        choose_printing = 3;
                        printConstants(choose_printing);
                    current_process->zeros++;
                    total_cost2 += 140;
                }
            }
            else if(pte->filemap==1)
            {
                if (O_option)
                    //cout << " FIN" << endl;
                    choose_printing = 4;
                    printConstants(choose_printing);
                current_process->fins++;
                total_cost2 += 2800;
            }
            

            // neframesetter func
            // newframe->procID = current_process->pid;
            // newframe->vpage = cur_vpage;
            // newframe->age = 0;
            // newframe->tau = ins_counter-1;
            newframe->setFrame(cur_vpage,current_process->pid,0,ins_counter-1);
            if (O_option)
               // cout << " MAP " << newframe->frameNum << endl;
                choose_printing = 6;
                printConstants(choose_printing);
                cout<<newframe->frameNum << endl;

            pte->present = 1;
            pte->frame_number = newframe->frameNum;
            total_cost2 += 300;
            current_process->maps++;
        }
         if (pte->present)
         {
              //modified if wr
            if (current_instr == 'r' )
            {
                //rwaccesses++;
                pte->referenced = 1;
                
            }

            if (current_instr == 'w')
            {
                if(pte->write_protect ==0)
                {
                    pte->modified = 1;
                     pte->referenced = 1;
                }
                else
                {
                    current_process->segprot++;
                    total_cost2 += 420;
                    pte->modified = 0;
                     pte->referenced = 1;
                    if (O_option)
                        //cout << " SEGPROT" << endl;
                        choose_printing = 5;
                        printConstants(choose_printing);
                    
                }
               
            }
         }
     }
     else
     {
         PTE *x;
            cout << "EXIT current process " << curr_vPageNo << endl;
          //  for (int i = 0; i < processes[currentIns.page].pageTable.size(); i++)
            int a = curr_vPageNo;
            for (int i = 0; i < proc_list[a]->pageTable.size(); i++)
            {
               // x = &processes[currentIns.page].pageTable[i];
                x = &proc_list[curr_vPageNo]->pageTable[i];
                if (x->present)
                {
                    FTE *f = &frameTable[x->frame_number];
                    //cout << " UNMAP " << f->procID << ":" << f->vpage << endl;
                    choose_printing = 7;
                    printConstants(choose_printing);
                    cout<<f->procID << ":" << f->vpage << endl;
                    setFTEFree(&frameTable[x->frame_number]);
                    
                    total_cost2 += 400;
               
                   proc_list[curr_vPageNo]->unmaps++;

                    if (x->modified && x->filemap)
                    {
                        
                            total_cost2 += 2400; // call 2400 const
                            //processes[currentIns.page].fouts++;
                            proc_list[curr_vPageNo]->fouts++; // call fout add func
                            cout << " FOUT" << endl;
                        
                    }
                    // if (x->modified)
                    // {
                    //     if (x->filemap)
                    //     {
                    //         total_cost2 += 2400; // call 2400 const
                    //         //processes[currentIns.page].fouts++;
                    //         proc_list[curr_vPageNo]->fouts++; // call fout add func
                    //         cout << " FOUT" << endl;
                    //     }
                    // }
                }
                setPTEBits( &proc_list[curr_vPageNo]->pageTable[i]);
                // x->present = 0;
                // x->referenced = 0;
                // x->pagedout = 0;
            }

            total_proc_exits++; //// call proc exits add func
            total_cost2 += 1250; // 2400 const call

            continue;
     }
        
        // //modified if wr
        // if (current_instr == 'r' || current_instr == 'w')
        // {
        //     //rwaccesses++;
        //     pte->referenced = 1;
            
        // }

        // if (current_instr == 'w')
        // {
        //     if (pte->write_protect)
        //     {
        //         if (O_option)
        //             cout << " SEGPROT" << endl;
        //         current_process->segprot++;
        //         total_cost2 += 420;
        //     }
        //     else
        //     {
        //         pte->modified = 1;
        //     }
        // }
    }
}

// void costCalc()
// {

//     //adding one by one for correct calc
//     for (int i = 0; i < processes.size(); i++)
//     {
//         Process *proc = &processes[i];
//         //adding maps
//         for (int i = 0; i < proc->maps; i++)
//         {
//             total_cost += (unsigned long long)400;
//         }
//         //adding unmaps
//         for (int i = 0; i < proc->unmaps; i++)
//         {
//             total_cost += (unsigned long long)400;
//         }
//         //adding pageins
//         for (int i = 0; i < proc->pageins; i++)
//         {
//             total_cost += (unsigned long long)3000;
//         }
//         //pageouts
//         for (int i = 0; i < proc->pageouts; i++)
//         {
//             total_cost += (unsigned long long)3000;
//         }
//         //fins
//         for (int i = 0; i < proc->fins; i++)
//         {
//             total_cost += (unsigned long long)2500;
//         }
//         //fouts
//         for (int i = 0; i < proc->fouts; i++)
//         {
//             total_cost += (unsigned long long)2500;
//         }
//         //zero
//         for (int i = 0; i < proc->zeros; i++)
//         {
//             total_cost += (unsigned long long)150;
//         }
//         for (int i = 0; i < proc->segv; i++)
//         {
//             total_cost += (unsigned long long)240;
//         }
//         for (int i = 0; i < proc->segprot; i++)
//         {
//             total_cost += (unsigned long long)300;
//         }
//     }

//     total_cost += (unsigned long long)rwaccesses;

//     for (int i = 0; i < total_context_switches; i++)
//         total_cost += (unsigned long long)121;
//     for (int i = 0; i < total_proc_exits; i++)
//         total_cost += (unsigned long long)121;
// }
void read_randomfile(string randomfilename)
{
    ifstream in2(randomfilename);
    string s;
    if (!in2)
    {
        cout << "cannot open rand file";
        exit(0);
    }
    else
    {
        getline(in2, s);
        rand_nums = atoi(&s[0]);
        while (getline(in2, s))
        {
            randoms.push_back(atoi(&s[0]));
        }
    }
    // rand_file.open(randomfilename.c_str());
    // string line;

    // getline(rand_file, line);
    // rand_num = stoi(line);
    // for(int i = 1; i <= rand_num; i++){
    // getline(rand_file, line);
    // int rand = stoi(line);
    // random_nums.push_back(rand);
  
}
void readInstructionsFile(string filename)
{
  
    string s;
    int num_proc;
    int num_vmas;
    int start_page;
    int end_page;
    int write_protected;
    int file_mapped;
   // ifstream in(filename);
    ifstream input;
    input.open(filename.c_str());
    if (!input)
    {
        cout << "Cannot open input file.\n";
        exit(0);
    }
    else
    {
        getline(input, s);
        while (s[0] == '#')
            getline(input, s);
        num_proc = atoi(&s[0]);
       // cout<<"num proc "<<num_proc<<endl;
        getline(input, s);
        while (s[0] == '#')
            getline(input, s);

        for (int i = 0; i < num_proc; i++) //procRead
        {
            Process process;
            ptr = new Process();
            ptr->setProcess();
            ptr->pid = i;
            // process.setProcess();
            // process.pid = i;
            num_vmas = atoi(&s[0]);
            //cout<<"num of vmas "<<num_vmas<<endl;
            getline(input, s);
            for (int j = 0; j < num_vmas; j++)
            {
                char *pch;
                pch = strtok(&s[0], " ");
                start_page = atoi(pch);
                //cout<<"start_page "<<start_page<<endl;
                pch = strtok(NULL, " ");
                end_page = atoi(pch);
               // cout<<"end_page "<<end_page<<endl;
                pch = strtok(NULL, " ");
                write_protected = atoi(pch);
                //cout<<"write_protected "<<write_protected<<endl;
                pch = strtok(NULL, " ");
                file_mapped = atoi(pch);
                //cout<<"file_mapped "<<file_mapped<<endl;
              //  VirtualMemoryAddress vma(start_page, end_page, write_protected, file_mapped);
                VirtualMemoryAddress *vma;
                vma  = new VirtualMemoryAddress();
                vma->setVMA(start_page, end_page, write_protected, file_mapped);
                ptr->virtualMemoryAreasList.push_back(vma);
               // ptr.virtualMemoryAreasList->push_back(vma);
               // process.vmas.push_back(vma);
                
                getline(input, s);
                while (s[0] == '#')
                    getline(input, s);
            }
            proc_list.push_back(ptr);
           //processes.push_back(process);
        }

        if (s[0] == '#')
            getline(input, s);
        else
            while (s[0] != '#' && input) // read c,r,w instructions
            {
               // inst ins;
               Instruction instr;
                char *pch;
                pch = strtok(&s[0], " ");
              //  cout<<"instr :"<< pch<<endl;
                instr.command = *pch;
                pch = strtok(NULL, " ");
               // cout<<"instr addr:"<< pch<<endl;
                instr.page= atoi(pch);
                instructionList.push_back(instr);
                getline(input, s);
            }
    }
    input.close();

}


int main(int argc, char **argv)
{
    int c;
    char *argPtr;
    while ((c = getopt(argc, argv, "f:a:o:")) != -1)
    {
        switch (c)
        {
        case 'f':
            num_frames = atoi(optarg);
           
            break;
        case 'a':
            switch (optarg[0])
            {
            case 'f':
                algChoice = 0; //fifo
                break;
            case 'r':
                algChoice = 1; //random
                break;
            case 'c':
                algChoice = 2; //clock
                break;
            case 'e':
                algChoice = 3; //enhanced Second Chance NRU
                break;
            case 'a':
                algChoice = 4; //aging
                break;
            case 'w':
                algChoice = 5; //working set
                break;
            default:
                break;
            }
            break;
        case 'o':
            argPtr = optarg;
            while (*argPtr != '\0')
            {
                switch (*argPtr)
                {
                case 'O':
                    O_option = true;
                    break;
                case 'P':
                    P_option = true;
                    break;
                case 'F':
                    F_option = true;
                    break;
                case 'S':
                    S_option = true;
                    break;
                case 'x':
                    x_option = true;
                    break;
                case 'y':
                    y_option = true;
                    break;
                case 'f':
                    f_option = true;
                    break;
                case 'a':
                    a_option = true;
                    break;
                default:
                    break;
                }
                argPtr++;
            }
            break;
        default:
            break;
        }
    }

    string inp_file = argv[optind];
    string ran_file = argv[optind + 1];

    readInstructionsFile(inp_file);
    read_randomfile(ran_file);
   // checker();
    //reader(inp_file, ran_file); //call to reader to read files and instruction
    pager->frameTableinit(); // init frame table
     //allocateFrameTable();
    interpretReadWriteExitContextSwitchIns(instructionList); //call to simulation to sim the required algorithm
    finalOutput(); //final outputs
    return 0;
}

void printSumm()
{
    for(Process *process: proc_list)
        {
            process->print();
        }
}
void finalOutput()
{
    if (P_option)
       // for (int i = 0; i < processes.size(); i++)
        for (int i = 0; i < proc_list.size(); i++)
        {
            printPageTable(i);
        }
    if (F_option)
        printFrameTable();
    if (S_option)
    {
        printSumm();

      
        printf("TOTALCOST %lu %llu %llu %llu %lu\n", instructionList.size(), total_context_switches, total_proc_exits, total_cost2, sizeof(PTE));
    }
}
