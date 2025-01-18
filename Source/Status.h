#ifndef STATUS_H
#define STATUS_H

#include <vector>
using namespace std;

class Status
{
private:
    struct Register
    {
        bool NotAv;
        int ReOrder;
    };
    vector<Register> Rs;

public:
    Status();
    bool Notbusy(int reg) const;
    int ReOrderGet(int reg) const;
    void AssignAvalivblty(int reg, bool NotAv);
    void ReOrderSet(int reg, int ReOrder);
    void Initialze();
};

#endif // STATUS_H
