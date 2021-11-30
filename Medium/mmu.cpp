#include <iostream>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <fstream>
#include <stdio.h>
#include <getopt.h>
#include <bits/stdc++.h>

using namespace std;

bool summaryOpt;
bool frameTableOpt;
bool pageTableOpt;
bool Oopt;
const int page_table_numbr = 64;
class PTE
{
    public:
        unsigned PRESENT:1;
        unsigned WRITE_PROTECT:1;
        unsigned MODIFIED:1;
        unsigned REFERENCED:1;
        unsigned PAGEDOUT:1;
        unsigned frameNumber:7;
        unsigned fileMapped:1;
        unsigned vPageNo:6;
        unsigned vmaSet:1;
        unsigned vma:12;

        PTE()
        {
            PRESENT = 0;
            WRITE_PROTECT  = 0;
            REFERENCED = 0;
            MODIFIED = 0;
            PAGEDOUT = 0;
            frameNumber = 0;
            fileMapped = 0;
            vmaSet = 0;
            vma = 0;
        }

};
class VirtualMemoryAddress
{
    public:
        int starting_virtual_page;
        int ending_virtual_page;
        int write_protected;
        int file_mapped;

        VirtualMemoryAddress(int svp, int evp, int wp, int fm)
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
            int pid;
            vector<VirtualMemoryAddress> vmas;
            PTE pageTable[page_table_numbr];
            unsigned long maps;
            unsigned long unmaps;
            unsigned long ins;
            unsigned long outs;
            unsigned long fins;
            unsigned long fouts;
            unsigned long zeros;
            unsigned long segv;
            unsigned long segprot;
           
            void setProcess()
            {
                pid =0;
                int i;
                for(i=0; i<page_table_numbr; i++)
                {
                    pageTable[i] = *(new PTE);
                    pageTable[i].vPageNo = i;
                }

                unmaps = 0;
                maps = 0;
                ins = 0;
                outs = 0;
                fins = 0;
                fouts = 0;
                zeros = 0;
                segv = 0;
                segprot = 0;
            }
};
vector<Process> processes;
class Frame
{
     public:
        Process *process;
        PTE *vpage;
        int frameNo;
        uint32_t age;
        int tau;
    
    public:
        Frame()
        {
            process = NULL;
            vpage = NULL;
            age = 0;
            tau = 0;
        }

        void setFrame(Process *process, PTE *vpage)
        {
            this->process = process;
            this->vpage = vpage;
        }
};
Frame * FrameTable;
queue<Frame *> FreeList;
int MAX_FRAMES;

class Instruction
{
    public:
        char command;
        int page;
};

vector<Instruction> instructionList;
class Pager
{
    public:
        virtual Frame* select_victim_frame() = 0;
};
Pager *pager;

// First in First out
class FIFO: public Pager
{
    int hand;
    public:
        FIFO()
        {
            hand = 0;
        }

        Frame* select_victim_frame()
        {
            Frame *f = &FrameTable[hand];
            hand = (hand + 1) % MAX_FRAMES;
            return f;
        }
};
void readInstructionsFile(string filename)
{
     cout<<"inside instruc file"<<endl;
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
        cout<<"num proc "<<num_proc<<endl;
        getline(input, s);
        while (s[0] == '#')
            getline(input, s);

        for (int i = 0; i < num_proc; i++) //procRead
        {
            Process process;
            process.setProcess();
            process.pid = i;
            num_vmas = atoi(&s[0]);
            cout<<"num of vmas "<<num_vmas<<endl;
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
                VirtualMemoryAddress vma(start_page, end_page, write_protected, file_mapped);
                process.vmas.push_back(vma);
                
                getline(input, s);
                while (s[0] == '#')
                    getline(input, s);
            }

           processes.push_back(process);
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
               // cout<<"instr :"<< pch<<endl;
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
// void allocateFrameTable();
// void printFrameTable();

int main(int argc, char* argv[])
{
    int option;
    while((option = getopt (argc,argv,"f:a:o:")) != -1)
    {
        switch(option)
        {
            case 'o':   if(optarg!=NULL)
                        {
                            string s = optarg;
                            for(int i=0; i<s.size();i++)
                            {
                                switch(s[i])
                                {
                                    case 'F':   frameTableOpt = true;
                                                break;
                                    case 'P':   pageTableOpt = true;
                                                break;
                                    case 'O':   Oopt = true;
                                                break;
                                    case 'S':   summaryOpt = true;
                                                break;
                                }
                            }
                        }
                        break;

            case 'f':   MAX_FRAMES = stoi(optarg);
                        break;

            case 'a':   if(optarg!=NULL)
                        {
                            switch(optarg[0])
                            {
                                case 'f':   pager = new FIFO();
                                            break;
                                // case 'r':   pager = new Random();
                                //             break;
                                // case 'c':   pager = new Clock();
                                //             break;
                                // case 'e':   pager = new NRU();
                                //             break;
                                // case 'a':   pager = new Aging();
                                //             break;
                                // case 'w':   pager = new WorkingSet();
                                //             break;
                            }
                        }  

                        break;

            case '?':   if(optopt == 'o')
                            fprintf (stderr, "Option -%c requires an argument.\n", optopt);
                        break;
            default:    cout << "nothing" << endl;
                        break;
        }
    }
 
    
    string filename =  string(argv[argc-2]);
    //cout<<"filename  "<<filename<<endl;
    cout<<"read instruc file"<<endl;
    readInstructionsFile(filename);
    cout<<"total no of instructions"<<instructionList.size()<<endl;
    string randfile =  string(argv[argc-1]); 
    //cout<<"randfile  "<<randfile<<endl;

   cout<<"no of processes"<<processes.size()<<endl;

    // allocateFrameTable();
    // printFrameTable();
    
   for (int i = 0; i < processes.size(); i++)
    {
        Process x = processes[i];
        cout <<"process id :"<< x.pid << " " << endl;

        for (int j = 0; j < x.vmas.size(); j++)
        {
            cout << x.vmas[j].starting_virtual_page << " " << x.vmas[j].ending_virtual_page<< " " << x.vmas[j].write_protected << " " << x.vmas[j].file_mapped << endl;
        }

        // for(unsigned int a =0; a < sizeof(x.pageTable); a++)
        // {
        //     cout<<"frame no :"<< x.pageTable[a].frameNumber<<endl;
        // }
        // for (int k = 0; k < x.; k++)
        // {
        //    // cout << x.pageTable[k].frame_number << " : " << x.pageTable[k].hole << endl;
        // }

        cout << endl;
        cout << endl;
    }
    
    
    
}
