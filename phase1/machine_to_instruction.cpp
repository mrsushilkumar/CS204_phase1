#include<iostream>
#include<fstream>
using namespace std;

void main()
{
    string t;
    int i=0;
    ifstream fbin("machine_code.txt");
    ofstream fbout("Instruction.mc");
    while (!fbin.eof())
    {
        fbin>>t;
        fbout<<"0x"<< std::hex<< std::uppercase <<i<<" "<<t<< std::endl;
        i=i+4;
    }
}
