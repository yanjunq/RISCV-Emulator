#include "utils.h"
#include <stdio.h>
#include <stdlib.h>

//helper function for checking the binary
void print_binary(unsigned int number, int size) // Size is in bits
{
  if (!number)
  {
    putc('0', stdout);
    return;
  }
  if (size == 1)
  {
    putc((number & 1) ? '1' : '0', stdout);
    return;
  }
  print_binary(number >> 1, size - 1);
  putc((number & 1) ? '1' : '0', stdout);
}

/* Sign extends the given field to a 32-bit integer where field is
 * interpreted an n-bit integer. */
int sign_extend_number(unsigned int field, unsigned int n) {
  /* YOUR CODE HERE */
  int result = (int)field<<(32-n)>>(32-n);

  return result;
}

/* Unpacks the 32-bit machine code instruction given into the correct
 * type within the instruction struct */
Instruction parse_instruction(uint32_t instruction_bits) {
  /* YOUR CODE HERE */
  Instruction instruction;
  // add x8, x0, x0     hex : 00000433  binary = 0000 0000 0000 0000 0000 01000
  // Opcode: 0110011 (0x33) Get the Opcode by &ing 0x1111111, bottom 7 bits
  instruction.opcode = instruction_bits & ((1U << 7) - 1);

  // Shift right to move to pointer to interpret next fields in instruction.
  instruction_bits >>= 7;

  switch (instruction.opcode) {
  // R-Type
  case 0x33:
    // instruction: 0000 0000 0000 0000 0000 destination : 01000
    instruction.rtype.rd = instruction_bits & ((1U << 5) - 1);
    instruction_bits >>= 5;

    // instruction: 0000 0000 0000 0000 0 func3 : 000
    instruction.rtype.funct3 = instruction_bits & ((1U << 3) - 1);
    instruction_bits >>= 3;

    // instruction: 0000 0000 0000  src1: 00000
    instruction.rtype.rs1 = instruction_bits & ((1U << 5) - 1);
    instruction_bits >>= 5;

    // instruction: 0000 000        src2: 00000
    instruction.rtype.rs2 = instruction_bits & ((1U << 5) - 1);
    instruction_bits >>= 5;

    // funct7: 0000 000
    instruction.rtype.funct7 = instruction_bits & ((1U << 7) - 1);
    break;
  // case for I-type 
  case 0x13:
  case 0x03:
  case 0x73:
    
    instruction.itype.rd = (instruction_bits & 0x1F);
    // instruction_bits >>= 5;

    instruction.itype.funct3 = ((instruction_bits>>5) & 0x07);
    // instruction_bits >>= 3;

    instruction.itype.rs1 = ((instruction_bits>>8)&0x1F);
    // instruction_bits >>= 5;

    instruction.itype.imm = ((instruction_bits>>13) & 0xFFF);
    // instruction.itype.imm = 0;
    break;

    //case for S-type
    case 0x23:
    
     instruction.stype.imm5 = (instruction_bits & 0x1F);
    //  instruction_bits >>= 5;

     instruction.stype.funct3 = ((instruction_bits>>5) & 0x07);
    //  instruction_bits >>= 3;

     instruction.stype.rs1 = ((instruction_bits>>8) & 0x1F);
    //  instruction_bits >>= 5;

     instruction.stype.rs2 = ((instruction_bits>>13) & 0x1F);
    //  instruction_bits >>= 5;

    instruction.stype.imm7 = ((instruction_bits>>18) & 0x7F);
    break;

    //case for B-type
    case 0x63:
     instruction.sbtype.imm5 = instruction_bits & (0x1F);
    //  instruction_bits >>= 5;

     instruction.sbtype.funct3 = ((instruction_bits>>5) & 0x07);
    //  instruction_bits >>= 3;

     instruction.sbtype.rs1 = ((instruction_bits>>8) & 0x1F);
    //  instruction_bits >>= 5;

     instruction.sbtype.rs2 = ((instruction_bits>>13) & 0x1F);
    //  instruction_bits >>= 5;

     instruction.sbtype.imm7 = ((instruction_bits>>18) & 0x7F);

    break;

    // //case for J-type
    case 0x6F:
     instruction.ujtype.rd = instruction_bits & (0x1F);
     instruction.ujtype.imm = (instruction_bits>>5) & (0xFFFFF);
   
     
    break;

    // //case for U-type
    case 0x37: 
    instruction.utype.rd = instruction_bits & (0x1F);
    instruction.utype.imm = (instruction_bits>>5) & (0xFFFFF);

    break;

  default:
    exit(EXIT_FAILURE);
  }
  return instruction;
}

/* Return the number of bytes (from the current PC) to the branch label using
 * the given branch instruction */
int get_branch_offset(Instruction instruction) {
  /* YOUR CODE HERE */
  int result = 0x000000000;
  int last = (instruction.sbtype.imm5 & 0x1)<<12;
  int sign = (instruction.sbtype.imm7 & 0x800)<<1;
  int t1 = (instruction.sbtype.imm5) & 0x1E;
  int t2 = (instruction.sbtype.imm7 & 0x7F)<<5;

  result |= t1|t2|last|sign; 


  return sign_extend_number(result,13);


}

/* Returns the number of bytes (from the current PC) to the jump label using the
 * given jump instruction */
int get_jump_offset(Instruction instruction) {
  /* YOUR CODE HERE */
  int result = 0x00000000;
  int t1 = (instruction.ujtype.imm & 0xFF)<<12;
  int t2 = ((instruction.ujtype.imm & 0x100)>>8)<<11;
  int t3 = (instruction.ujtype.imm & 0x7FE00)>>8;
  int sign = (instruction.ujtype.imm & 0x80000)<<1;
  result|= t1|t2|t3|sign;

  return sign_extend_number(result, 21);
}

int get_store_offset(Instruction instruction) {
  /* YOUR CODE HERE */
  int result = 0x00000000;
  int t1 = instruction.stype.imm5 & 0x0000001F;
  int t2 = ((instruction.stype.imm7) << 5) & 0x00000FE0;
  result |= t1 | t2;

  return sign_extend_number(result,12);
}


void handle_invalid_instruction(Instruction instruction) {
  printf("Invalid Instruction: 0x%08x\n", instruction.bits);
}

void handle_invalid_read(Address address) {
  printf("Bad Read. Address: 0x%08x\n", address);
  exit(-1);
}

void handle_invalid_write(Address address) {
  printf("Bad Write. Address: 0x%08x\n", address);
  exit(-1);
}
