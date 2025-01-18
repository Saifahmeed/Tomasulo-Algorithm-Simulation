#ifndef BRANCH_PREDICT_H
#define BRANCH_PREDICT_H

#include <iostream>
#include <iomanip>

class Branch_Predict
{
private:
    int AllBranches;
    int mispred;

public:
    Branch_Predict();

    bool predict() const;
    void assignPred(bool real, bool pred);
    double mispredRate() const;
    void print() const;
};

#endif // BRANCH_PREDICT_H
