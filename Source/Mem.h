#ifndef MEM_H
#define MEM_H

#include <unordered_map>
using namespace std;

class Mem
{
private:
    unordered_map<int, int> mem;

public:
    Mem();
    void assign(int Addr, int value);
    int read(int Addr) const;
    void initialize(int Addr, int value);
};

#endif // MEM_H
