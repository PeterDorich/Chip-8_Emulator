/****************************************************************************
 * Program: Chip-8 Emulator
 * Author: Peter Dorich
  
 * File: chip8.h
 * Header file for chip-8 class.
****************************************************************************/
#ifndef CHIP_8_H
#define CHIP_8_H
#include <stdint.h>

class Chip8 {
    private:

        unsigned short opcode;              //Currently stored operation code.

        unsigned char memory[4096];         //4k memory for a chip-8 emulated as an array.
        unsigned char V[16];                //16 chip-8 registers. [with last register being a carry flag]

        unsigned short I;                   //Index register. 
        unsigned short pc;                  //Program counter.


        unsigned char delay_timer;          
        unsigned char sound_timer;

        unsigned short stack[16];           
        unsigned short sp;                  //Stack pointer

        void init();    
    
    
    public: 

        Chip8();                            
        ~Chip8();
        
        void emulateCycle();                //Function to emulate a single chip-8 cpu cycle.
        bool load(const char * filename);   //Load ROM

        unsigned char gfx[64 *32];          //represents 2048 pixel screen.

        unsigned char key[16];     

        bool draw_flag;                     //System sets a drawflag to indicate that we need to update screen.
                                            //Only 2 opcodes update screen: 0x00E0(clear screen), and 0xDXYN(draw sprite)

};

#endif /* CHIP_8_H  */
