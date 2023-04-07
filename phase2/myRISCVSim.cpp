
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

int dp_BranchTargetSelect,dp_ResultSelect,dp_RFWrite,dp_ALUOperation,dp_OP2Select;
int dp_MemOp,dp_Isbranch,dp_branchOperation;
int dp_PC;

int ep_BranchTargetSelect,ep_ResultSelect,ep_RFWrite,ep_ALUOperation,ep_OP2Select;
int ep_MemOp,ep_Isbranch,ep_branchOperation;
int ep_PC;

int mp_BranchTargetSelect,mp_ResultSelect,mp_RFWrite,mp_ALUOperation,mp_OP2Select;
int mp_MemOp,mp_Isbranch,mp_branchOperation;
int mp_PC;

// it is used to set the reset values
// reset all registers and memory content to 0

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
  if (Isbranch==0)
  {
    PC=PC+4;
  }
  else if(Isbranch==1)
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
  if (OP2Select==0)
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
  else if (OP2Select==3)
  {
    Op2=ImmU.to_ulong();
  }

  switch (ALUOperation)
  {
  case 0:
    resultALU = Op1 + Op2;
    break;
  
  default:
    break;
  }

}
// perform the memory operation
void mem()
{
}
// writes the results back to register file
void write_back()
{
  PC = PC + 4;
  switch (opcode.to_ulong())
  {
  case 51: // R type
    X[rd.to_ulong()] = resultALU;
    break;
  case 19: // I type
    X[rd.to_ulong()] = resultALU;
    break;
  case 3: // Load type
    X[rd.to_ulong()] = resultMEM;
    break;
  case 99: // B type
    switch (funct3.to_ulong())
    {
    case 0: // beq
      if (X[rs1.to_ulong()] == X[rs2.to_ulong()])
      {
        PC = resultALU;
      }
      break;
    case 1: // bne
      if (X[rs1.to_ulong()] != X[rs2.to_ulong()])
      {
        PC = resultALU;
      }
      break;
    case 4: // blt
      if (X[rs1.to_ulong()] < X[rs2.to_ulong()])
      {
        PC = resultALU;
      }
      break;
    case 5: // bge
      if (X[rs1.to_ulong()] >= X[rs2.to_ulong()])
      {
        PC = resultALU;
      }
      break;
    default:
      break;
    }
    break;
  case 111: // jal
    X[rd.to_ulong()] = PC;
    PC = resultALU;
    break;
  case 103: // jalr
    X[rd.to_ulong()] = PC;
    PC = resultALU;
    break;
  case 55: // lui
    X[rd.to_ulong()] = resultALU;
    break;
  case 23: // auipc
    X[rd.to_ulong()] = resultALU;
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