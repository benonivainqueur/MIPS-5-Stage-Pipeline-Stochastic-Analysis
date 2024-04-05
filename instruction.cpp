#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <bitset>

using namespace std;

// Define MIPS opcodes
#define LW_OPCODE "100011"
#define SW_OPCODE "101011"
#define ADDI_OPCODE "001000"
#define ANDI_OPCODE "001100"
#define BNE_OPCODE "000101"
#define ADD_OPCODE "000000"  
#define SUB_OPCODE "000000"

const string opcodes[] = {LW_OPCODE, SW_OPCODE, ADDI_OPCODE, ANDI_OPCODE, BNE_OPCODE, ADD_OPCODE, SUB_OPCODE};

// Define available memory and register capacity
#define MEM_CAPACITY 1024 // Example memory capacity
#define REG_CAPACITY 32   // Example register capacity

// Function to generate random instruction
string generateRandomInstruction() {
    // Generate random opcode index
    int opcodeIndex = rand() % (sizeof(opcodes) / sizeof(opcodes[0]));

    // Generate random register and memory indices
    int rs = rand() % REG_CAPACITY;
    int rt = rand() % REG_CAPACITY;
    int rd = rand() % REG_CAPACITY;
    int memoryIndex = rand() % MEM_CAPACITY;

    // Construct instruction based on opcode and indices
    string instruction = opcodes[opcodeIndex];

    if (opcodes[opcodeIndex] == ADD_OPCODE || opcodes[opcodeIndex] == SUB_OPCODE) {
        // For ADD and SUB, use the same format but set different function codes
        string funct;
        if (opcodes[opcodeIndex] == ADD_OPCODE)
            funct = "100000"; // Add function code
        else if (opcodes[opcodeIndex] == SUB_OPCODE)
            funct = "100010"; // Subtract function code

        instruction += bitset<5>(rs).to_string();       // Add rs
        instruction += bitset<5>(rt).to_string();       // Add rt
        instruction += bitset<5>(rd).to_string();       // Add rd
        instruction += "00000";                         // Shamt (unused for ADD and SUB)
        instruction += funct;                           // Add function code
    } else {
        // For other instructions, use the existing format
        instruction += bitset<5>(rs).to_string();       // Add rs
        instruction += bitset<5>(rt).to_string();       // Add rt
        instruction += bitset<16>(memoryIndex).to_string(); // Add memory index
    }

    return instruction;
}

int main() {
    // Seed random number generator
    srand(time(0));

    // Generate and output random instructions
    const int NUM_INSTRUCTIONS = 100; // Number of instructions to generate
    ofstream outFile("machine_code_instructions.txt");
    if (outFile.is_open()) {
        for (int i = 0; i < NUM_INSTRUCTIONS; ++i) {
            string instruction = generateRandomInstruction();
            outFile << instruction << endl;
        }
        outFile.close();
        cout << "Random machine code instructions generated and saved to machine_code_instructions.txt" << endl;
    } else {
        cerr << "Error: Unable to open file for writing." << endl;
        return 1;
    }

    return 0;
}
