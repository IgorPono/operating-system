#include <stddef.h>
#include <stdint.h>
#include "rprintf.h"
#include "page.h"




#define MULTIBOOT2_HEADER_MAGIC         0xe85250d6

const unsigned int multiboot_header[] __attribute__((section(".multiboot")))  = {MULTIBOOT2_HEADER_MAGIC, 0, 16, -(16+MULTIBOOT2_HEADER_MAGIC), 0, 12};

uint8_t inb (uint16_t _port) {
    uint8_t rv;
    __asm__ __volatile__ ("inb %1, %0" : "=a" (rv) : "dN" (_port));
    return rv;
}


void outb (uint16_t _port, uint8_t val) {
__asm__ __volatile__ ("outb %0, %1" : : "a" (val), "dN" (_port) );
}



unsigned char keyboard_map[128] =
{
   0,  27, '1', '2', '3', '4', '5', '6', '7', '8',     /* 9 */
 '9', '0', '-', '=', '\b',     /* Backspace */
 '\t',                 /* Tab */
 'q', 'w', 'e', 'r',   /* 19 */
 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', /* Enter key */
   0,                  /* 29   - Control */
 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',     /* 39 */
'\'', '`',   0,                /* Left shift */
'\\', 'z', 'x', 'c', 'v', 'b', 'n',                    /* 49 */
 'm', ',', '.', '/',   0,                              /* Right shift */
 '*',
   0,  /* Alt */
 ' ',  /* Space bar */
   0,  /* Caps lock */
   0,  /* 59 - F1 key ... > */
   0,   0,   0,   0,   0,   0,   0,   0,  
   0,  /* < ... F10 */
   0,  /* 69 - Num lock*/
   0,  /* Scroll Lock */
   0,  /* Home key */
   0,  /* Up Arrow */
   0,  /* Page Up */
 '-',
   0,  /* Left Arrow */
   0,  
   0,  /* Right Arrow */
 '+',
   0,  /* 79 - End key*/
   0,  /* Down Arrow */
   0,  /* Page Down */
   0,  /* Insert Key */
   0,  /* Delete Key */
   0,   0,   0,  
   0,  /* F11 Key */
   0,  /* F12 Key */
   0,  /* All other keys are undefined */
};





int xPos = 0;
int yPos = 0;

putc(int c){
	
	if (yPos >= 25 || (yPos == 24 && c == '\n' )) {
		uint8_t *curr = 0xb8000 + 160;
		uint8_t *freeMem = 0xb8000;
		for(int i = 0; i < 3840; i++){
			*freeMem = *curr;
			++freeMem;
			++curr;
		}

		for(int j = 0; j<80; j++){
			*freeMem =' '; 
			++freeMem;
			*freeMem = 0x07;
			++freeMem;	
		}	
		yPos = yPos -= 1;	
	}


	if( c == '\n'){
		xPos = 0;
		yPos +=1;
		return;	
	}

	uint8_t *mem = 0xb8000;
	mem[xPos+160*yPos] = c;
	mem[(xPos+160*yPos)+1] = 0x07;

	xPos = (xPos + 2) % 160;
	if (xPos == 0) {
		yPos +=1;
	}

}



	
void main() {
    unsigned short *vram = (unsigned short*)0xb8000; // Base address of video mem
    const unsigned char color = 7; // gray text on black background

    esp_printf(putc, "Hello\n");
    
    init_pfa_list();
    struct ppage* mem_space = allocate_physical_pages(5);
    int size = sizeOfList(mem_space);
    free_physical_pages(mem_space);



    while(1) {
        uint8_t status = inb(0x64);

        if(status & 1) {
            uint8_t scancode = inb(0x60);
	    if(keyboard_map[scancode] >= 1 && keyboard_map[scancode]!=0){
              // esp_printf(putc, "0x%02x   %c\n", ((unsigned int)scancode) & 0xff, keyboard_map[scancode]);
	   	    if((scancode & 0x80) == 0){ 
	   	        esp_printf(putc, "%c", keyboard_map[scancode]);
		    }
		 }
        }
    }
}
