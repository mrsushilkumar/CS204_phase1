
/*

The project is developed as part of Computer Architecture class
Project Name: Functional Simulator for subset of RISCV Processor

Developer's Name:SUSHIL KUMAR
Developer's Entry number:2021CSB1136
Developer's Name:GYANENDRA MANI
Developer's Entry number:2021csb1090
Developer's Name:RAKESH
Developer's Entry number:2021CSB1126
Date:

*/

/* myRISCVSim.cpp
   Purpose of this file: implementation file for myRISCVSim with pileline
*/

#include<bitset>
#include <iostream>
#include <sstream>
#include <string>
#include<fstream>
 
using namespace std;

//stalls and dependencies
int stall,depend;
//type of dependencies
bool data_H=false,control_H=false,flag_H=false;

int stay;
// current instruction
int PC = 0,cycles=0;
bool loop = false;

//stay
int stay_if,stay_de,stay_ex,stay_ma;

//stages operator
int IF=1,DE=0,EX=0,MA=0,WB=0;

// result after execution
int resultALU, resultMEM;

// Register file
static int X[32];
// memory
static unsigned char MEM[32000];

// current instruction
bitset<32> Inst,ImmI,ImmS,ImmB,ImmJ,ImmU; // int i = stoul(p, nullptr, 16);

bitset<7> opcode, funct7; // opcode

bitset<5> rd, rs1, rs2;

bitset<3> funct3;

int mtype;
int Op2;
int Op1_RFread,Op2_RFread;

//control signals
int BranchTargetSelect,ResultSelect,RFWrite,ALUOperation,OP1Select,OP2Select,MemOp;
int Isbranch,branchAdd;

//fetch stage
int fp_PC;
bitset<32> fp_Inst;

//decode stage
int dp_BranchTargetSelect,dp_ResultSelect,dp_RFWrite,dp_ALUOperation,dp_OP2Select;
int dp_MemOp,dp_Isbranch,dp_OP1Select;
int dp_ImmI,dp_ImmS,dp_ImmB,dp_ImmJ,dp_ImmU;
int dp_mtype,dp_Op2_RFread,dp_Op1,dp_rd,dp_rs1, dp_rs2;
int dp_PC;

//execute stage
int ep_ResultSelect,ep_resultALU,ep_MemOp,ep_Isbranch,ep_RFWrite;
int ep_mtype,ep_Op2_RFread,ep_OP2Select,ep_OP1Select;
int ep_PC,ep_rd,ep_rs1, ep_rs2;

//memory stage
int mp_resultALU,mp_resultMEM,mp_ResultSelect;
int mp_PC,mp_rd,mp_rs1, mp_rs2,mp_OP2Select,mp_OP1Select,mp_RFWrite;

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

//resolve hazard
void resolveH()
{
  if (flag_H==true && data_H!=true)
  {
    dp_Op1 = X[rs1.to_ulong()];
    dp_Op2_RFread = X[rs2.to_ulong()];
    EX=1;
    DE=1;
    IF=1;
    flag_H=false;
  }
  
}

//reset pipeline buffer
void resetIF()
{
  Inst=0;
}

void resetDE()
{
    dp_OP2Select = 0;
    dp_OP1Select = 0;
    dp_Op2_RFread = 0;
    dp_ALUOperation = 0;
    dp_BranchTargetSelect = 0;
    dp_mtype = 0;
    dp_ResultSelect = 0;
    dp_MemOp = 0;
    dp_RFWrite = 0;
    dp_Isbranch = 0;
    dp_Op1 = 0;
    dp_rd = 0;
    dp_rs1 = 0;
    dp_rs2 = 0;
    dp_ImmI = 0;
    dp_ImmS = 0;
    dp_ImmU = 0;
    dp_ImmJ = 0;
    dp_ImmB = 0;
}

void resetEX()
{
  ep_ResultSelect=0;
  ep_resultALU=0;
  ep_MemOp=0;
  ep_Isbranch=0;
  ep_RFWrite=0;
  ep_mtype=0;
  ep_Op2_RFread=0;
  ep_OP2Select=0;
  ep_OP1Select=0;
  ep_PC=0;
  ep_rd=0;
  ep_rs1=0;
  ep_rs2=0;
}

void resetMA()
{
  mp_ResultSelect=0;
  mp_resultALU=0;
  mp_RFWrite=0;
  mp_OP2Select=0;
  mp_OP1Select=0;
  mp_PC=0;
  mp_rd=0;
  mp_rs1=0;
  mp_rs2=0;
}

// reads from the instruction memory and updates the instruction register
void fetch()
{
  if (ep_Isbranch==0 && cycles!= 0)
  {
    PC=PC+4;
  }
  else if(ep_Isbranch==1 && cycles!=0)
  {
    PC=branchAdd;
  }

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
    cout<<s<<" ";
    while (1)
    {
      FileName >> x;
      if (FileName.eof())
      {
        loop=true;
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
  
}

// reads the instruction register, reads operand1, operand2 from register file, decides the operation to be performed in execute stage
void decode()
{
  Isbranch=0;
  MemOp=0;
  OP2Select=4;

  for (int i = 0; i < 7; i++) // opcode
  {
    opcode[i]=fp_Inst[i];
  }
  for (int i = 7; i < 12; i++) //rd
  {
    rd[i-7]=fp_Inst[i];
  }
  for (int i = 12; i < 14; i++) //funct3
  {
    funct3[i-12]=fp_Inst[i];
  }
  for (int i = 15; i < 20; i++) //rs1
  {
    rs1[i-15]=fp_Inst[i];
  }
  for (int i = 20; i < 25; i++) //rs2
  {
    rs2[i-20]=fp_Inst[i];
  }
  for (int i = 25; i < 32; i++)  //funct7
  {
    funct7[i-25]=fp_Inst[i];
  }

  Op1_RFread = X[rs1.to_ulong()];
  mtype=funct3.to_ulong();

  //for immidiate and control signals
  switch (opcode.to_ulong())
  {
  case 19://type I
    OP2Select=1;
    OP1Select=0;
    if (funct3.to_ulong()==1 || funct3.to_ulong()==5) //immidiate
    {
    
      for (int i = 0; i < 5; i++)
      {
        ImmI[i]=fp_Inst[i+20];
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
        ImmI[i]=fp_Inst[i+20];
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
    RFWrite=1;
    ResultSelect = 2;
    OP1Select=0;
    
    for (int i = 0; i < 12; i++)
    {
      ImmI[i]=fp_Inst[i+20];
    }
    for (int i = 12; i < 32; i++)
    {
      ImmI[i]=ImmI[11];
    }
    break;
  case 103://jalr immidiate
    ALUOperation=11;
    OP2Select=1;
    BranchTargetSelect=2;
    Isbranch=1;
    RFWrite=1;
    OP1Select=0;
    ResultSelect=1;
    
    for (int i = 0; i < 12; i++)
    {
      ImmI[i]=fp_Inst[i+20];
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
    RFWrite=0;
    OP1Select=0;
    ResultSelect=0;

    for (int i = 0; i < 5; i++)//immidiate
    {
      ImmS[i]=fp_Inst[i+7];
    }
    for (int i = 0; i < 7; i++)
    {
      ImmS[i+5]=fp_Inst[i+25];
    }
    for (int i = 12; i < 32; i++)
    {
      ImmS[i]=ImmS[11];
    }   
    break;
  case 99://type B immidiate
    Isbranch=1;
    BranchTargetSelect=1;
    RFWrite=0;
    ALUOperation =12;
    OP1Select=0;
    ResultSelect=0;
    OP2Select=0;

    ImmB[0]=0;
    ImmB[11]=fp_Inst[7];
    for (int i = 0; i < 4; i++)
    {
      ImmB[i+1]=fp_Inst[i+8];
    }
    for (int i = 0; i < 6; i++)
    {
      ImmB[i+5]=fp_Inst[i+25];
    }
    ImmB[12]=fp_Inst[31];
    for (int i = 13; i < 32; i++)
    {
      ImmB[i]=ImmB[12];
    }
    break;
  case 55://lui immidiate
    OP2Select=3;
    RFWrite=1;
    ALUOperation=10;
    OP1Select=1;
    ResultSelect=3;

    for (int i = 0; i < 12; i++)
    {
      ImmU[i]=0;
    }
    for (int i = 0; i < 20; i++)
    {
      ImmU[i+12]=fp_Inst[i+12];
    }    
    break;
  case 23://auipc immididate
    OP2Select=3;
    RFWrite=1;
    ALUOperation=9;
    OP1Select=1;
    ResultSelect=3;

    for (int i = 0; i < 12; i++)
    {
      ImmU[i]=0;
    }
    for (int i = 0; i < 20; i++)
    {
      ImmU[i+12]=fp_Inst[i+12];
    }
    break;
  case 111:// jal immidiate
    Isbranch=1;
    BranchTargetSelect=0;
    RFWrite=1;
    ALUOperation = 11;
    OP1Select=1;
    ResultSelect=1;

    ImmJ[0]=0;
    for (int i = 0; i < 8; i++)
    {
      ImmJ[i+12]=fp_Inst[i+12];
    }
    ImmJ[11]=fp_Inst[20];
    for (int i = 0; i < 10; i++)
    {
      ImmJ[i+1]=fp_Inst[i+21];
    }
    ImmJ[20]=fp_Inst[31];
    for (int i = 21; i < 32; i++)
    {
      ImmJ[i]=ImmJ[20];
    }
    break;
  default:
    break;
  }
  if (opcode.to_ulong()==51)
  {
    OP2Select=0;
  }
  if (opcode.to_ulong()==51 || opcode.to_ulong()==19)
  {
    RFWrite=1;
    OP1Select=0;
    ResultSelect=3;
    
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
    Op2=dp_Op2_RFread;
  }
  else if (dp_OP2Select==1)
  {
    Op2=dp_ImmI;
  }
  else if (dp_OP2Select==2)
  {
    Op2=dp_ImmS;
  }
  else if (dp_OP2Select==3)
  {
    Op2=dp_ImmU;
  }
  


  switch (dp_ALUOperation)
  {
  case 0:
    resultALU = dp_Op1 + Op2;
    break;
  case 1:
    resultALU = dp_Op1 - Op2;
    break;
  case 2:
    resultALU = dp_Op1 ^ Op2;
    break;
  case 3:
    resultALU = dp_Op1 | Op2;
    break;
  case 4:
    resultALU = dp_Op1 & Op2;
    break;
  case 5:
    resultALU = dp_Op1 << Op2;
    break;
  case 6:
    resultALU = shiftRL(dp_Op1,Op2);
    break;
  case 7:
    resultALU = dp_Op1 >> Op2;
    break;
  case 8:
    resultALU = (dp_Op1 < Op2 ? 1:0);
    break;
  case 9:
    resultALU = dp_PC + Op2;
    break;
  case 10:
    resultALU = Op2;
    break;
  case 11:
    resultALU = dp_PC + 4;
  case 12:
    
  default:
    break;
  }
  int zero;
  switch (dp_BranchTargetSelect)
  {
  case 0:
    branchAdd = dp_PC + dp_ImmJ;
    break;
  case 1:
    switch (dp_mtype)
    {
    case 0:
      zero = (dp_Op1 == dp_Op2_RFread)?0:1;
      break;
    case 1:
      zero = (dp_Op1 != dp_Op2_RFread)?0:1;
      break;
    case 4:
      zero = (dp_Op1 < dp_Op2_RFread)?0:1;
      break;
    case 5:
      zero = (dp_Op1 >= dp_Op2_RFread)?0:1;
      break;
    
    default:
      break;
    }

    if (zero==0)
    {
      branchAdd = dp_PC +dp_ImmB;
    }
    else
    {
      branchAdd = dp_PC + 4;
    }
    break;
  case 2:
    branchAdd = dp_Op1 + dp_ImmI;
    break;
  default:
    break;
  }

}
// perform the memory operation
void mem()
{
   bitset<8> a1,a2,a3,a4;
   bitset<16> b1;
   bitset<32> c1;
   if(ep_MemOp==1){
      switch(ep_mtype)
      {
        case 0:
          a1 = (int)(MEM[ep_resultALU]);
          resultMEM = (int8_t)(a1.to_ulong());
          break;
        case 1:
          a1 = (int)(MEM[ep_resultALU]);
          a2 = (int)(MEM[ep_resultALU+1]);
          for (int i = 0; i < 8; i++)
          {
            b1[i] = a1[i];
          }
          for (int i = 0; i < 8; i++)
          {
            b1[i+8] = a2[i];
          }
          resultMEM = (int16_t)(b1.to_ulong());
          break;
        case 2:
          a1 = (int)(MEM[ep_resultALU]);
          a2 = (int)(MEM[ep_resultALU+1]);
          a2 = (int)(MEM[ep_resultALU+2]);
          a2 = (int)(MEM[ep_resultALU+3]);
          for (int i = 0; i < 8; i++)
          {
            c1[i] = a1[i];
          }
          for (int i = 0; i < 8; i++)
          {
            c1[i+8] = a2[i];
          }
          for (int i = 0; i < 8; i++)
          {
            c1[i+16] = a2[i];
          }
          for (int i = 0; i < 8; i++)
          {
            c1[i+24] = a2[i];
          }
          resultMEM = (int32_t)(c1.to_ulong());
          break;
        default:
          break;
      }
   }
   else if(ep_MemOp==2)
   {
      switch(ep_mtype)
      {
        case 0:
          a1 = (int8_t)ep_Op2_RFread;
          MEM[ep_resultALU] = a1.to_ulong();
          break;
        case 1:
          b1 = (int16_t)ep_Op2_RFread;
          for (int i = 0; i < 8; i++)
          {
            a1[i] = b1[i];
          }
          for (int i = 0; i < 8; i++)
          {
            a2[i] = b1[i+8]; 
          }
          MEM[ep_resultALU] = a1.to_ulong();
          MEM[ep_resultALU+1] = a2.to_ulong();
          break;
        case 2:
          c1 = (int32_t)ep_Op2_RFread;
          for (int i = 0; i < 8; i++)
          {
            a1[i] = c1[i];
          }
          for (int i = 0; i < 8; i++)
          {
            a2[i] = c1[i+8]; 
          }
          for (int i = 0; i < 8; i++)
          {
            a3[i] = c1[i+16];
          }
          for (int i = 0; i < 8; i++)
          {
            a4[i] = c1[i+24]; 
          }
          MEM[ep_resultALU] = a1.to_ulong();
          MEM[ep_resultALU+1] = a2.to_ulong();
          MEM[ep_resultALU+2] = a3.to_ulong();
          MEM[ep_resultALU+3] = a4.to_ulong();
          break;

        default:
          break;
      }
   }
}
// writes the results back to register file
void write_back()
{
  
  switch (mp_ResultSelect)
  {
  
   case 1:
        X[mp_rd] = mp_PC+4;
        break;
   case 2:
        X[mp_rd]=mp_resultMEM;
       break; 
   case 3:
        X[mp_rd]=mp_resultALU;
       break;
  default:
    break;
  }
  X[0]=0;
}

//hand shake function for pipeline
void handshake()
{
  if(MA==1)
  {
    //MA/WA
    mp_PC = ep_PC;
    mp_rd =ep_rd;
    mp_rs1 =ep_rs1;
    mp_rs2 =ep_rs2;
    mp_resultALU = ep_resultALU;
    mp_ResultSelect = ep_ResultSelect;
    mp_resultMEM = resultMEM;
    mp_OP2Select = ep_OP2Select;
    mp_OP1Select = ep_OP1Select;
    WB=1;
  }
  else
  {
    WB=0;
  }
  

  if(EX==1)
  {
    //EX/MA
    ep_PC = dp_PC;
    ep_mtype = dp_mtype;
    ep_MemOp = dp_MemOp;
    ep_resultALU = resultALU;
    ep_ResultSelect = dp_ResultSelect;
    ep_Isbranch=dp_Isbranch;
    ep_RFWrite = dp_RFWrite;
    ep_Op2_RFread = dp_Op2_RFread;
    ep_rd = dp_rd;
    ep_rs1 = dp_rs1;
    ep_rs2 = dp_rs2;
    ep_OP2Select = dp_OP2Select;
    ep_OP1Select = dp_OP1Select;
    MA=1;
  }
  else
  {
    MA=0;
  }
  

  if(DE==1)
  {
    //DE/EX
    dp_PC = fp_PC;
    dp_OP2Select = OP2Select;
    dp_OP1Select= OP1Select;
    dp_Op2_RFread = Op2_RFread;
    dp_ALUOperation = ALUOperation;
    dp_BranchTargetSelect = BranchTargetSelect;
    dp_mtype = mtype;
    dp_ResultSelect = ResultSelect;
    dp_MemOp = MemOp;
    dp_RFWrite = RFWrite;
    dp_Isbranch = Isbranch;
    dp_Op1 = Op1_RFread;
    dp_rd = (int32_t)rd.to_ulong();
    dp_rs1 = (int32_t)rs1.to_ulong();
    dp_rs2 = (int32_t)rs2.to_ulong();
    dp_ImmI = (int32_t)ImmI.to_ulong();
    dp_ImmS = (int32_t)ImmS.to_ulong();
    dp_ImmU = (int32_t)ImmU.to_ulong();
    dp_ImmJ = (int32_t)ImmJ.to_ulong();
    dp_ImmB = (int32_t)ImmB.to_ulong();
    EX=1;
  }
  else
  {
    EX=0;
  }
  
  
  if(IF==1)
  {
    //IF/DE
    fp_PC = PC;
    fp_Inst = Inst;
    DE=1;
  }
  else
  {
    DE=0;
  }
  
  //hazards
  data_H=false;
  if (stay==1)
  {
    resetEX();
  }
  else if (stay==2)
  {
    resetMA();
  }
  
  

  if (stay==0 && ((ep_rd == dp_rs1 && ep_rd !=0 ) && (dp_OP1Select==0) || (ep_rd == dp_rs2 && ep_rd != 0 && dp_OP2Select == 0)) && (ep_RFWrite==1))
  {
    flag_H=true;
    data_H=true;
    IF=0;
    DE=0;
    EX=0;
    stay=1;
  }
  else if (stay==1 && ((mp_rd == dp_rs1 && mp_rd !=0 ) && (dp_OP1Select==0) || (mp_rd == dp_rs2 && mp_rd != 0 && dp_OP2Select == 0)) && (mp_RFWrite==1))
  {
    flag_H=true;
    data_H=true;
    IF=0;
    DE=0;
    EX=0;
    stay=2;
  }
  else if(ep_Isbranch == 1 )
  {
    IF=1;
    DE=0;
    EX=0;
    stay=3;
  }
  else
  {
    stay=0;
  }
  
  resolveH();
}

int main()
{
  X[2]=31996;
  while ((IF!=0 || DE!=0 || EX!=0 || MA!=0 || WB!=0))
  {
    
    
    if (loop==true)
    {
      IF=0;
      cout<<"loop"<<endl;
    }
    if(WB==1)
    {
      write_back();
      cout<<"wb ";
    }

    if(MA==1)
    {
      mem();
      cout<<"ma ";
    }
    if(EX==1)
    {
      execute();
      cout<<"ex ";
    }
    if(DE==1)
    {
      decode();
      cout<<"de ";
    }
    if (IF==1)
    {
      fetch();
      cout<<"if ";
    }
    handshake();

    cout<< std::hex<< std::uppercase<<(int32_t)Inst.to_ulong() <<" "<<dp_rd<<" "<<ep_rd<<" "<<mp_rd<<endl;
    cycles++;
  }

  //test
  cout<<cycles<<endl;
  //test

  ofstream outputFile("output.txt");
  outputFile <<"\n"<< "Values of resister's" <<"\n\n";
  for (int i = 0; i < 32; i++)
  {
    outputFile << "x" << i << " -> " << X[i] << endl;
  }
  outputFile <<"\n"<< "Values at the memory addresses" <<"\n\n";
  for (int i = 0; i < 32000/4; i++)
  {
    outputFile << "0x"<<hex<<i*4 <<" -> "<< (int)MEM[i*4]<<"   "<< (int)MEM[i*4+1]<<"   "<< (int)MEM[i*4+2]<<"   "<< (int)MEM[i*4+3] << endl;
  }
}
