#include <stdio.h>  // for stderr
#include <stdlib.h> // for exit()
#include "types.h"
#include "utils.h"
#include "riscv.h"

void execute_rtype(Instruction, Processor *);
void execute_itype_except_load(Instruction, Processor *);
void execute_branch(Instruction, Processor *);
void execute_jal(Instruction, Processor *);
void execute_load(Instruction, Processor *, Byte *);
void execute_store(Instruction, Processor *, Byte *);
void execute_ecall(Processor *, Byte *);
void execute_lui(Instruction, Processor *);

void execute_instruction(uint32_t instruction_bits, Processor *processor, Byte *memory)
{
    Instruction instruction = parse_instruction(instruction_bits);
    switch (instruction.opcode)
    {
    case 0x33:
        execute_rtype(instruction, processor);
        break;
    case 0x13:
        execute_itype_except_load(instruction, processor);
        break;
    case 0x73:
        execute_ecall(processor, memory);
        break;
    case 0x63:
        execute_branch(instruction, processor);
        break;
    case 0x6F:
        execute_jal(instruction, processor);
        break;
    case 0x23:
        execute_store(instruction, processor, memory);
        break;
    case 0x03:
        execute_load(instruction, processor, memory);
        break;
    case 0x37:
        execute_lui(instruction, processor);
        break;
    default: // undefined opcode
        handle_invalid_instruction(instruction);
        exit(-1);
        break;
    }
}

void execute_rtype(Instruction instruction, Processor *processor)
{
    switch (instruction.rtype.funct3)
    {
    case 0x0:
        switch (instruction.rtype.funct7)
        {
        case 0x0:
            // Add
            processor->R[instruction.rtype.rd] =
                ((sWord)processor->R[instruction.rtype.rs1]) +
                ((sWord)processor->R[instruction.rtype.rs2]);

            processor->PC += 4;

            break;
        case 0x1:
            // Mul
            processor->R[instruction.rtype.rd] =
                ((sWord)processor->R[instruction.rtype.rs1]) *
                ((sWord)processor->R[instruction.rtype.rs2]);

            processor->PC += 4;
            break;

        case 0x20:
            // Sub
            processor->R[instruction.rtype.rd] =
                ((sWord)processor->R[instruction.rtype.rs1]) -
                ((sWord)processor->R[instruction.rtype.rs2]);

            processor->PC += 4;

            break;

        default:
            handle_invalid_instruction(instruction);
            exit(-1);
            break;
        }
        break;
    case 0x1:
        switch (instruction.rtype.funct7)
        {
        case 0x0:
            // SLL
            //     sWord shift = 0x00000000;
            //    shift |= ((sWord)processor->R[instruction.rtype.rs2]) & 0x0000001F;
            // processor->R[instruction.rtype.rd] =
            //     ((sWord)processor->R[instruction.rtype.rs1]) << ((((sWord)processor->R[instruction.rtype.rs2]) & 0x0000001F)| 0x00000000);
            processor->R[instruction.rtype.rd] = (processor->R[instruction.rtype.rs1]) << (processor->R[instruction.rtype.rs2]);
            processor->PC += 4;

            break;
        case 0x1:;
            // MULH
            sDouble result = (((sDouble)instruction.rtype.rs1) * ((sDouble)instruction.rtype.rs2)) >> 32;
            // sDouble result = 0x0000000000000000, t1 = 0x0000000000000000, t2 = 0x00000000;
            // result |= (((sDouble)processor->R[instruction.rtype.rs1]) | t1) * (((sDouble)processor->R[instruction.rtype.rs1]) | t1);
            // t2 |= ((result & 0xFFFFFFFF00000000) >> 32);
            processor->R[instruction.rtype.rd] = result;
            processor->PC += 4;

            break;
        case 0x2:
            // SLT
            processor->R[instruction.rtype.rd] =
                (((sWord)processor->R[instruction.rtype.rs1]) <
                 ((sWord)processor->R[instruction.rtype.rs2]))
                    ? 1
                    : 0;
            processor->PC += 4;
            break;
        case 0x4:
            switch (instruction.rtype.funct7)
            {
            case 0x0:
                // XOR
                processor->R[instruction.rtype.rd] =
                    (sWord)(((sWord)processor->R[instruction.rtype.rs1]) ^
                            ((sWord)processor->R[instruction.rtype.rs2]));
                processor->PC += 4;

                break;
            case 0x1:
                // DIV
                processor->R[instruction.rtype.rd] =
                    (((sWord)processor->R[instruction.rtype.rs1]) /
                     ((sWord)processor->R[instruction.rtype.rs2]));
                processor->PC += 4;
                break;
            default:
                handle_invalid_instruction(instruction);
                exit(-1);
                break;
            }
            break;
        case 0x5:
            switch (instruction.rtype.funct7)
            {
            case 0x0:
                // SRL
                processor->R[instruction.rtype.rd] =
                    ((Word)processor->R[instruction.rtype.rs1]) >>
                    (((Word)processor->R[instruction.rtype.rs2]) & 0x1F);
                processor->PC += 4;

                break;
            case 0x20:
                // SRA
                processor->R[instruction.rtype.rd] =
                    ((sWord)processor->R[instruction.rtype.rs1]) >>
                    (((sWord)processor->R[instruction.rtype.rs2]) & 0x1F);
                processor->PC += 4;

                break;
            default:
                handle_invalid_instruction(instruction);
                exit(-1);
                break;
            }
            break;
        case 0x6:
            switch (instruction.rtype.funct7)
            {
            case 0x0:
                // OR
                processor->R[instruction.rtype.rd] =
                    (((sWord)processor->R[instruction.rtype.rs1]) |
                     ((sWord)processor->R[instruction.rtype.rs2]));
                processor->PC += 4;
                break;
            case 0x1:
                // REM
                processor->R[instruction.rtype.rd] =
                    (((sWord)processor->R[instruction.rtype.rs1]) %
                     ((sWord)processor->R[instruction.rtype.rs2]));
                processor->PC += 4;
                break;
            default:
                handle_invalid_instruction(instruction);
                exit(-1);
                break;
            }
            break;
        case 0x7:
            // AND
            processor->R[instruction.rtype.rd] =
                (((sWord)processor->R[instruction.rtype.rs1]) &
                 ((sWord)processor->R[instruction.rtype.rs2]));
            processor->PC += 4;
            break;
        default:
            handle_invalid_instruction(instruction);
            exit(-1);
            break;
        }
    }
    // processor->PC += 4;
}

void execute_itype_except_load(Instruction instruction, Processor *processor)
{
    switch (instruction.itype.funct3)
    {
    case 0x0:
        // ADDI
        processor->R[instruction.itype.rd] =
            ((sWord)(processor->R[instruction.itype.rs1])) +
            sign_extend_number((instruction.itype.imm), 12);
        processor->PC += 4;

        // processor->R[instruction.itype.rd] = 0xAAAAAAA;

        break;
    case 0x1:
        // SLLI
        processor->R[instruction.itype.rd] =
            // ((sWord)processor->R[instruction.itype.rs1]) << ((sWord)processor->R[instruction.itype.imm]); ////////////
            ((sWord)processor->R[instruction.itype.rs1]) << ((sWord)sign_extend_number(instruction.itype.imm, 12));
        processor->PC += 4;

        break;
    case 0x2:
        // STLI
        if ((sWord)(processor->R[instruction.itype.rs1]) < (sWord)(instruction.itype.imm))
        {
            processor->R[instruction.itype.rd] = 0x00000001;
        }
        else
        {
            processor->R[instruction.itype.rd] = 0x00000000;
        }
        processor->PC += 4;
        // processor->R[instruction.itype.rd] = ((sWord)(instruction.itype.rs1) <
        //                                       (sWord)(instruction.itype.imm))? 1 : 0;
        break;
    case 0x4:
        // XORI
        processor->R[instruction.itype.rd] = (sWord)(processor->R[instruction.itype.rs1]) ^
                                             (sWord)sign_extend_number(instruction.itype.imm, 12);
        processor->PC += 4;

        break;
    case 0x5:;
        // Shift Right (You must handle both logical and arithmetic)
        int shift = 0x00000000, temp = 0x00000000;
        temp |= ((instruction.itype.imm) & 0xFE0) >> 5;
        shift |= (instruction.itype.imm) & 0x01F;
        if ((sWord)temp == 0x00000000)
        {
            processor->R[instruction.itype.rd] = ((Word)processor->R[instruction.itype.rs1]) >> shift;
        }
        else
        {
            processor->R[instruction.itype.rd] = ((Word)processor->R[instruction.itype.rs1]) >> shift;
        }
        processor->PC += 4;
        break;

    case 0x6:
        // ORI
        processor->R[instruction.itype.rd] =
            (sWord)processor->R[instruction.itype.rs1] |
            (sWord)processor->R[instruction.itype.imm] | 0x00000000;
        processor->PC += 4;

        break;
    case 0x7:
        // ANDI
        processor->R[instruction.itype.rd] =
            ((sWord)processor->R[instruction.itype.rs1]) &
            (sign_extend_number((instruction.itype.imm), 12));
        processor->PC += 4;

        break;
    default:
        handle_invalid_instruction(instruction);
        break;
    }
}

void execute_ecall(Processor *p, Byte *memory)
{
    Register i;

    // syscall number is given by a0 (x10)
    // argument is given by a1
    switch (p->R[10])
    {
    case 1: // print an integer
        printf("%d", p->R[11]);
        break;
    case 4: // print a string
        for (i = p->R[11]; i < MEMORY_SPACE && load(memory, i, LENGTH_BYTE); i++)
        {
            printf("%c", load(memory, i, LENGTH_BYTE));
        }
        break;
    case 10: // exit
        printf("exiting the simulator\n");
        exit(0);
        break;
    case 11: // print a character
        printf("%c", p->R[11]);
        break;
    default: // undefined ecall
        printf("Illegal ecall number %d\n", p->R[10]);
        exit(-1);
        break;
    }
}

void execute_branch(Instruction instruction, Processor *processor)
{
    switch (instruction.sbtype.funct3)
    {
    case 0x0:
        // BEQ
        if ((sWord)processor->R[instruction.sbtype.rs1] == (sWord)processor->R[instruction.sbtype.rs2])
        {
            processor->PC += (sWord)get_branch_offset(instruction);
        }
        else
        {
            processor->PC += 4;
        }

        break;
    case 0x1:
        // BNE
        if ((sWord)processor->R[instruction.sbtype.rs1] != (sWord)processor->R[instruction.sbtype.rs2])
        {
            processor->PC += (sWord)get_branch_offset(instruction);
        }
        else
        {
            processor->PC += 4;
        }
        break;
    default:
        handle_invalid_instruction(instruction);
        exit(-1);
        break;
    }
}

void execute_load(Instruction instruction, Processor *processor, Byte *memory)
{
    switch (instruction.itype.funct3)
    {
    case 0x0:
        // LB
        processor->R[instruction.itype.rd] = load(memory,
                                                  (sWord)(processor->R[instruction.itype.rs1]) + (sWord)sign_extend_number(instruction.itype.imm, 12),
                                                  LENGTH_BYTE);

        processor->PC += 4;

        break;
    case 0x1:
        // LH

        processor->R[instruction.itype.rd] = load(memory,
                                                  (sWord)(processor->R[instruction.itype.rs1]) + (sWord)sign_extend_number(instruction.itype.imm, 12),
                                                  LENGTH_HALF_WORD);

        processor->PC += 4;

        break;
    case 0x2:
        // LW

        processor->R[instruction.itype.rd] = load(memory,
                                                  (sWord)(processor->R[instruction.itype.rs1]) + (sWord)sign_extend_number(instruction.itype.imm, 12),
                                                  LENGTH_WORD);

        processor->PC += 4;

        break;
    default:
        handle_invalid_instruction(instruction);
        processor->PC += 4;
        break;
    }
}

void execute_store(Instruction instruction, Processor *processor, Byte *memory)
{
    switch (instruction.stype.funct3)
    {
    case 0x0:;
        // SB
        sWord temp = (sWord)(processor->R[instruction.stype.rs1]) + (sWord)get_store_offset(instruction);
        store(memory,
              temp,
              LENGTH_BYTE,
              (processor->R[instruction.stype.rs2]));
        processor->PC += 4;

        break;
    case 0x1:
        // SH
        store(memory,
              (sWord)(processor->R[instruction.stype.rs1]) + (sWord)get_store_offset(instruction),
              LENGTH_HALF_WORD,
              (processor->R[instruction.stype.rs2]));
        processor->PC += 4;
        break;
    case 0x2:
        // SW
        store(memory,
              (sWord)(processor->R[instruction.stype.rs1]) + (sWord)get_store_offset(instruction),
              LENGTH_WORD,
              (processor->R[instruction.stype.rs2]));
        processor->PC += 4;
        break;
    default:
        handle_invalid_instruction(instruction);
        exit(-1);
        break;
    }
}

void execute_jal(Instruction instruction, Processor *processor)
{
    processor->R[instruction.utype.rd] = processor->PC + 4;
    processor->PC += get_jump_offset(instruction);

    /* YOUR CODE HERE */
}

void execute_lui(Instruction instruction, Processor *processor)
{
    /* YOUR CODE HERE */
    // processor->R[instruction.utype.rd] = 0xbcd5b000;

    // processor->R[instruction.utype.rd] = (sWord)sign_extend_number(instruction.utype.imm,20) << 12;
    processor->R[instruction.utype.rd] = (sWord)sign_extend_number(instruction.utype.imm, 20) << 12;
    processor->PC += 4;
    // 10111100110101011011
    // 00000000000011101111111111111111
}

void store(Byte *memory, Address address, Alignment alignment, Word value)
{
    /* YOUR CODE HERE */
    if (alignment == LENGTH_BYTE)
    {
        memory[address] = (Byte)(value & 0x000000FF);
        return;
    }
    else if (alignment == LENGTH_HALF_WORD || alignment == LENGTH_WORD)
    {

        memory[address] = (Byte)(value & 0x000000FF);
        memory[address + 1] = (Byte)((value & 0x0000FF00) >> 8);
        if (alignment == LENGTH_HALF_WORD)
        {
            return;
        }
        memory[address + 2] = (Byte)((value & 0x0000FF00) >> 16);
        memory[address + 3] = (Byte)((value & 0x00FF0000) >> 24);
    }
    return;
}

Word load(Byte *memory, Address address, Alignment alignment)
{
    /* YOUR CODE HERE */
    Word result = 0x00000000;
    if (alignment == LENGTH_BYTE)
    {
        result |= memory[address];
        return result;
        // return (result | memory[address]);
    }
    else if (alignment == LENGTH_HALF_WORD || alignment == LENGTH_WORD)
    {
        // for(int i = 0; i < 4; i++){
        //     result = result | (memory[address+i]<<(i*8));
        //     if (alignment == LENGTH_HALF_WORD && i == 1){
        //         return result;
        //     }
        // }
        result |= memory[address];
        result |= (memory[address + 1] << 8);
        if (alignment == LENGTH_HALF_WORD)
        {
            return result;
        }
        result |= (memory[address + 2] << 16);
        result |= (memory[address + 3] << 24);

        return result;
    }

    return result;
}
