#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <bitset> // Include the <bitset> header for using bitset

using namespace std;

// Define MIPS opcode base
#define LW_OPCODE "100011"
#define SW_OPCODE "101011"
#define ADDI_OPCODE "001000"
#define ANDI_OPCODE "001100"
#define BNE_OPCODE "000101"

const string opcodes[] = {LW_OPCODE, SW_OPCODE, ADDI_OPCODE, ANDI_OPCODE, BNE_OPCODE};

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
    instruction += bitset<5>(rs).to_string();  // Add rs
    instruction += bitset<5>(rt).to_string();  // Add rt
    instruction += bitset<5>(rd).to_string();  // Add rd
    instruction += bitset<16>(memoryIndex).to_string();  // Add memory index

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