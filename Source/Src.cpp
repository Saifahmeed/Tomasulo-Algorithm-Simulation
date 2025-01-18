#include <bits/stdc++.h>
#include "Opcode.h"
#include "Rob.h"
#include "Reordring.h"
#include "Mem.h"
#include "Status.h"
#include "Branch_Predict.h"
using namespace std;

string StringofOp(Opcode opcode)
{
    if (opcode == Opcode::ADD)
        return "ADD";
    else if (opcode == Opcode::ADDI)
        return "ADDI";
    else if (opcode == Opcode::MUL)
        return "MUL";
    else if (opcode == Opcode::LOAD)
        return "LOAD";
    else if (opcode == Opcode::STORE)
        return "STORE";
    else if (opcode == Opcode::BEQ)
        return "BEQ";
    else if (opcode == Opcode::CALL)
        return "CALL";
    else if (opcode == Opcode::RET)
        return "RET";
    else if (opcode == Opcode::NAND)
        return "NAND";
    else
        return "NotKnown";
}

class Inst
{
private:
    Opcode opcode;
    int rs1, rs2, rd, Imm;
    int memAdr;
    int IssueCyc, execStartCyc, execEndCyc, WriteCyc, CommCyc;
    bool Spec;

public:
    Inst(Opcode opp = Opcode::NotKnown, int rs1 = 0, int rs2 = 0, int rd = 0, int imm = 0, int Addr = -1)
        : opcode(opp), rs1(rs1), rs2(rs2), rd(rd), Imm(imm), memAdr(Addr),
        IssueCyc(-1), execStartCyc(-1), execEndCyc(-1), WriteCyc(-1), CommCyc(-1), Spec(false) {
    }
    int ImmGet() const { return Imm; }
    int memAddrGet() const { return memAdr; }
    Opcode OpGet() const { return opcode; }
    int IssueCycGet() const { return IssueCyc; }
    int rs1Get() const { return rs1; }
    int rs2Get() const { return rs2; }
    int rdGet() const { return rd; }
    int execStartCycGet() const { return execStartCyc; }
    int execEndCycGet() const { return execEndCyc; }
    void execEndCycSet(int Cyc) { execEndCyc = Cyc; }
    int WriteCycGet() const { return WriteCyc; }
    void IssueCycSet(int Cyc) { IssueCyc = Cyc; }
    void execStartCycSet(int Cyc) { execStartCyc = Cyc; }
    int CommCycGet() const { return CommCyc; }
    void WriteCycSet(int Cyc) { WriteCyc = Cyc; }
    void CommCycSet(int Cyc) { CommCyc = Cyc; }
    static Inst PARSE(const string L, int Addr)
    {
        stringstream PUT(L);
        string StringOfOp, R1, R2, R3, StringOfImm;

        PUT >> StringOfOp;
        if (StringOfOp == "ADD")
        {
            PUT >> R1 >> R2 >> R3;
            return Inst(Opcode::ADD, RegParse(R2), RegParse(R3), RegParse(R1), 0, Addr);
        }
        else if (StringOfOp == "ADDI")
        {
            PUT >> R1 >> R2 >> StringOfImm;
            return Inst(Opcode::ADDI, RegParse(R2), 0, RegParse(R1), stoi(StringOfImm), Addr);
        }
        else if (StringOfOp == "MUL")
        {
            PUT >> R1 >> R2 >> R3;
            return Inst(Opcode::MUL, RegParse(R2), RegParse(R3), RegParse(R1), 0, Addr);
        }
        else if (StringOfOp == "LOAD")
        {
            PUT >> R1 >> StringOfImm;
            size_t S = StringOfImm.find('('), T = StringOfImm.find(')');
            int Offs = stoi(StringOfImm.substr(0, S));
            int Bs = RegParse(StringOfImm.substr(S + 1, T - S - 1));
            return Inst(Opcode::LOAD, Bs, 0, RegParse(R1), Offs, Addr);
        }
        else if (StringOfOp == "STORE")
        {
            PUT >> R1 >> StringOfImm;
            size_t S = StringOfImm.find('('), T = StringOfImm.find(')');
            int Offs = stoi(StringOfImm.substr(0, S));
            int Bs = RegParse(StringOfImm.substr(S + 1, T - S - 1));
            return Inst(Opcode::STORE, Bs, RegParse(R1), 0, Offs, Addr);
        }
        else if (StringOfOp == "BEQ")
        {
            PUT >> R1 >> R2 >> StringOfImm;
            return Inst(Opcode::BEQ, RegParse(R1), RegParse(R2), 0, stoi(StringOfImm), Addr);
        }
        else if (StringOfOp == "CALL")
        {
            PUT >> StringOfImm;
            return Inst(Opcode::CALL, 0, 0, 0, stoi(StringOfImm), Addr);
        }
        else if (StringOfOp == "RET")
        {
            return Inst(Opcode::RET, 0, 0, 0, 0, Addr);
        }
        return Inst(Opcode::NotKnown);
    }

    static int RegParse(const string reg)
    {
        int n = stoi(reg.substr(1));
        return n;
    }
};
Rob::Rob() : opcode(Opcode::NotKnown), dest(-1), value(0), busy(false), Spec(false), misP(false), Index(-1) {}
void Rob::WriteBcycsSet(int cyc) { WriteBcycs = cyc; }
int Rob::WriteBcycsGet() const { return WriteBcycs; }
void Rob::assign_opcode(Opcode op) { opcode = op; }
Opcode Rob::OpGet() const { return opcode; }
void Rob::DestSet(int d) { dest = d; }
int Rob::DestGet() const { return dest; }
void Rob::valueSet(int v) { value = v; }
int Rob::valueGet() const { return value; }
void Rob::busySet(bool r) { busy = r; }
bool Rob::busyGet() const { return busy; }
void Rob::IndexSet(int id) { Index = id; }
int Rob::IndexGet() const { return Index; }
void Rob::Initialze()
{
    opcode = Opcode::NotKnown;
    dest = -1;
    value = 0;
    busy = false;
    Spec = false;
    misP = false;
    Index = -1;
}


Reordring::Reordring(int len)
    : DataRob(len), headPointer(0), tailPointer(0), len(len), c(0) {
}

int Reordring::allocate()
{
    if (IsCountEqLen())
    {
        return -1;
    }
    int j = tailPointer;
    tailPointer = (tailPointer + 1) % len;
    c++;
    return j;
}

Rob& Reordring::DataRobGet(int j)
{
    return DataRob[j];
}

void Reordring::DataRobSet(int j, const Rob& entry)
{
    DataRob[j] = entry;
}

Rob& Reordring::topp()
{
    return DataRob[headPointer];
}

void Reordring::lastt()
{
    DataRob[headPointer].Initialze();
    headPointer = (headPointer + 1) % len;
    c--;
}

bool Reordring::IsCountEqLen() const
{
    return c == len;
}

bool Reordring::IsCountZero() const
{
    return c == 0;
}

Status::Status() : Rs(32, { false, -1 }) {}

bool Status::Notbusy(int reg) const
{
    return reg == 0 ? false : Rs[reg].NotAv;
}

int Status::ReOrderGet(int reg) const
{
    return Rs[reg].ReOrder;
}

void Status::AssignAvalivblty(int reg, bool NotAv)
{
    if (reg != 0)
        Rs[reg].NotAv = NotAv;
}

void Status::ReOrderSet(int reg, int ReOrder)
{
    if (reg != 0)
        Rs[reg].ReOrder = ReOrder;
}

void Status::Initialze()
{
    for (int i = 1; i < Rs.size(); ++i)
    {
        Rs[i].ReOrder = -1;
        Rs[i].NotAv = false;
    }
}

Mem::Mem() {}

void Mem::assign(int Addr, int value)
{
    mem[Addr] = value & 0xFFFF;  //  16 bits
}

int Mem::read(int Addr) const
{
    auto point = mem.find(Addr);
    int value = (point != mem.end()) ? point->second : 0;
    return value;
}

void Mem::initialize(int Addr, int value)
{
    assign(Addr, value);
}

Branch_Predict::Branch_Predict() : AllBranches(0), mispred(0) {}

bool Branch_Predict::predict() const
{
    return false;
}

void Branch_Predict::assignPred(bool real, bool pred)
{
    if (real != pred)
        mispred++;
    AllBranches++;
}

double Branch_Predict::mispredRate() const
{
    if (AllBranches == 0)
        return 0.0;
    return (static_cast<double>(mispred) / AllBranches) * 100.0;
}

void Branch_Predict::print() const
{
    cout << "\n                  Branch Prediction Statistics                \n";
    cout << "─────────────────────────────────────────────────────────────────────────────────────────\n";
    cout << "─ Total Branches           │ " << setw(15) << right << AllBranches << " \n";
    cout << "─ Mispredictions           │ " << setw(15) << right << mispred << " \n";
    cout << "─ Misprediction Rate (%)   │ " << setw(15) << right << mispredRate() << " \n";
    cout << "─────────────────────────────────────────────────────────────────────────────────────────\n";
}

class ReservationStation
{
private:
    bool busy;
    Opcode opcode;
    int Vj, Vk; // values o
    int Qj, Qk;
    int Imm;
    int EntryId;
    int Cycsleft;

public:
    ReservationStation()
        : busy(false), opcode(Opcode::NotKnown), Vj(0), Vk(0), Qj(-1), Qk(-1),
        Imm(0), EntryId(-1), Cycsleft(0) {
    }

    void AssignAvalivblty(bool b) { busy = b; }
    bool Notbusy() const { return busy; }

    void assign_opcode(Opcode op) { opcode = op; }
    Opcode OpGet() const { return opcode; }

    void VjSett(int v) { Vj = v; }
    int VjGet() const { return Vj; }

    void VkSet(int v) { Vk = v; }
    int VkGet() const { return Vk; }

    void QjSet(int q) { Qj = q; }
    int QjGet() const { return Qj; }

    void QkSet(int q) { Qk = q; }
    int QkGet() const { return Qk; }

    void immSet(int imm) { Imm = imm; }
    int ImmGet() const { return Imm; }

    void ROBentryIndexSet(int index) { EntryId = index; }
    int ROBentryIndexGet() const { return EntryId; }

    void CycsLeftSet(int Cycs) { Cycsleft = Cycs; }
    int CycsLeftGet() const { return Cycsleft; }

    void CyscsMinusOne()
    {
        if (Cycsleft > 0)
            Cycsleft--;
    }

    void Initialze()
    {
        busy = false;
        opcode = Opcode::NotKnown;
        Vj = Vk = 0;
        Qj = Qk = -1;
        Imm = 0;
        EntryId = -1;
        Cycsleft = 0;
    }
};

class ExecutionUnit
{
private:
    vector<int> Regs;
    vector<ReservationStation> RS;
    Reordring rob;
    vector<Inst> Insts;
    Status RegStatus;
    Mem& mem;
    int Cyc;
    Branch_Predict branchBredictor;

    unordered_map<Opcode, int> busylist;
    unordered_map<Opcode, int> exec_Cycs;

    int CommitedInsts;
    int branchMispred;

public:
    ExecutionUnit(int reservStationLen, int ROBlen, Mem& mem)
        : RS(reservStationLen), rob(ROBlen), mem(mem), Cyc(0),
        CommitedInsts(0), branchMispred(0)
    {
        exec_Cycs = {
            {Opcode::LOAD, 6},
            {Opcode::STORE, 6},
            {Opcode::BEQ, 1},
            {Opcode::CALL, 1},
            {Opcode::RET, 1},
            {Opcode::ADD, 2},
            {Opcode::ADDI, 2},
            {Opcode::NAND, 1},
            {Opcode::MUL, 8} };
        Regs.resize(32, 0);
        busylist = {
            {Opcode::LOAD, 2},
            {Opcode::STORE, 1},
            {Opcode::BEQ, 1},
            {Opcode::CALL, 1},
            {Opcode::RET, 1},
            {Opcode::ADD, 4},
            {Opcode::ADDI, 4},
            {Opcode::NAND, 2},
            {Opcode::MUL, 1} };
    }

    void RegSet(int reg, int value)
    {
        if (reg >= 0 && reg < Regs.size())
            Regs[reg] = value;
    }

    void PutInts(const vector<Inst>& Ins)
    {
        Insts = Ins;
    }

    void Issue()
    {
        for (size_t i = 0; i < Insts.size(); i++)
        {
            int IssureCyc;
            auto& Ins = Insts[i];
            if (Ins.IssueCycGet() == -1)
            {
                bool RScheck = false;
                size_t RSid = 0;
                for (; RSid < RS.size(); RSid++)
                {
                    if (!RS[RSid].Notbusy() && busylist[Ins.OpGet()] > 0)
                    {
                        RScheck = true;
                        IssureCyc = Cyc;
                        bool errorr;
                        do
                        {
                            errorr = false;
                            for (const auto& instIssued : Insts)
                            {
                                if (instIssued.IssueCycGet() == IssureCyc)
                                {
                                    errorr = true;
                                    IssureCyc++;
                                    break;
                                }
                            }
                        } while (errorr);

                        Ins.IssueCycSet(IssureCyc);
                        break;
                    }
                }
                if (!RScheck)
                {
                    continue;
                }
                int ROBentryIndex = rob.allocate();
                if (ROBentryIndex == -1)
                {
                    break;
                }
                Rob ROBentry;
                ROBentry.assign_opcode(Ins.OpGet());
                ROBentry.DestSet(Ins.rdGet());
                ROBentry.busySet(false);
                ROBentry.IndexSet(i);
                rob.DataRobSet(ROBentryIndex, ROBentry);
                // found RS
                RS[RSid].AssignAvalivblty(true);
                RS[RSid].assign_opcode(Ins.OpGet());
                RS[RSid].ROBentryIndexSet(ROBentryIndex);
                RS[RSid].immSet(Ins.ImmGet());
                // rs1
                if (Ins.rs1Get() != 0)
                {
                    if (RegStatus.Notbusy(Ins.rs1Get()))
                    {
                        int ReOrder = RegStatus.ReOrderGet(Ins.rs1Get());
                        RS[RSid].QjSet(ReOrder);
                    }
                    else
                    {
                        RS[RSid].VjSett(Regs[Ins.rs1Get()]);
                        RS[RSid].QjSet(-1);
                    }
                }
                else
                {
                    RS[RSid].VjSett(0);
                    RS[RSid].QjSet(-1);
                }

                // rs2
                if (Ins.OpGet() == Opcode::ADDI || Ins.OpGet() == Opcode::LOAD || Ins.OpGet() == Opcode::CALL || Ins.OpGet() == Opcode::BEQ)
                {
                    RS[RSid].VkSet(Ins.ImmGet());
                    RS[RSid].QkSet(-1);
                }
                else if (Ins.OpGet() == Opcode::STORE)
                {
                    if (Ins.rs2Get() != 0)
                    {
                        if (RegStatus.Notbusy(Ins.rs2Get()))
                        {
                            int r = RegStatus.ReOrderGet(Ins.rs2Get());
                            RS[RSid].QkSet(r);
                        }
                        else
                        {
                            RS[RSid].VkSet(Regs[Ins.rs2Get()]);
                            RS[RSid].QkSet(-1);
                        }
                    }
                    else
                    {
                        RS[RSid].VkSet(0);
                        RS[RSid].QkSet(-1);
                    }
                }
                else
                {
                    if (Ins.rs2Get() != 0)
                    {
                        if (RegStatus.Notbusy(Ins.rs2Get()))
                        {
                            int r = RegStatus.ReOrderGet(Ins.rs2Get());
                            RS[RSid].QkSet(r);
                        }
                        else
                        {
                            RS[RSid].VkSet(Regs[Ins.rs2Get()]);
                            RS[RSid].QkSet(-1);
                        }
                    }
                    else
                    {
                        RS[RSid].VkSet(0);
                        RS[RSid].QkSet(-1);
                    }
                }
                RS[RSid].CycsLeftSet(exec_Cycs[Ins.OpGet()]);
                if (Ins.OpGet() != Opcode::STORE && Ins.OpGet() != Opcode::BEQ && Ins.OpGet() != Opcode::CALL && Ins.OpGet() != Opcode::RET)
                {
                    RegStatus.AssignAvalivblty(Ins.rdGet(), true);
                    RegStatus.ReOrderSet(Ins.rdGet(), ROBentryIndex);
                }
                busylist[Ins.OpGet()]--;
            }
        }
    }

    void Execution()
    {
        for (auto& rs : RS)
        {
            // Check if the reservation station is busy and ready to Execution
            if (rs.Notbusy() && rs.QjGet() == -1 && rs.QkGet() == -1)
            {
                int ROBentryIndex = rs.ROBentryIndexGet();
                Rob& ROBentry = rob.DataRobGet(ROBentryIndex);
                Inst& Ins = Insts[ROBentry.IndexGet()];

                // Handle SW dependency on LD write-back
                if (Ins.OpGet() == Opcode::STORE)
                {
                    int rSource = rs.QkGet(); // Dependency on Qk for SW
                    if (rSource != -1)
                    {
                        Rob& ROBs = rob.DataRobGet(rSource);
                        if (!ROBs.busyGet())
                        {
                            // Wait for the LD instruction to complete and write back
                            Ins.execStartCycSet(ROBs.WriteBcycsGet() + 1);
                            continue;
                        }
                    }
                }

                // Ensure start_execution_cycle is set correctly
                if (Ins.execStartCycGet() == -1) // Execution not started yet
                {
                    // Set start_execution_cycle dynamically based on dependencies
                    Ins.execStartCycSet(max(Ins.IssueCycGet() + 1, Cyc));
                }

                // Proceed with execution if the current cycle matches the start_execution_cycle
                if (Cyc >= Ins.execStartCycGet())
                {
                    if (rs.CycsLeftGet() > 0)
                    {
                        rs.CyscsMinusOne();// Decrement remaining cycles
                        if (rs.CycsLeftGet() == 0)
                        {
                            // Execution complete
                            int final = performOperation(rs.OpGet(), rs.VjGet(), rs.VkGet(), rs.ImmGet());

                            ROBentry.valueSet(final);
                            ROBentry.busySet(true);
                            Ins.execEndCycSet(Cyc);
                            Ins.WriteCycSet(Cyc + 1);

                            if (rs.OpGet() == Opcode::BEQ)
                            {
                                bool done = (rs.VjGet() == rs.VkGet());
                                bool pred = branchBredictor.predict();
                                branchBredictor.assignPred(done, pred);
                                if (done != pred)
                                {
                                    branchMispred++;
                                    flushing();
                                }
                            }

                            rs.Initialze();
                            busylist[rs.OpGet()]++;
                        }
                    }
                }
            }
        }
    }

    void WriteBack()
    {
        for (auto& rs : RS)
        {
            if (rs.Notbusy())
            {
                if (rs.QjGet() != -1)
                {
                    Rob& hold = rob.DataRobGet(rs.QjGet());
                    if (hold.busyGet())
                    {
                        rs.VjSett(hold.valueGet());
                        rs.QjSet(-1);
                    }
                }
                if (rs.QkGet() != -1)
                {
                    Rob& hold = rob.DataRobGet(rs.QkGet());
                    if (hold.busyGet())
                    {
                        rs.VkSet(hold.valueGet());
                        rs.QkSet(-1);
                    }
                }
            }
        }
    }

    void Commit()
    {
        if (!rob.IsCountZero())
        {
            Rob& entry = rob.topp();
            Inst& Ins = Insts[entry.IndexGet()];

            if (entry.busyGet() && Cyc > Ins.WriteCycGet())
            {
                if (entry.OpGet() == Opcode::STORE)
                {
                    mem.assign(entry.DestGet() + Ins.ImmGet(), entry.valueGet());
                }
                else if (entry.OpGet() == Opcode::BEQ)
                {
                }
                else if (entry.OpGet() == Opcode::CALL)
                {
                    Regs[1] = Ins.memAddrGet() + 1;
                    RegStatus.AssignAvalivblty(1, false);
                }
                else if (entry.OpGet() == Opcode::RET)
                {
                }
                else
                {
                    Regs[entry.DestGet()] = entry.valueGet();
                    RegStatus.AssignAvalivblty(entry.DestGet(), false); // free up
                }

                Ins.CommCycSet(Cyc);
                rob.lastt();
                CommitedInsts++;
            }
        }
    }

    void flushing()
    {
        // Flush busy RS
        for (auto& rs : RS)
        {
            if (rs.Notbusy())
            {
                busylist[rs.OpGet()]++;
                rs.Initialze();
            }
        }
        // Clear ROB
        while (!rob.IsCountZero())
            rob.lastt();
        RegStatus.Initialze();
    }
    void run()
    {
        while (CommitedInsts < Insts.size())
        {
            Commit();
            WriteBack();
            Execution();
            Issue();
            Cyc++;
        }
        double IPC = static_cast<double>(CommitedInsts) / Cyc;
        double branMisPredPercent = branchBredictor.mispredRate();
        Output();
        Output2(Cyc, CommitedInsts, IPC, branMisPredPercent);
        branchBredictor.print();
    }
    int performOperation(Opcode opcode, int Vj, int Vk, int imm) const
    {
        switch (opcode)
        {
        case Opcode::ADD:
            return Vj + Vk;
        case Opcode::ADDI:
            return Vj + imm;
        case Opcode::MUL:
            return (Vj * Vk) & 0xFFFF;
        case Opcode::NAND:
            return ~(Vj & Vk);
        case Opcode::LOAD:
            return mem.read(Vj + imm);
        case Opcode::STORE:
            return Vk;
        default:
            return 0;
        }
    }
    void Output()
    {
        cout << "\n                  Instruction Execution Metrics                 \n";
        cout << "─────────────────────────────────────────────────────────────────────────────────────────\n";
        cout << "─   Opcode   ─   Issue   ─  Start Execution  ─  End Execution  ─  Write   ─  Commit   ─\n";
        cout << "─────────────────────────────────────────────────────────────────────────────────────────\n";
        for (const auto& ins : Insts)
        {
            cout << " " << setw(10) << left << StringofOp(ins.OpGet())
                << "  " << setw(9) << right << ins.IssueCycGet()
                << "  " << setw(12) << right << ins.execStartCycGet()
                << "  " << setw(15) << right << ins.execEndCycGet()
                << "  " << setw(14) << right << ins.WriteCycGet()
                << "  " << setw(11) << right << ins.CommCycGet()
                << " \n";
        }

        cout << "─────────────────────────────────────────────────────────────────────────────────────────\n";
    }
    void Output2(int cycles, int commitedIns, double IPC, double branMisPredPercent)
    {
        cout << "\n                  Performance Metrics  \n";
        cout << "─────────────────────────────────────────────────────────────────────────────────────────\n";
        cout << "─ Total Execution Time (Cycles): " << setw(6) << right << cycles << " \n";
        cout << "─ IPC: " << setw(30) << fixed << setprecision(2) << right << IPC << " \n";
        cout << "─ Branch Misprediction (%): " << setw(6) << fixed << setprecision(2) << right << branMisPredPercent << " \n";
        cout << "─────────────────────────────────────────────────────────────────────────────────────────\n";
    }

};
vector<string> readInstsFromFile(const string& filename)
{
    vector<string> InstFromFile;
    ifstream file(filename);
    string L;
    while (getline(file, L))
    {
        L.erase(0, L.find_first_not_of(" \t"));
        L.erase(L.find_last_not_of(" \t") + 1);
        if (!L.empty())
        {
            InstFromFile.push_back(L);
        }
    }
    file.close();
    return InstFromFile;
}
// int main(int argc, char* argv[])
// {
//     if (argc < 2)
//     {
//         cerr << "Error: No file provided." << endl;
//         return 1; // Exit with an error if no file is provided
//     }

//     // Get the file path from the command line argument
//     string filename = argv[1];

//     // Print the file path to verify it's being passed correctly
//     cout << "Reading from file: " << filename << endl;

//     // Initialize memory and execution unit
//     Mem mem;
//     int reservStationLen = 10;
//     int ROBlen = 6;
//     ExecutionUnit executionUnit(reservStationLen, ROBlen, mem);

//     // Read instructions from the file
//     vector<string> InstFromFile = readInstsFromFile(filename);

//     vector<Inst> Insts;
//     int Addr = 0;
//     for (const auto& INS : InstFromFile)
//     {
//         Insts.push_back(Inst::PARSE(INS, Addr));
//         Addr += 4;
//     }

//     // Put instructions into the execution unit and run
//     executionUnit.PutInts(Insts);
//     executionUnit.run();

//     return 0;
// }

// int main(int argc, char* argv[])
// {
//     // Check if the input file was provided via command line arguments
//     if (argc < 2)
//     {
//         cerr << "Error: Please provide the input file as an argument." << endl;
//         return 1;
//     }

//     // Get the input file name from the command line argument
//     string inputFile = argv[1];

//     // Open output file (redirecting cout to output.txt)
//     freopen("output.txt", "w", stdout);

//     // Initialize the memory, execution unit, and read instructions
//     Mem mem;
//     int reservStationLen = 10;
//     int ROBlen = 6;
//     ExecutionUnit executionUnit(reservStationLen, ROBlen, mem);

//     // Read instructions from the file passed via the command line
//     vector<string> InstFromFile = readInstsFromFile(inputFile);
//     vector<Inst> Insts;
//     int Addr = 0;
//     for (const auto& INS : InstFromFile)
//     {
//         Insts.push_back(Inst::PARSE(INS, Addr));
//         Addr += 4;
//     }

//     // Run the execution unit
//     executionUnit.PutInts(Insts);
//     executionUnit.run();

//     return 0;
// }

