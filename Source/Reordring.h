#ifndef REORDRING_H
#define REORDRING_H

#include <vector>
#include "Rob.h"

class Reordring
{
private:
    std::vector<Rob> DataRob;
    int len;
    int headPointer;
    int tailPointer;
    int c;

public:
    Reordring(int len);
    int allocate();
    Rob& DataRobGet(int j);
    void DataRobSet(int j, const Rob& entry);
    Rob& topp();
    void lastt();
    bool IsCountEqLen() const;
    bool IsCountZero() const;
};

#endif // REORDRING_H
