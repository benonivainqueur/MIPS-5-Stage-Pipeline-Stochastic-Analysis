#include <iostream>
#include <fstream>
#include <cstdlib>
#include <math.h>
#include <stdlib.h>
#include <random>
#include <vector>
#include <algorithm>
#include <limits>
using namespace std;

// Array to store instructions
string instructions[500000];

// Arrays to store memory, registers, and pipeline stage information
int mem[5];
int reg[10];
int clockcycle = 0;
ofstream fout("Result.txt");

// Flags for different hazards
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
int rs = 0;
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

// Constants for hazard probabilities
double LOAD_USE_HAZARD_PROB = 0.1;
double BRANCH_HAZARD_PROB = 0.1;

// Variables to store hazard cycles and performance impact
int load_use_hazard_cycles = 0;
int branch_hazard_cycles = 0;
int total_hazard_cycles = 0;
double performance_impact = 0.0;

// Function to simulate hazards based on probability
bool simulateHazard(double probability)
{
    // static std::mt19937 gen(seed); // Seed the generator
    // static std::uniform_real_distribution<> dis(0.0, 1.0);
    unsigned int seed = 199;
    static random_device rd;
    // static mt19937 gen(rd());
    static mt19937 gen(seed);

    static uniform_real_distribution<> dis(0.0, 1.0);

    double randn = dis(gen);
    // cout << "Random number: " << randn << endl;
    return randn < probability;
}

// Function to convert binary to decimal
int b_d(string str, bool sign)
{
    bool f = false;
    int ans = 0;
    for (int i = 0; i < str.length(); i++)
    {
        ans = ans * 2 + str[i] - '0';
        if (i == 0 && str[i] == '1')
            f = true;
    }
    if (f && sign)
        ans = -(ans);
    return ans;
}

void print();

// Function to handle branch hazards and flush pipeline
void flush()
{
    ifid_input = true;
    pc = exepc + 4;
    if (simulateHazard(LOAD_USE_HAZARD_PROB))
    {
        // cout << "Load-Use Hazard Triggered" << endl;
        load_use_hazard_cycles++;
        lwhz = true;
    }
    if (instructions[(pc / 4)] == "")
    {
        finstruction = "00000000000000000000000000000000";
        ifid_input = false;
    }
    else
    {
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
    // print();
}

// Function to detect hazards and perform forwarding
void detect()
{
    if (exert == rs && exert != 0 && exesignal[3] == '1')
        readdata1 = aluout;
    if (exert == rt && exert != 0 && exesignal[3] == '1')
        readdata2 = aluout;

    if (memrt == rs && memrt != 0 && memsignal[0] == '1')
        readdata1 = (memsignal[1] == '1') ? memreaddata : memalu;
    if (memrt == rt && memrt != 0 && memsignal[0] == '1')
        readdata2 = (memsignal[1] == '1') ? memreaddata : memalu;

    if (idexsignal[5] == '1' && (rt == b_d(finstruction.substr(6, 5), false) || rt == b_d(finstruction.substr(11, 5), false)))
    {
        lwhz = true;
        pc -= 4;
        ifid_input = true;
    }
}

// Function to perform writeback stage
void writeback(string wbsignal, int rt, int alu, int readata)
{
    if (rt == 0)
        return;
    if (wbsignal[0] == '1')
    {
        if (wbsignal[1] == '0')
            reg[rt] = alu;
        else if (wbsignal[1] == '1')
            reg[rt] = readata;
    }
    else if (wbsignal[0] == '0')
        return;
}

// Function to perform instruction decode stage
void instructiondecode()
{
    idex_input=ifid_input;
    ifid_input=false;
    idpc=pc;
    op=finstruction.substr(0,6);//first 6 bits
    if(op=="000000")//r type
    {
        rs=b_d(finstruction.substr(6,5),false);
        rt=b_d(finstruction.substr(11,5),false);
        rd=b_d(finstruction.substr(16,5),false);
        fun=finstruction.substr(26);
        readdata1=reg[rs];
        readdata2=reg[rt];
        //r type no singextend
        signextend=b_d(finstruction.substr(16,16),true);
        if(rs==0&&rt==0&&rd==0&&fun=="000000"||lwhz==true)//no instruction
        {
            idexsignal="000000000";
            lwhz=false;

        }
        else
        {
            idexsignal="110000010";
        }

    }
    else//i type
    {
        rs=b_d(finstruction.substr(6,5),false);
        rt=b_d(finstruction.substr(11,5),false);
        readdata1=reg[rs];
        readdata2=reg[rt];
        signextend=b_d(finstruction.substr(16,16),true);
        //i type no
        rd=0;
        if(op=="100011")//lw
        {
            idexsignal="000101011";
        }
        else if(op=="101011")//sw
        {
            idexsignal="100100101";
        }
        else if(op=="001000")//addi
        {
            idexsignal="000100010";
        }
        else if(op=="001100")//andi
        {
            idexsignal="011100010";
        }
        else if(op=="000101")//bne
        {
            idexsignal="101010001";
        }
    }
}

// Function to perform execution stage
void execution()
{
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
    if (idexsignal[3] == '0')//alusrc== 0
        temp2 = readdata2;
    else //alusrc==0
        temp2 = signextend;
    if (aluop == "10")//r type
    {
        if (fun == "100000")//add
            aluout = temp + temp2;
        else if (fun == "100100")//and
            aluout = temp & temp2;
        else if (fun == "100101")//or
            aluout = temp | temp2;
        else if (fun == "100010")//sub
            aluout = temp - temp2;
        else if (fun == "101010")//slt
        {
            if (temp < temp2)
                aluout = 1;
            else
                aluout = 0;
        }
        else if (fun == "000000")
            aluout = 0;
    }
    else if (aluop == "00")//lw&sw
        aluout = temp + temp2;
    else if (aluop == "01")//bNeq
    {
        aluout = temp - temp2;
        bneq = true;
        if (aluout != 0)
            flu = true;
    }
    else if (idexsignal == "000100010")
    {
        aluout = temp + temp2;
    }
    else if (idexsignal == "011100010")
    {
        aluout = temp & temp2;
    }
    exepc = idpc + (signextend * 4);
}

// Function to perform memory access stage
void memoryaccess()
{
    meminput = exe_input;
    exe_input = false;
    membranch = false;
    memrt = exert;
    memalu = aluout;
    memsignal = exesignal.substr(3, 2);
    if (simulateHazard(BRANCH_HAZARD_PROB))
    {
        branch_hazard_cycles++;
        // cout << "Branch Hazard Triggered" << endl;
        bneq = true;
    }
    if (memsignal == "01" && memalu != 0 && bneq == true)
    {
        bneq = false;
        membranch = true;
    }
    if (memsignal == "11")
    {
        memreaddata = mem[memalu / 4];
    }
    else
    {
        memreaddata = 0;
    }
    if (memsignal == "01" && exesignal[2] == '1')
    {
        mem[memalu / 4] = writedata;
    }
}

// Function to perform instruction fetch stage
void instructionfetch()
{
    pc=pc+4;
    ifid_input=true;
    if(instructions[(pc/4)]=="")// pc/4= the number of instruction
    {
        finstruction="00000000000000000000000000000000";//no instruction
        ifid_input=false;
    }
    else
    {
        finstruction=instructions[(pc/4)];//fetch the instruction
    }
}

// Function to execute the pipeline stages
bool gocycling()
{
    wbrd = memrt;
    writeback(memsignal, memrt, memalu, memreaddata);
    memoryaccess();
    execution();
    instructiondecode();
    instructionfetch();
    clockcycle++;
    wbrd = memrt;

    if (flu == true)
    {
        flu = false;
        flush();
    }
    else
    {
        // print();
    }
    detect();

    if ((memsignal == "00" && memalu == 0 && memreaddata == 0) && exesignal == "00000" && idexsignal == "000000000" && clockcycle != 1)
    {
        return false;
    }
    else
        return true;
}

// Function to print the pipeline state
void print()
{
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
    if (lwhz)
    {
        fout << "Load-Use Hazard Triggered" << endl;
    }
    if (bneq)
    {
        fout << "Branch Hazard Triggered" << endl;
    }
    if (flu)
    {
        fout << "Forwarding Unit Used" << endl;
    }

    fout << "=================================================================" << endl;
}
void set_registers_and_memory()
{
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
    pc = 0;
    ifid_input = false;
    
}

// Function to run the Monte Carlo simulation
void runMonteCarloSimulation(int numSimulations, double loadUseHazardProb, double branchHazardProb)
{
    vector<double> performanceImpacts;
    random_device rd;
    mt19937 gen(rd());

    for (int i = 0; i < numSimulations; i++)
    {
        // Reset the pipeline state and hazard counters
        pc = 0;
        ifid_input = false;
        finstruction = "00000000000000000000000000000000";
        idpc = 0;
        op = "000000";
        fun = "000000";
        idexsignal = "000000000";
        idex_input = 0;
        readdata1 = 0;
        readdata2 = 0;
        signextend = 0;
        int rd = 0;
        rs = 0;
        rt = 0;
        rd = 0;
        exe_input = false;
        aluout = 0;
        writedata = 0;
        exert = 0;
        exepc = 0;
        temp = 0;
        temp2 = 0;
        exesignal = "000000000";
        aluop = "00";
        meminput = false;
        membranch = false;
        memreaddata = 0;
        memalu = 0;
        memrt = 0;
        memsignal = "00";
        wbrd = 0;
        lwhz = false;
        bneq = false;
        flu = false;
        clockcycle = 0;
        load_use_hazard_cycles = 0;
        branch_hazard_cycles = 0;
        total_hazard_cycles = 0;
        performance_impact = 0.0;
        set_registers_and_memory();

        // Set the hazard probabilities for this simulation
        LOAD_USE_HAZARD_PROB = loadUseHazardProb;
        BRANCH_HAZARD_PROB = branchHazardProb;

        // Run the pipeline simulation
        while (gocycling() == true)
            ;

        // Calculate the performance impact for this simulation
        total_hazard_cycles = load_use_hazard_cycles + branch_hazard_cycles;
        performance_impact = (double)total_hazard_cycles / (double)clockcycle * 100;

        // Store the performance impact in the vector
        performanceImpacts.push_back(performance_impact);
    }

    // Calculate the average performance impact
    double averagePerformanceImpact = accumulate(performanceImpacts.begin(), performanceImpacts.end(), 0.0) / numSimulations;

    // Find the minimum and maximum performance impacts
    auto minMax = minmax_element(performanceImpacts.begin(), performanceImpacts.end());
    double minPerformanceImpact = *minMax.first;
    double maxPerformanceImpact = *minMax.second;

    cout << "Hazard Probabilities: Load-Use=" << loadUseHazardProb << ", Branch=" << branchHazardProb << endl;
    // Print the results
    cout << "Average performance impact due to hazards (%): " << averagePerformanceImpact << endl;
    cout << "Minimum performance impact due to hazards (%): " << minPerformanceImpact << endl;
    cout << "Maximum performance impact due to hazards (%): " << maxPerformanceImpact << endl;
    cout << "=================================================================" << endl;
}

int main()
{
  
    ifstream fin("Input.txt");
    int i = 1;
    while (fin >> instructions[i])
        i++;

    // Run the Monte Carlo simulation with different hazard probabilities
    runMonteCarloSimulation(1000, 0.2, 0.2); // Low hazard probabilities
    runMonteCarloSimulation(1000, 0.5, 0.5); // Medium hazard probabilities
    runMonteCarloSimulation(1000, 0.8, 0.8); // High hazard probabilities
    runMonteCarloSimulation(1000, 0.9, 0.01); // Low hazard probabilities
    runMonteCarloSimulation(1000, 0.01, 0.01); // Medium hazard probabilities

    fin.close();
    fout.close();

    return 0;
}