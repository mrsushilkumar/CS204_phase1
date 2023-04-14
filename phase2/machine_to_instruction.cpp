#include<iostream>
#include<fstream>
using namespace std;

int main()
{
    string t;
    int i=0;
    ifstream fbin("machine_code.txt");
    ofstream fbout("input.mc");
    while (!fbin.eof())
    {
        fbin>>t;
        fbout<<"0x"<< std::hex <<i<<" "<<t<< std::endl;
        i=i+4;
    }
    return 0;
}
