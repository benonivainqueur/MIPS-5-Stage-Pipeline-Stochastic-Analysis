#include <iostream>
#include <fstream>
#include <cstdlib>
#include <math.h>
#include <stdlib.h>
#include <random>
using namespace std;

string instructions[100];
int mem[5];
int reg[10];
int clockcycle = 0;
ofstream fout("Result.txt");

bool lwhz = false;
bool bneq = false;
bool flu = false;

int pc = 0;
bool ifid_input = false;
string finstruction = "00000000000000000000000000000000";

int idpc = 0;
string op = "000000";
string fun = "000000";
string idexsignal = "000000000";
bool idex_input = 0;
int readdata1 = 0;
int readdata2 = 0;
int signextend = 0;
long int rs = 0;
int rt = 0;
int rd = 0;

bool exe_input = false;
int aluout = 0;
int writedata = 0;
int exert = 0;
int exepc = 0;
int temp = 0;
int temp2 = 0;
string exesignal = "000000000";
string aluop = "00";

bool meminput = false;
bool membranch = false;
int memreaddata = 0;
int memalu = 0;
int memrt = 0;
string memsignal = "00";

int wbrd = 0;

const double LOAD_USE_HAZARD_PROB = 0.9;
const double BRANCH_HAZARD_PROB = 0.9;

int load_use_hazard_cycles = 0;
int branch_hazard_cycles = 0;
int total_hazard_cycles = 0;
double performance_impact = 0.0;

bool simulateHazard(double probability) {
    double randn = ((double)rand() / RAND_MAX);
    cout << "Random number: " << randn << endl;
    return  randn < probability;
}

int b_d(string str, bool sign) {
    bool f = false;
    int ans = 0;
    for (int i = 0; i < str.length(); i++) {
        ans = ans * 2 + str[i] - '0';
        if (i == 0 && str[i] == '1')
            f = true;
    }
    if (f && sign)
        ans = -(ans);
    return ans;
}

void print();

void flush() {
    ifid_input = true;
    pc = exepc + 4;
    if (simulateHazard(LOAD_USE_HAZARD_PROB)) {
        cout << "Load-Use Hazard Triggered" << endl;
        load_use_hazard_cycles++;
        lwhz = true;
    }
    if (instructions[(pc / 4)] == "") {
        finstruction = "00000000000000000000000000000000";
        ifid_input = false;
    } else {
        finstruction = instructions[(pc / 4)];
    }
    idex_input = true;
    readdata1 = 0;
    readdata2 = 0;
    signextend = 0;
    rs = 0;
    rt = 0;
    rd = 0;
    idpc = 0;
    idexsignal = "000000000";
    print();
}

void detect() {
    if (exert == rs && exert != 0 && exesignal[3] == '1')
        readdata1 = aluout;
    if (exert == rt && exert != 0 && exesignal[3] == '1')
        readdata2 = aluout;

    if (memrt == rs && memrt != 0 && memsignal[0] == '1')
        readdata1 = (memsignal[1] == '1') ? memreaddata : memalu;
    if (memrt == rt && memrt != 0 && memsignal[0] == '1')
        readdata2 = (memsignal[1] == '1') ? memreaddata : memalu;

    if (idexsignal[5] == '1' && (rt == b_d(finstruction.substr(6, 5), false) || rt == b_d(finstruction.substr(11, 5), false))) {
        lwhz = true;
        pc -= 4;
        ifid_input = true;
    }
}

void writeback(string wbsignal, int rt, int alu, int readata) {
    if (rt == 0)
        return;
    if (wbsignal[0] == '1') {
        if (wbsignal[1] == '0')
            reg[rt] = alu;
        else if (wbsignal[1] == '1')
            reg[rt] = readata;
    } else if (wbsignal[0] == '0')
        return;
}

void instructiondecode() {
    idex_input = ifid_input;
    ifid_input = false;
    idpc = pc;
    op = finstruction.substr(0, 6);
    if (op == "000000") {
        rs = b_d(finstruction.substr(6, 5), false);
        rt = b_d(finstruction.substr(11, 5), false);
        rd = b_d(finstruction.substr(16, 5), false);
        fun = finstruction.substr(26);
        readdata1 = reg[rs];
        readdata2 = reg[rt];
        signextend = b_d(finstruction.substr(16, 16), true);
        if (rs == 0 && rt == 0 && rd == 0 && fun == "000000" || lwhz == true) {
            idexsignal = "000000000";
            lwhz = false;
        } else {
            idexsignal = "110000010";
        }
    } else {
        rs = b_d(finstruction.substr(6, 5), false);
        rt = b_d(finstruction.substr(11, 5), false);
        readdata1 = reg[rs];
        readdata2 = reg[rt];
        signextend = b_d(finstruction.substr(16, 16), true);
        rd = 0;
        if (op == "100011")
            idexsignal = "000101011";
        else if (op == "101011")
            idexsignal = "100100101";
        else if (op == "001000")
            idexsignal = "000100010";
        else if (op == "001100")
            idexsignal = "011100010";
        else if (op == "000101")
            idexsignal = "101010001";
    }
}

void execution() {
    exe_input = idex_input;
    idex_input = false;
    aluop = idexsignal.substr(1, 2);
    exesignal = idexsignal.substr(4, 5);
    writedata = readdata2;
    if (idexsignal[0] == '1' && idexsignal[1] == '1')
        exert = rd;
    else
        exert = rt;
    temp = readdata1;
    if (idexsignal[3] == '0')
        temp2 = readdata2;
    else
        temp2 = signextend;
    if (aluop == "10") {
        if (fun == "100000")
            aluout = temp + temp2;
        else if (fun == "100100")
            aluout = temp & temp2;
        else if (fun == "100101")
            aluout = temp | temp2;
        else if (fun == "100010")
            aluout = temp - temp2;
        else if (fun == "101010") {
            if (temp < temp2)
                aluout = 1;
            else
                aluout = 0;
        } else if (fun == "000000")
            aluout = 0;
    } else if (aluop == "00")
        aluout = temp + temp2;
    else if (aluop == "01") {
        aluout = temp - temp2;
        bneq = true;
        if (aluout != 0)
            flu = true;
    } else if (idexsignal == "000100010") {
        aluout = temp + temp2;
    } else if (idexsignal == "011100010") {
        aluout = temp & temp2;
    }
    exepc = idpc + (signextend * 4);
}

void memoryaccess() {
    meminput = exe_input;
    exe_input = false;
    membranch = false;
    memrt = exert;
    memalu = aluout;
    memsignal = exesignal.substr(3, 2);
    if (simulateHazard(BRANCH_HAZARD_PROB)) {
        branch_hazard_cycles++;
        cout << "Branch Hazard Triggered" << endl;
        bneq = true;
    }
    if (memsignal == "01" && memalu != 0 && bneq == true) {
        bneq = false;
        membranch = true;
    }
    if (memsignal == "11") {
        memreaddata = mem[memalu / 4];
    } else {
        memreaddata = 0;
    }
    if (memsignal == "01" && exesignal[2] == '1') {
        mem[memalu / 4] = writedata;
    }
}

void instructionfetch() {
    pc = pc + 4;
    ifid_input = true;
    if (instructions[(pc / 4)] == "") {
        finstruction = "00000000000000000000000000000000";
        ifid_input = false;
    } else {
        finstruction = instructions[(pc / 4)];
    }
}

bool gocycling() {
    wbrd = memrt;
    writeback(memsignal, memrt, memalu, memreaddata);
    memoryaccess();
    execution();
    instructiondecode();
    instructionfetch();
    clockcycle++;
    wbrd = memrt;

    if (flu == true) {
        flu = false;
        flush();
    } else {
        print();
    }
    detect();

    if ((memsignal == "00" && memalu == 0 && memreaddata == 0) && exesignal == "00000" && idexsignal == "000000000" && clockcycle != 1) {
        return false;
    } else
        return true;
}

void print() {
    fout << "CC " << clockcycle << ":" << endl;
    fout << endl;
    fout << "Registers:" << endl;
    for (int k = 0; k < 10; k++)
        fout << "$" << k << ": " << reg[k] << endl;
    fout << endl;
    fout << "Data memory:" << endl;
    fout << "0x00: " << mem[0] << endl;
    fout << "0x04: " << mem[1] << endl;
    fout << "0x08: " << mem[2] << endl;
    fout << "0x0C: " << mem[3] << endl;
    fout << "0x10: " << mem[4] << endl;
    fout << endl;
    fout << "IF/ID :" << endl;
    fout << "PC              " << pc << endl;
    fout << "Instruction     " << finstruction << endl;
    fout << endl;
    fout << "ID/EX :" << endl;
    fout << "ReadData1       " << readdata1 << endl;
    fout << "ReadData2       " << readdata2 << endl;
    fout << "sign_ext        " << signextend << endl;
    fout << "Rs              " << rs << endl;
    fout << "Rt              " << rt << endl;
    fout << "Rd              " << rd << endl;
    fout << "Control signals " << idexsignal << endl;
    fout << endl;
    fout << "EX/MEM :" << endl;
    fout << "ALUout          " << aluout << endl;
    fout << "WriteData       " << writedata << endl;
    fout << "Rt/Rd           ";
    fout << exert << endl;
    fout << "Control signals " << exesignal << endl;
    fout << endl;
    fout << "MEM/WB :" << endl;
    fout << "ReadData        " << memreaddata << endl;
    fout << "ALUout          " << memalu << endl;
    fout << "Rt/Rd           " << wbrd << endl;
    fout << "Control signals " << memsignal << endl;
    
    // Add hazard information
    if (lwhz) {
        fout << "Load-Use Hazard Triggered" << endl;
    }
    if (bneq) {
        fout << "Branch Hazard Triggered" << endl;
    }
    if (flu) {
        fout << "Forwarding Unit Used" << endl;
    }
    
    fout << "=================================================================" << endl;
}


int main() {
    mem[0] = 5;
    mem[1] = 9;
    mem[2] = 4;
    mem[3] = 8;
    mem[4] = 7;

    reg[0] = 0;
    reg[1] = 9;
    reg[2] = 8;
    reg[3] = 7;
    reg[4] = 1;
    reg[5] = 2;
    reg[6] = 3;
    reg[7] = 4;
    reg[8] = 5;
    reg[9] = 6;

    ifstream fin("Input.txt");
    int i = 1;
    while (fin >> instructions[i])
        i++;

    srand(time(NULL)); // Seed for random number generator

    while (gocycling() == true);

    fin.close();
    fout.close();

    // Calculate the number of additional cycles needed due to hazards
    // load_use_hazard_cycles = lwhz ? 1 : 0;
    // branch_hazard_cycles = bneq ? 1 : 0;
    total_hazard_cycles = load_use_hazard_cycles + branch_hazard_cycles;

    // Calculate performance impact
    performance_impact = (double)total_hazard_cycles / (double)clockcycle * 100;

    cout << "Total cycles lost due to Load-Use Hazards: " << load_use_hazard_cycles << endl;
    cout << "Total cycles lost due to Branch Hazards: " << branch_hazard_cycles << endl;
    cout << "Total additional cycles needed due to hazards: " << total_hazard_cycles << endl;
    cout << "Performance impact due to hazards (%): " << performance_impact << endl;

    return 0;
}