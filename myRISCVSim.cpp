
/*

The project is developed as part of Computer Architecture class
Project Name: Functional Simulator for subset of RISCV Processor

Developer's Name:SUSHIL KUMAR
Developer's Email id:2021csb1136@iitrpr.ac.in
Date:

*/

/* myRISCVSim.cpp
   Purpose of this file: implementation file for myRISCVSim
*/

#include <iostream>
#include <sstream>
#include <string>
#include<bits/stdc++.h>
using namespace std;

// current instruction
int PC;

// Register file
static unsigned int X[32];
// flags
// memory
static unsigned char MEM[4000];

// intermediate datapath and control path signals
static unsigned int instruction_word;
static unsigned int operand1;
static unsigned int operand2;

//current instruction
bitset<32> Inst;//int i = stoul(p, nullptr, 16);

bitset<7> opcode,funct7;//opcode

bitset<5> rd,rs1,rs2;

bitset<3> funct3;
bitset<12> immI,immS;
bitset<32> immU;
bitset<13> immB;
bitset<21> immJ;

// it is used to set the reset values
// reset all registers and memory content to 0
void reset_proc()
{
}

// reads from the instruction memory and updates the instruction register
void fetch(){
    string x,s;
    s="0x"+(to_string(PC));
    fstream FileName;                   
    FileName.open("input.mc", ios::in);         
    if(!FileName){                        
        cout<<"File doesn’t exist.";          
    }else{
        while (1) {         
            FileName>>x;              
            if(FileName.eof())          
                break;
            if(x==s){
              FileName>>x;
              x.erase(x.begin(),x.begin()+2);
              unsigned int num =  stoul(x, nullptr, 16); 
              Inst=num; 
              break;    
            }              
        }
    }
    FileName.close(); 
        PC=PC+4;
}
// reads the instruction register, reads operand1, operand2 fromo register file, decides the operation to be performed in execute stage
void decode()
{
  for (int i = 0; i < 7; i++)//opcode
  {
    opcode[i]=Inst[i];
  }
  for (int i = 0; i < 5; i++)//rd
  {
    rd[i]=Inst[i+7];
  }
  for (int i = 0; i < 3; i++)//funct3
  {
    funct3[i]=Inst[i+12];
  }
  for (int i = 0; i < 5; i++)//rs1
  {
    rs1[i]=Inst[i+15];
  }
  for (int i = 0; i < 5; i++)//rs2
  {
    rs2[i]=Inst[i+20];
  }
  for (int i = 0; i < 7; i++)//funct7
  {
    funct7[i]=Inst[i+25];
  }

  for (int i = 0; i < 12; i++)//immidaite of type I
  {
    immI[i]=Inst[i+20];
  }               //immidaite of type I

  for (int i = 0; i < 5; i++)//immidiate of type S
  {
    immS[i]=Inst[i+7];
  }
  for (int i = 0; i < 7; i++)
  {
    immS[i+5]=Inst[i+25];
  }         //immidiate of type S
  
  immB[11]=Inst[7];     //immidiate of B type
  for (int i = 0; i < 4; i++)
  {
    immB[i+1]=Inst[i+8];
  }
  for (int i = 0; i < 6; i++)
  {
    immB[i+5]=Inst[i+25];
  }
  immB[12]=Inst[31];  //immidiate of B type
  
  for (int i = 0; i < 20; i++) //immidiate of U type
  {
    immU[i+12]=Inst[i+12];
  }//immidiate of U type

  for (int i = 0; i < 8; i++) //immidiate of J type
  {
    immJ[i+12]=Inst[i+12];
  }
  immJ[11]=Inst[20];
  for (int i = 0; i < 10; i++)
  {
    immJ[i+1]=Inst[i+21];
  }
  immJ[20]=Inst[31];  //immidiate of J type
}

// executes the ALU operation based on ALUop
void execute()
{
}
// perform the memory operation
void mem()
{
}
// writes the results back to register file
void write_back()
{
}

void run_riscvsim()
{
  while (1)
  {
    fetch();
    decode();
    execute();
    mem();
    write_back();
  }
}

int main()
{
}
