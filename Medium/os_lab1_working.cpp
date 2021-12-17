#include<iostream>
#include<stdio.h>
#include<fstream>
#include<vector>
#include<iomanip>
#include<sstream> 
#include<string>
#include <algorithm>
#include <unordered_set>
#include<string.h>
#include <deque>
#include <list>
using namespace std;

int moduleSize[100];

// class Token 
// {
//     public:
//     int lineNumber;
//     int lineOffset;
//     string tokenContents;

//     void setToken(int lineNo, int lineOff, string tokenStr)
//     {
//         lineNumber = lineNo;
//         lineOffset = lineOff;
//         tokenContents = tokenStr;
//     }
// };
struct Token
{
int lineNumber;
    int lineOffset;
    string tokenContents;
};
class Symbol
{
    public:
    string sym;
    int Addr;
    int moduleNum;
    bool alreadyDefined = false;
    bool used = false;

};


void __parseerror(int errcode, Token tokenVar, bool flag) 
{
	string errstr[] = {
		"NUM_EXPECTED",
		"SYM_EXPECTED",
		"ADDR_EXPECTED",
		"SYM_TOO_LONG",
		"TOO_MANY_DEF_IN_MODULE",
		"TOO_MANY_USE_IN_MODULE",
		"TOO_MANY_INSTR",
	};
    if(flag)
    {
    cout<<"Parse Error line "<<tokenVar.lineOffset<<" offset "<<tokenVar.lineNumber<<": "<<errstr[errcode]<<endl; 
    }
    else{
        cout<<"Parse Error line "<<tokenVar.lineNumber<<" offset "<<tokenVar.lineOffset<<": "<<errstr[errcode]<<endl; 
    }
    
	exit(0);
}
Token getNextToken()
{}
int readInt(Token tokenVar) 
{   
    string s = tokenVar.tokenContents;
    int i;
	try
	{
		 i = std::stoi(s);
		
	}
	catch (std::invalid_argument const &e)
	{
		__parseerror(0, tokenVar, false);
	}
	catch (std::out_of_range const &e)
	{
		__parseerror(0, tokenVar, false);
	}

	return i;
}

string readSymbol(Token tokenVar)
{
    string s = tokenVar.tokenContents;
    int i = 0;
    if(strlen(&s[0])<1)
    {
       // cout<<"92"<<endl;
        __parseerror(1,tokenVar, true);
    }
    while(s[i])
    {
        if(i == 0)
        {
            if(!isalpha(s[i]))
            
            __parseerror(1,tokenVar, false);
            //cout<<"102"<<endl;
        }

        if(!isalnum(s[i]))
        {
            //cout<<"106"<<endl;
            __parseerror(1,tokenVar, false);
        }

        i++;
    }
    char* k = &s[0];
    if(strlen(k)>16)
    {
        __parseerror(3,tokenVar, false);
    }
    return s;

}

string readIAER(Token tokenVar)
{
    string s = tokenVar.tokenContents;
    
    if(s.length()>1)
    {   
       // cout<<"124"<<endl;
        __parseerror(2,tokenVar, false);
    }
    if(s[0]!='I'&&s[0]!='A'&&s[0]!='E'&&s[0]!='R')
    {   
        //cout<<"129"<<endl;
        __parseerror(2,tokenVar, true);
    }

    return s;
}

deque<Symbol> symbolQ;

void parse1(string fileName)
{
    
    ifstream file(fileName);
    string line;
    int temp;
    int lineNo = 1;
    char cArray[100];
    int totalInst = 0;

    //vector<Token> tokens; 

    list<Token> tokens;
    if(!file)
    {
        cout<<"Unable to open file "<<fileName<<endl;
        exit(0);
    }
    int ll;
    //deque<Symbol> symbol;
    
    while(getline(file,line))
    {   
        //getting tokens
        ll = strlen(&line[0]);
        int lineOffset = 1;
        char* str = &line[0];
        char* tok;
        char* remainingLine;
        tok = strtok (str," \t");
        while (tok != NULL)
        {   
            //cout<<"Token is : "<<tok<<" found at positon  "<<lineOffset<<" in line "<<lineNo<<endl;
            int curTokLength = strlen(tok);
            //Token t;
            struct  Token t;
            t.lineNumber = lineNo;
            t.lineOffset = lineOffset;
            t.tokenContents = tok;
            
           // t.setToken(lineNo,lineOffset,tok);

            tokens.push_back(t);
            
            remainingLine = strtok (NULL,"");
            tok = strtok (remainingLine, " \t");
            if(tok!=NULL)
            {
                char * pch;
                pch = strstr (remainingLine,tok);
                lineOffset = lineOffset + curTokLength + strlen(remainingLine) - strlen(pch) + 1;
            }   
        }
        lineNo++;
    }
    Token last;
    if(lineNo == 1 && ll ==0) 
    {
        last.lineOffset = 1;
        last.lineNumber = 1;
        last.tokenContents = "";
        //last.setToken(1,1,"");
    }
    else 
    {
        last.lineOffset =lineNo-1;
        last.lineNumber = ll+1;
        last.tokenContents = "";
        //last.setToken(lineNo-1,ll+1,"");
    }
    tokens.push_back(last);

    int ct = 0;
    int moduleNo = 0; //current module number 
    int curBaseAddress = 0;
    
    //parsing 
    while(ct<tokens.size()-1)
    {   
        moduleNo++;
        
        Token curTok;
        int defcount;
        try
        {   
            list<Token>::iterator it = tokens.begin();
            advance(it,ct);

            curTok = *it;
            //curTok = tokens.at(ct);   
                  
            defcount = readInt(curTok);
            //cout<<curTok.tokenContents; 
            ct++;
        }
        catch(exception E)
        {
            __parseerror(0, last, false);
        }

        if(defcount > 16)
        {
            __parseerror(4,curTok, false);
        }    
        //creating symbols
        for(int p = 0;p<defcount;p++)
        {   
            Symbol s;
            
            
            try
            {
                Token sym;
                // = tokens.at(ct);
                list<Token>::iterator it = tokens.begin();
                advance(it,ct);
                sym = *it;
                string k = readSymbol(sym);
                
                s.sym =k;
                ct++;

            }
            catch(exception E)
            {
               // cout<<"266"<<endl;
                __parseerror(1, last, false);
            }

            try
            {
              // Token symAddr = tokens.at(ct);
              Token symAddr;
              list<Token>::iterator it = tokens.begin();
                advance(it,ct);
                symAddr = *it;
               int addr = readInt(symAddr)+curBaseAddress;
               s.Addr = addr;
               s.moduleNum = moduleNo;
               ct++;
            }
            catch(exception E)
            {
                __parseerror(0, last, false);   
            }
            int flag = 0;
            for(int d = 0;d<symbolQ.size();d++)
            {
                if(symbolQ[d].sym.compare(s.sym)==0)
                {
                    symbolQ[d].alreadyDefined = true;
                    flag = 1;
                }
            }
            if(flag == 0)
            {
                symbolQ.push_back(s);
            }
            
            
            
        }
        //creating symbols over
        
        int usecount;
        try
        {
            //curTok = tokens.at(ct);
            list<Token>::iterator it = tokens.begin();
            advance(it,ct);

            curTok = *it;
            usecount = readInt(curTok);
            ct++;
        }
        catch(exception E)
        {
            __parseerror(0, last, false);
        }

        if(usecount > 16)
        {
            __parseerror(5,curTok, false);
        }
        

        for(int p=0;p<usecount;p++)
        {
            try
            {
               // Token sym = tokens.at(ct);
                Token sym;
                // = tokens.at(ct);
                list<Token>::iterator it = tokens.begin();
                advance(it,ct);
                sym = *it;
                string k = readSymbol(sym);
                ct++;
            }
            catch(exception E)
            {
                //cout<<"342"<<endl;
                __parseerror(1, last, false);
            }

        }
        
        //usecount over

        int instcount;
        
        Token insTok;
        try
        {
            //insTok = tokens.at(ct);
           ///  Token sym;
                // = tokens.at(ct);
                list<Token>::iterator it = tokens.begin();
                advance(it,ct);
                insTok = *it;

            instcount = readInt(insTok);
            totalInst = totalInst+instcount;
            ct++;

        }
        
        catch(const std::exception& e)
        {
            __parseerror(0,last, false);
        }
        if(totalInst>512)
        {
            __parseerror(6,insTok, false);
        }
        for(int k=0;k<instcount;k++)
        {   
            string addressMode;
            int addr;
            try
            {
                Token aMode ;//= tokens.at(ct);
                list<Token>::iterator it = tokens.begin();
                advance(it,ct);
                aMode = *it;
                addressMode = readIAER(aMode); 
                ct++;   
            }
            catch(const std::exception& e)
            {
                //cout<<"386"<<endl;
                __parseerror(2,last, false);
            }
            
            try
            {
               // Token addrs = tokens.at(ct);
                Token addrs ;//= tokens.at(ct);
                list<Token>::iterator it = tokens.begin();
                advance(it,ct);
                addrs = *it;
                addr = readInt(addrs);

                ct++;
            }
            catch(const std::exception& e)
            {
                __parseerror(0,last, false);
            }
            
            
            //checks done here
        }
        curBaseAddress = curBaseAddress + instcount;
        moduleSize[moduleNo]=curBaseAddress;
        
    }

    file.close();
  //  return symbol;

    
}


void parse2(string fileName)
{
    
    ifstream file(fileName);
    string line;
    int temp;
    int lineNo = 1;
    char cArray[100];

    list<Token> tokens; 

    if(!file)
    {
        cout<<"Unable to open file "<<fileName<<endl;
        exit(0);
    }
    int ll;
    vector<Symbol> symbol;
    while(getline(file,line))
    {   
        //getting tokens
        ll = strlen(&line[0]);
        int lineOffset = 1;
        char* str = &line[0];
        char* tok;
        char* remainingLine;
        tok = strtok (str," \t");
        
        while (tok != NULL)
        {   
            //cout<<"Token is : "<<tok<<" found at positon  "<<lineOffset<<" in line "<<lineNo<<endl;
            int curTokLength = strlen(tok);
            struct  Token t;
            t.lineNumber = lineNo;
            t.lineOffset = lineOffset;
            t.tokenContents = tok;

            tokens.push_back(t);
            
            remainingLine = strtok (NULL,"");
            tok = strtok (remainingLine, " \t");
            if(tok!=NULL)
            {
                char * pch;
                pch = strstr (remainingLine,tok);
                lineOffset = lineOffset + curTokLength + strlen(remainingLine) - strlen(pch) + 1;
            }   
        }
        lineNo++;
    }
    Token last;
    if(lineNo == 1 && ll ==0) 
    {
        last.lineOffset = 1;
        last.lineNumber = 1;
        last.tokenContents = "";
        //last.setToken(1,1,"");
    }
    else 
    {
        last.lineOffset =lineNo-1;
        last.lineNumber = ll+1;
        last.tokenContents = "";
        //last.setToken(lineNo-1,ll+1,"");
    }
    tokens.push_back(last);


    int ct = 0;
    int moduleNo = 0; //current module number 
    int curBaseAddress = 0;
    int memoryMap = 0;
    //parsing 
    while(ct<tokens.size()-1)
    {   
        moduleNo++;
        Token curTok;
        int defcount;
        cout<<curTok.tokenContents;
        try
        {   
            list<Token>::iterator it = tokens.begin();
            advance(it,ct);

            curTok = *it;          
            defcount = readInt(curTok);
            ct++;
        }
        catch(exception E)
        {
            __parseerror(0, last, false);
        }
 
        ct = ct+2*defcount;
        
        int usecount;
        try
        {
           list<Token>::iterator it = tokens.begin();
            advance(it,ct);

            curTok = *it;
            usecount = readInt(curTok);
            ct++;
        }
        catch(exception E)
        {
            __parseerror(0, last, false);
        }
        vector<string> useCountSyms;
        vector<int> RefferedInE; 
        for(int p=0;p<usecount;p++)
        {
            try
            {
                 Token sym;
                // = tokens.at(ct);
                list<Token>::iterator it = tokens.begin();
                advance(it,ct);
                sym = *it;
                string k = readSymbol(sym);
                useCountSyms.push_back(k);
                for(int e =0;e<symbolQ.size();e++)
                {
                    if(symbolQ[e].sym.compare(k)==0)
                    {
                        symbolQ[e].used=true;
                    }
                }
                ct++;
            }
            catch(exception E)
            {
               // cout<<"559"<<endl;
                __parseerror(1, last, false);
            }

        }
        
        //usecount over

        int instcount;
        Token insTok;
        try
        {
            list<Token>::iterator it = tokens.begin();
                advance(it,ct);
                insTok = *it;
            instcount = readInt(insTok);
            ct++;
        }
        
        catch(const std::exception& e)
        {
            __parseerror(0,last, false);
        }
        if(instcount>512)
        {
            __parseerror(6,insTok, false);
        }
        int memory = 0;
        bool errorS = false;
        string errstr = "";
        for(int k=0;k<instcount;k++)
        {   
            string addressMode;
            int addr;
            try
            {
            Token aMode ;//= tokens.at(ct);
                list<Token>::iterator it = tokens.begin();
                advance(it,ct);
                aMode = *it;
                addressMode = readIAER(aMode); 
                ct++;   
            }
            catch(const std::exception& e)
            {
                //cout<<"598"<<endl;
                __parseerror(2,last, false);
            }
            
            try
            {
               Token addrs ;//= tokens.at(ct);
                list<Token>::iterator it = tokens.begin();
                advance(it,ct);
                addrs = *it;
                addr = readInt(addrs);
                //cout<<"addr"<<addr;
                ct++;
            }
            catch(const std::exception& e)
            {
                __parseerror(0,last, false);
            }
            bool opcodeErr = false;
            
            int opcode = addr/1000;

            
            int operand = addr%1000;
            int outputAddr = 0;
            if(addr>9999)
            {
                
                opcodeErr = true;
                errstr = "Error: Illegal opcode; treated as 9999";
                opcode = 9;
                operand = 999;
                outputAddr = 9999;
                
                
            }

            else if(addressMode[0]=='R')
            {
                opcode = addr/1000;
                operand = addr%1000;
                
                if(operand<instcount)
                    outputAddr = curBaseAddress + opcode*1000 + operand;
                else
                {
                    outputAddr = curBaseAddress + opcode*1000;
                    errstr = "Error: Relative address exceeds module size; zero used";
                    errorS = true;
                }
                

            }
            else;
            if (addressMode[0]=='I')
            {
                if(addr>9999)
                {
                    addr = 9999;
                    errstr = "Error: Illegal immediate value; treated as 9999";
                    errorS = true;
                }
                outputAddr = addr;
            }
            else if (addressMode[0]=='E')
            {
                string useSym;
                try
                {   
                    useSym = useCountSyms.at(operand);
                    RefferedInE.push_back(operand);
                    //cout<<useSym;
                    bool notfound = true;
                    for(int u = 0;u<symbolQ.size();u++)
                    {
                        Symbol S = symbolQ[u];
                        if(S.sym.compare(useSym)==0)
                        {
                            
                            int opc = opcode;
                            int operand = S.Addr;
                            outputAddr = (opc*1000)+operand;
                            notfound = false;
                            break;
                        } 


                    }
                        if(notfound)
                        {
                            errstr="Error: "+useSym+ " is not defined; zero used";
                            errorS = true;
                            outputAddr = opcode*1000+0;
                        }
                }
                catch(const std::exception& e)
                {
                    errstr = "Error: External address exceeds length of uselist; treated as immediate";
                    errorS = true;
                    outputAddr = addr;
                    
                }

            }
            else if (addressMode[0]=='A')
            {
                if(operand<512)
                {
                    outputAddr = addr;
                }
                else
                {
                    errstr = "Error: Absolute address exceeds machine size; zero used";
                    outputAddr = opcode*1000;
                    errorS = true;
                }
                
            }
            else;
            
            
            cout<<setfill('0')<<setw(3)<<memoryMap;
            memoryMap++;
            cout<<":"<<" "<<setfill('0')<<setw(4)<<outputAddr;
            if(opcodeErr)
            {
                cout<<" "<<errstr;
            }
            else if(errorS)
            {
                cout<<" "<<errstr;
            }
            
            cout<<endl;
            errstr = "";
            errorS = false;

            
            //checks done here
        }
        curBaseAddress = curBaseAddress + instcount;


        for(int g = 0;g<useCountSyms.size();g++)
        {
            bool found = false;
            for(int n=0;n<RefferedInE.size();n++)
            {
                if(g==RefferedInE[n])
                {
                    found = true;
                }
            }
            if(!found)
            cout<<"Warning: Module "<<moduleNo<<": "<<useCountSyms[g]<<" appeared in the uselist but was not actually used\n";
		    
        }




    }

    //return symTable;
    
}
int main(int argc, char** argv) 
{ 
    //check inputs format:
    if(argc!=2)
    {
        cout<<" Wrong Inputs \n";
        exit(0);
    }

    string fileName = argv[1];


	 parse1(fileName);

	  cout<<"Symbol Table"<<endl;

    for(int i =0;i<symbolQ.size();i++)
    {
        cout<<symbolQ[i].sym<<"="<<symbolQ[i].Addr;
        if(symbolQ[i].alreadyDefined)
        {
            cout<<" Error: This variable is multiple times defined; first value used";
        }
        cout<<endl;
    }


cout<<endl<<"Memory Map"<<endl;
    parse2(fileName);

     cout<<endl;

    for(int e =0;e<symbolQ.size();e++)
    {
        if(symbolQ[e].used==false)
        {
            cout<<"Warning: Module "<<symbolQ[e].moduleNum<<": "<<symbolQ[e].sym<<" was defined but never used"<<endl;
        }
    }    
    
    
    return 0;

}
