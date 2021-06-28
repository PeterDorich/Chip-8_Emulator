/****************************************************************************
 * Program: Chip-8 Emulator
 * Author: Peter Dorich
  
 * File: chip8.cpp
 * Contains implementation for all opcodes for the chip-8 system.
 * I included a short description for each opcode from the wikapedia Chip-8
 * documentation.
****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <time.h>

#include "chip8.h"

//Used to represent a hex sprite to the display
unsigned char chip8_fontset[80] = 
{
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

Chip8::Chip8()
{

}
Chip8::~Chip8()
{

}

//Initialize Chip-8
void Chip8::init()
{
    pc = 0x200;             //Program counter starts at 0x200
    opcode = 0;
    I = 0;
    sp = 0;

    //Clear display
    for(int i = 0; i < 2048; i++)
    {
        gfx[i] = 0;
    }
  
    //Clear registers, keys, and stack. All are arrays of size 16
    for(int i = 0; i < 16; i++)
    {
        V[i] = 0;
        key[i] = 0;
        stack[i] = 0;
    }

    //Clear Memory
    for(int i = 0; i < 4096; i++)
    {
        memory[i] = 0;
    }

    //Load fontset
    for(int i = 0; i < 80; i++)
    {
        memory[i] = chip8_fontset[i];
    }

    //Reset the delay and sound timer:
    delay_timer = 0;
    sound_timer = 0;

    draw_flag = true;

    //Initialize random seed for 0xC000
    srand(time(NULL));
}


//Loads rom:
bool Chip8::load(const char *file_path)
{
    //Initialise
    init();

    printf("Loading ROM: %s\n", file_path);

    //Open ROM file with file poitners
    FILE* rom = fopen(file_path, "rb");
    if (rom == NULL) {
        printf("Could not open ROM.");
        return false;
    }

    // Get file size, jump to end and get the position
    fseek(rom, 0, SEEK_END);
    long rom_size = ftell(rom);
    //go back to beginning of file
    rewind(rom);

    //Dynamically allocate memory to store the rom. 
    char* buffer = (char*) malloc(sizeof(char) * rom_size);
    if (buffer == NULL) {
        printf("Could not allocate memory");
        return false;
    }

    //Copy ROM into buffer
    size_t result = fread(buffer, sizeof(char), (size_t)rom_size, rom);
    if (result != rom_size) {
        printf("ERROR");
        return false;
    }

    //Copy rom into the Chip8 memory, starting at 0x200, or 512
    if ((4096-512) > rom_size){
        for (int i = 0; i < rom_size; i++) {
            memory[i + 512] = (uint8_t)buffer[i];    
        }
    }
    else {
        printf("ROM too large to fit in memory.");
        return false;
    }

    //Close the rom file and free the buffer. 
    fclose(rom);
    free(buffer);


    return true;
}


//Emulates a single Chip-8 cycle by fetching, decoding, and executing opcodes.
void Chip8::emulateCycle()
{
    /*FETCH
        opcode is 2 bytes, so we must shift left by 8 bits and merge
        with the next portion of the opcode using bitwise 
    */
    opcode = memory[pc] << 8 | memory[pc + 1];


    //Decode: 
    //Opcode implementation
    switch(opcode & 0xF000)
    {

        
        case 0x0000:
            switch(opcode & 0x000F)
            {
                case 0x0000:    //0000: Clears the screen. Sets draw flag.
                    for(int i = 0; i < 2048; i++)
                    {
                        gfx[i] = 0;
                    }
                    draw_flag = true;
                    pc += 2;
                    break;
                
                case 0x000E:    //00EE: Returns from a subroutine. (set pc to previous value from stack)
                    pc = stack[--sp];
                    pc += 2;
                    break;

                default: 
                    printf("Opcode not found: 0x%X\n", opcode);
            }
            break;
        
        case 0x1000:     //1NNN: Jumps to address "NNN"
            pc = opcode & 0x0FFF;
            break;

        case 0x2000:    //2NNN: Calls subroutine at "NNN"
            stack[sp] = pc;
            ++sp;
            pc = opcode & 0x0FFF;
            break;

        case 0x3000:    //3XNNN: Skips next instruction if VX == NN
            if(V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF))
            {
                pc += 4; //+2 for current instruction and +2 to skip next instruction
            }
            else
            {
                pc += 2;
            }
            break;

        case 0x4000:    //4XNN: Skips next instruction if Vx != NN
            if(V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
            {
                pc += 4;
            }
            else
            {
                pc += 2;
            }
            break;

        case 0x5000:    //5XYN: Skips next instruction if VX == VY
            if(V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4])
            {
                pc += 4;
            }
            else
            {
                pc += 2;
            }
            break;

        case 0x6000:    //6XNN: Sets VX to NN
            V[(opcode & 0x0F00) >> 8] = (opcode & 0x00FF);
            pc += 2;
            break;

        case 0x7000:    //Adds NN to VX
            V[(opcode & 0x0F00) >> 8] += (opcode & 0x00FF);
            pc += 2;
            break;

        case 0x8000:
            switch(opcode & 0x000F)
            {
                case 0x0000:    //8XY0: Sets VX to value of VY
                    V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;

                case 0x0001:    //8XY1: Sets VX to VX or VY
                    V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] | V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;

                case 0x0002:    //8XY2: Sets VX to VX and VY
                    V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] & V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;

                case 0x0003:    //8XY3: Sets VX to VX xor VY
                    V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] ^ V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;

                case 0x0004:    //8XY4: Adds VY to VX. VF is set to 1 when carry is set, 0 if not 
                                //      Carry is set if VY is greater than VX.
                    if(V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode & 0x0F00) >> 8]))
                    {
                        V[15] = 1; //Carry Flag is the last register
                    }
                    else
                    {
                        V[15] = 0;  
                    }
                    V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;

                case 0x0005:    //8XY5: VY is subtracted from VX. VF is set to 0 when theres a borrow, 1 if not
                    if(V[(opcode & 0x00F0) >> 4] > V[(opcode & 0x0F00) >> 8])
                    {
                        V[15] = 0; //VY is bigger than VX, so there is a borrow
                    }
                    else
                    {
                        V[15] = 1;
                    }
                    V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;

                //8XY6: Stores the least signigicant bit of VX in VF,
                //      and shift VX to the right by 1
                case 0x0006:
                    V[15] = V[(opcode & 0x0F00) >> 8] & 1;      //set VF to VX Least significant digit, using bitwise &.
                    V[(opcode & 0x0F00) >> 8] >>= 1;            //Shift VX to the right by 1;
                    pc += 2;
                    break;

                case 0x0007:    //8XY7: Sets VX to VY minus VX. VF is set to 0 when theres a borrow, 1 when not.
                    if(V[(opcode & 0x0F00) >> 8] > V[(opcode & 0x00F0) >> 4])
                    {
                        V[15] = 0;
                    }
                    else
                    {
                        V[15] = 1;
                    }
                    V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];
                    pc +=2;
                    break;

                //8XYE: Stores the most significant bit of VX in VF 
                //      and shift VX to the right by 1
                case 0x000E: 
                    V[15] = V[(opcode & 0x0F00) >> 8] >> 7;
                    V[(opcode & 0x0F00) >> 8] <<= 1;
                    pc += 2;
                    break;

                default:
                    printf("Opcode not found: 0x%X\n", opcode);
                    break;
            }
            break;

        case 0x9000:    //9XY0: Skips next instruction if VX != VY
            if(V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4])
            {
                pc += 4;
            }
            else
            {
                pc += 2;
            }
            break;

        case 0xA000:    //ANNN: Sets I to the address "NNN"
            I = opcode & 0x0FFF;
            pc += 2;
            break;

        case 0xB000:    //BNNN: Jumps to the address NNN plus V0
            pc = (opcode & 0x0FFF) + V[0];
            break;

        case 0xC000:
        {
            //CXNN: Sets VX to the result of bitwise op on a random number (0-255) and NN
        
            int rand_n = rand() % 256;                                    //Generates a random number between 0-255 using the random seed.
            V[(opcode & 0x0F00) >> 8] = rand_n & ((opcode & 0x00FF));     //Sets VX to bitwise op on rand and NN

            pc += 2;         
            break;
        }
        /*DXYN: Display function.
                Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N+1 pixels.
                Each row of 8 pixels is read as bit-coded starting from memory location I.
                I value does not change after the execution of this instruction. 
                VF is set to 1 if any screen pixels are flipped from set to unset when the sprite is drawn,
                and to 0 if that does not happen.
        */
        case 0xD000: 
        {
            //Fetch X and Y coordinates from sprite
            unsigned short X = V[(opcode & 0x0F00) >> 8];
            unsigned short Y = V[(opcode & 0x00F0) >> 4]; 
            unsigned short height = opcode & 0x000F;        //N
            unsigned short pixel;                           //1 bit pixel

            V[15] = 0;

            for(int yline = 0; yline < height; yline++)
            {
                pixel = memory[I + yline];
                for(int xline = 0; xline < 8; xline++)
                {
                    if((pixel & (0x80 >> xline)) != 0)
                    {
                        if(gfx[(X + xline + ((Y + yline) * 64))] == 1)
                        {
                            V[15] = 1;
                        }
                        //SEG FAULT WHEN SPRITE GOES OFF SCREEN.
                        //Sprite should wrap around to the other side: 
                        gfx[(X + xline + ((Y + yline) * 64)) % (64 * 32)] ^= 1;
                    }
                }
            }
            
            draw_flag = true;       //Screen needs to be updated.
            pc += 2;
        }
            break;
        
        case 0xE000:    
            switch(opcode & 0x00FF)
            {
                case 0x009E:    //EX9E: Skips next instruction if the key stored in VX is pressed
                    if(key[V[(opcode & 0x0F00) >> 8]] != 0)
                    {
                        pc += 4;
                    }
                    else
                    {
                        pc += 2;
                    }
                    break;

                case 0x00A1:    //EXA1: Skips the next instruction if the key stored in VX is no pressed
                    if(key[V[(opcode & 0x0F00) >> 8]] == 0)
                    {
                        pc += 4;
                    }
                    else
                    {
                        pc += 2;
                    }
                    break;

                default:
                    printf("Opcode not found: 0x%X\n", opcode);
            }

        case 0xF000: 
            switch(opcode & 0x00FF)
            {
                case 0x0007:    //FX07: Sets VX to the value of the delay timer
                    V[(opcode & 0x0F00) >> 8] = delay_timer;
                    pc += 2;
                    break;

                case 0x000A:    //FX0A: A key press is awaited, then stored in VX (Blocking operation)
                {
                    bool key_pressed = false;
                    for (int i = 0; i < 16; i++)
                    {
                        if(key[i] != 0)
                        {
                            V[(opcode & 0x0F00) >> 8] = i;
                            key_pressed = true;
                        }
                    }
                    if(!key_pressed)
                        return;
                    pc += 2;
                }
                    break;

                case 0x0015:    //FX15: Sets the delay timer to VX
                    delay_timer = V[(opcode & 0x0F00) >> 8];
                    pc += 2;
                    break;
                
                case 0x0018:    //FX18: sets the sound timer to VX
                    sound_timer = V[(opcode & 0x0F00) >> 8];
                    pc += 2;
                    break;
                
                case 0x001E:    //FX1E: Adds VX to I. VF not affected.
                    if(I + V[(opcode & 0x0F00) >> 8] > 0xFFF)
                        V[0xF] = 1;
                    else
                        V[0xF] = 0;
                    I += V[(opcode & 0x0F00) >> 8];
                    pc += 2;
                    break;
                
                //FX29: Sets I to the location of the sprite for the character in VX
                //      Characters 0-F are represented by a 4x5 font.
                case 0x0029:    
                    I = (V[(opcode & 0x0F00)>> 8] * 5);     //4x5 sprite
                    pc += 2;
                    break;

                /*FX33: Takes the decimal representation of VX:
                        memory address[I] = most significant digit,
                        memory address[I+1] = middle significant digit, 
                        memory address[I+2] = least significant digit. 
                */
                case 0x0033:
                    memory[I] = V[(opcode & 0x0F00) >> 8] / 100;    //divide by 100 for the most significant digit
                    memory[I+1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;    
                    memory[I+2] = (V[(opcode & 0x0F00) >> 8] % 10);
                    pc += 2;
                    break;

                /*FX55: Stores V0 to VX starting at address I. 
                        The offset from I is increased by 1 for each value written,
                        but I itself is left unmodified. 
                */ 
                case 0x0055:
                    for(int i = 0; i <= ((opcode & 0x0F00) >> 8); i++)
                    {
                        memory[I + i] = V[i];
                    }
                    I += ((opcode & 0x0F00) >> 8) + 1;
                    pc += 2;
                    break;

                /*FX65: Fills V0 to VX with values from memory starting at address I.
                        The offset from I is increased by 1 for each value written, 
                        but I itself is left unmodified. 
                */
                case 0x0065:   
                    for(int i = 0; i <= ((opcode & 0x0F00) >> 8); i++)
                    {
                        V[i] = memory[I + i];
                    }
                    I += ((opcode & 0x0F00) >> 8) + 1;
                    pc += 2;
                    break;

                default:
                    printf("Opcode not found: 0x%X\n", opcode);
                    break;
            }
            break;
        
        default: 
            printf("Opcode not found: 0x%X\n", opcode);
            break;
    }

    
    if(delay_timer > 0)
    {
        --delay_timer;
    }
    if(sound_timer > 0)
    {
        if(sound_timer == 1)
        {
            printf("Beep - sound not yet implemented\n");
        }
        --sound_timer;
    }
}






