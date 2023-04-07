
/*

The project is developed as part of Computer Architecture class
Project Name: Functional Simulator for subset of RISCV Processor

Developer's Name:SUSHIL KUMAR
Developer's Entry number:2021CSB1136
Developer's Name:GYANENDRA MANI
Developer's Entry number:2021csb1090
Developer's Name:RAKESH
Developer's Entry number:2021CSB1126
Date:13-3-2023

*/

/* myRISCVSim.cpp
   Purpose of this file: implementation file for myRISCVSim
*/
#include<bitset>
#include <iostream>
#include <sstream>
#include <string>
#include<fstream>

using namespace std;

// current instruction
int PC = 0, loop = 1;

//stages operator
int IF,DE,EX,MA,WB;

// result after execution
int resultALU, resultMEM;

// Register file
static int X[32];

// memory
static unsigned char MEM[10000];

// current instruction
bitset<32> Inst,ImmI,ImmS,ImmB,ImmJ,ImmU; // int i = stoul(p, nullptr, 16);

bitset<7> opcode, funct7; // opcode

bitset<5> rd, rs1, rs2;

bitset<3> funct3;

int mtype;

int Op1,Op2;
int Op1_RFread,Op2_RFread;

//control signals
int BranchTargetSelect,ResultSelect,RFWrite,ALUOperation,OP2Select,MemOp;
int Isbranch,branchOperation,branchAdd;

//decode stage
int dp_BranchTargetSelect,dp_ResultSelect,dp_RFWrite,dp_ALUOperation,dp_OP2Select;
int dp_MemOp,dp_Isbranch,dp_branchOperation;
int dp_ImmI,dp_ImmS,dp_ImmB,dp_ImmJ,dp_ImmU;
int dp_PC;

//execute stage
int ep_BranchTargetSelect,ep_ResultSelect,ep_RFWrite,ep_ALUOperation,ep_OP2Select;
int ep_MemOp,ep_Isbranch,ep_branchOperation,dp_resultALU,ep_ImmU,mp_rd;
int ep_ImmI,ep_ImmS,ep_ImmB,ep_ImmJ,ep_ImmU;
int ep_PC;

//memory stage
int mp_BranchTargetSelect,mp_ResultSelect,mp_RFWrite,mp_ALUOperation,mp_OP2Select;
int mp_MemOp,mp_Isbranch,mp_branchOperation,mp_resultALU,mp_ImmU,mp_rd;
int mp_ImmI,mp_ImmS,mp_ImmB,mp_ImmJ,mp_ImmU;
int mp_PC;

// it is used to set the reset values
// reset all registers and memory content to 0
int shiftRL(int a,int b)
{
  bitset<32> t;
  t=a;
  if(b>=0)
  {
    for (int i = 0; i < 32-b; i++)
    {
      t[i]=t[i+b];
    }
    for (int i = 0; i < b; i++)
    {
      t[32-b+i]=0;
    }
  }
  return t.to_ulong();
}

// reads from the instruction memory and updates the instruction register
void fetch()
{
  string x, s;
  stringstream ss;
  ss << hex << PC;
  s = "0x" + ss.str();
  ifstream FileName;
  FileName.open("input.mc", ios::in);
  if (!FileName)
  {
    cout << "File doesn’t exist.";
  }
  else
  {
    while (1)
    {
      FileName >> x;
      if (FileName.eof())
      {
        loop = 0;
        break;
      }
      if (x == s)
      {
        FileName >> x;
        x.erase(x.begin(), x.begin() + 2);
        unsigned int num = stoul(x, nullptr, 16);
        Inst = num;
        break;
      }
    }
  }
  FileName.close();
  if (ep_Isbranch==0)
  {
    PC=PC+4;
  }
  else if(ep_Isbranch==1)
  {
    PC=branchAdd;
  }
  else
  {
    PC=resultALU;
  }

}

// reads the instruction register, reads operand1, operand2 from register file, decides the operation to be performed in execute stage
void decode()
{
  Isbranch=0;
  MemOp=0;
  OP2Select=0;
  mtype=0;

  for (int i = 0; i < 7; i++) // opcode
  {
    opcode[i]=Inst[i];
  }
  for (int i = 7; i < 12; i++) //rd
  {
    rd[i]=Inst[i];
  }
  for (int i = 12; i < 14; i++) //funct3
  {
    funct3[i]=Inst[i];
  }
  for (int i = 15; i < 20; i++) //rs1
  {
    rs1[i]=Inst[i];
  }
  for (int i = 20; i < 25; i++) //rs2
  {
    rs2[i]=Inst[i];
  }
  for (int i = 25; i < 32; i++)  //funct7
  {
    funct7[i]=Inst[i];
  }

  Op1_RFread=X[rs1.to_ulong()];
  MemOp=funct3.to_ulong();

  //for immidiate and control signals
  switch (opcode.to_ulong())
  {
  case 19://type I
    OP2Select=1;

    if (funct3.to_ulong()==1 || funct3.to_ulong()==5) //immidiate
    {
      for (int i = 0; i < 5; i++)
      {
        ImmI[i]=Inst[i+20];
      }
      for (int i = 5; i < 32; i++)
      {
        ImmI[i]=ImmI[4];
      }
    }
    else
    {
      for (int i = 0; i < 12; i++)
      {
        ImmI[i]=Inst[i+20];
      }
      for (int i = 12; i < 32; i++)
      {
        ImmI[i]=ImmI[11];
      }
    }

    break;
  case 3://load immidiate
    ALUOperation=0;
    MemOp=1;
    OP2Select=1;
    mtype=1;
    dp_RFWrite=1;
    ResultSelect = 2;
    
    for (int i = 0; i < 12; i++)
    {
      ImmI[i]=Inst[i+20];
    }
    for (int i = 12; i < 32; i++)
    {
      ImmI[i]=ImmI[11];
    }
    break;
  case 103://jalr immidiate
    ALUOperation=0;
    OP2Select=1;
    Isbranch=2;
    branchOperation=2;
    dp_RFWrite=1;
    
    for (int i = 0; i < 12; i++)
    {
      ImmI[i]=Inst[i+20];
    }
    for (int i = 12; i < 32; i++)
    {
      ImmI[i]=ImmI[11];
    }
    break;
  case 35://type S
    ALUOperation=0;
    MemOp=2;
    OP2Select=2;
    mtype=2;
    dp_RFWrite=0;

    for (int i = 0; i < 5; i++)//immidiate
    {
      ImmS[i]=Inst[i+7];
    }
    for (int i = 0; i < 7; i++)
    {
      ImmS[i+5]=Inst[i+25];
    }
    for (int i = 12; i < 32; i++)
    {
      ImmS[i]=ImmS[11];
    }   
    break;
  case 99://type B immidiate
    Isbranch=1;
    BranchTargetSelect=1;
    dp_RFWrite=0;

    ImmB[0]=0;
    ImmB[11]=Inst[7];
    for (int i = 0; i < 4; i++)
    {
      ImmB[i+1]=Inst[i+8];
    }
    for (int i = 0; i < 6; i++)
    {
      ImmB[i+5]=Inst[i+25];
    }
    ImmB[12]=Inst[31];
    for (int i = 13; i < 32; i++)
    {
      ImmB[i]=ImmB[12];
    }
    break;
  case 55://lui immidiate
    OP2Select=3;
    dp_RFWrite=1;

    for (int i = 0; i < 12; i++)
    {
      ImmU[i]=0;
    }
    for (int i = 0; i < 20; i++)
    {
      ImmU[i+12]=Inst[i+12];
    }    
    break;
  case 23://auipc immididate
    OP2Select=3;
    dp_RFWrite=1;

    for (int i = 0; i < 12; i++)
    {
      ImmU[i]=0;
    }
    for (int i = 0; i < 20; i++)
    {
      ImmU[i+12]=Inst[i+12];
    }
    break;
  case 111:// jal immidiate
    Isbranch=1;
    BranchTargetSelect=0;
    dp_RFWrite=1;

    ImmJ[0]=0;
    for (int i = 0; i < 8; i++)
    {
      ImmJ[i+12]=Inst[i+12];
    }
    ImmJ[11]=Inst[20];
    for (int i = 0; i < 10; i++)
    {
      ImmJ[i+1]=Inst[i+21];
    }
    ImmJ[20]=Inst[31];
    for (int i = 21; i < 32; i++)
    {
      ImmJ[i]=ImmJ[20];
    }
    break;
  default:
    break;
  }

  if (opcode.to_ulong()==51 || opcode.to_ulong()==19)
  {
    dp_RFWrite=1;
    
    switch (funct3.to_ulong())
    {
    case 0:
      if (funct7.to_ulong()==0 || opcode.to_ulong()==19)
      {
        ALUOperation=0;//add
      }
      else
      {
        ALUOperation=1;//sub
      }      
      break;
    case 4:
      ALUOperation=2;//xor
      break;
    case 6:
      ALUOperation=3;//or
      break;
    case 7://and
      ALUOperation=4;
      break;
    case 1://sll
      ALUOperation=5;
      break;
    case 5:
      if (funct7.to_ulong()==0)
      {
        ALUOperation=6;//srl
      }
      else
      {
        ALUOperation=7;//sra
      }      
      break;
    case 2://slt
      ALUOperation=8;
      break;
    default:
      break;
    }
  }
  Op2_RFread=X[rs2.to_ulong()];

}

// executes the ALU operation based on ALUop
void execute()
{

  if (dp_OP2Select==0)
  {
    Op2=Op2_RFread;
  }
  else if (OP2Select==1)
  {
    Op2=ImmI.to_ulong();
  }
  else if (OP2Select==2)
  {
    Op2=ImmS.to_ulong();
  }


  switch (dp_ALUOperation)
  {
  case 0:
    resultALU = Op1 + Op2;
    break;
  case 1:
    resultALU = Op1 - Op2;
    break;
  case 2:
    resultALU = Op1 ^ Op2;
    break;
  case 3:
    resultALU = Op1 | Op2;
    break;
  case 4:
    resultALU = Op1 & Op2;
    break;
  case 5:
    resultALU = Op1 << Op2;
    break;
  case 6:
    resultALU = shiftRL(Op1,Op2);
    break;
  case 7:
    resultALU = Op1 >> Op2;
    break;
  case 8:
    resultALU = (Op1 < Op2 ? 1:0);
    break;
  
  default:
    break;
  }

  switch (BranchTargetSelect)
  {
  case 0:
    branchAdd = dp_PC + dp_ImmJ;
    break;
  case 1:
    branchAdd = dp_PC + dp_ImmB;
    break;
  
  default:
    break;
  }

}
// perform the memory operation
void mem()
{
   bitset<32>b1=resultALU;
   bitset<16>b2(b1.to_string(), 16, 31);
   bitset<8>b3(b1.to_string(), 24, 31);
   if(MemOp==1){
      switch(mtype){
         case 0:
            resultMEM=MEM[(int8_t)b3.to_ulong()];
            break;
         case 1:
             resultMEM=MEM[(int16_t)b2.to_ulong()];
            break;
         case 2:
             resultMEM=MEM[(int32_t)b1.to_ulong()];
            break;
         default:
            break;
      }
   }else if(MemOp==2){
      switch(){
           switch(mtype){
         case 0:
           MEM[(int8_t)b3.to_ulong()]=Op2_RFread;
            break;
         case 1:
            MEM[(int816_t)b2.to_ulong()]=Op2_RFread;
            break;
         case 2:
             MEM[(int32_t)b1.to_ulong()]=Op2_RFread;
            break;
         default:
            break;
      } 
      }
   }
}
// writes the results back to register file
void write_back()
{
  
  switch (ResultSelect)
  {
  
   case 1:
        X[mp_rd]=PC+4;
        break;
   case 2:
        X[mp_rd]=resultMEM;
       break; 
   case 3:
        X[mp_rd]=resultALU;
       break;
   case 4:
        X[mp_rd]=mp_ImmU;
      break;
  default:
    break;
  }
}

int main()
{

  while (1)
  {
    fetch();
    if (!loop)
      break;
    decode();
    execute();
    mem();
    write_back();
  }
  ofstream outputFile("output.txt");
  outputFile <<"\n"<< "Values of resister's" <<"\n\n";
  for (int i = 0; i < 32; i++)
  {
    outputFile << "x" << i << " -> " << X[i] << endl;
  }
  outputFile <<"\n"<< "Values at the memory addresses" <<"\n\n";
  for (int i = 0; i < 10000; i++)
  {
    outputFile << i <<" -> "<< MEM[i] << endl;
  }
}
