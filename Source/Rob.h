#ifndef ROB_H
#define ROB_H

#include "Opcode.h"

class Rob {
private:
    Opcode opcode;
    int dest;
    int value;
    bool busy;
    bool Spec;
    bool misP;
    int Index;
    int WriteBcycs;

public:
    Rob();
    void assign_opcode(Opcode op);
    Opcode OpGet() const;
    void DestSet(int d);
    int DestGet() const;
    void valueSet(int v);
    int valueGet() const;
    void busySet(bool r);
    void WriteBcycsSet(int cycle);
    int WriteBcycsGet() const;
    bool busyGet() const;
    void IndexSet(int id);
    int IndexGet() const;
    void Initialze();
};

#endif
