
#include <stdint.h>
#include "rprintf.h"

#define MULTIBOOT2_HEADER_MAGIC         0xe85250d6

const unsigned int multiboot_header[] __attribute__((section(".multiboot")))  = {MULTIBOOT2_HEADER_MAGIC, 0, 16, -(16+MULTIBOOT2_HEADER_MAGIC), 0, 12};

uint8_t inb (uint16_t _port) {
    uint8_t rv;
    __asm__ __volatile__ ("inb %1, %0" : "=a" (rv) : "dN" (_port));
    return rv;
}

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
    esp_printf(putc, "Igor");
    esp_printf(putc, "\n");
    esp_printf(putc, "Igor");
    esp_printf(putc, "\n");
   esp_printf(putc, "\n");
esp_printf(putc, "\n");
esp_printf(putc, "\n");
esp_printf(putc, "\n");
esp_printf(putc, "\n");
esp_printf(putc, "\n");
esp_printf(putc, "\n");
esp_printf(putc, "\n");
esp_printf(putc, "\n");
esp_printf(putc, "\n");
esp_printf(putc, "\n");
esp_printf(putc, "\n");
esp_printf(putc, "\n");
esp_printf(putc, "\n");
esp_printf(putc, "\n");
esp_printf(putc, "\n");
esp_printf(putc, "\n");

esp_printf(putc, "\n");
esp_printf(putc, "\n");
esp_printf(putc, "\n");

esp_printf(putc, "\n");
esp_printf(putc, "\n");
esp_printf(putc, "\n");


esp_printf(putc, "\n");
esp_printf(putc, "\n");
esp_printf(putc, "\n");

esp_printf(putc, "\n");
esp_printf(putc, "\n");
esp_printf(putc, "\n");

esp_printf(putc, "\n");
esp_printf(putc, "\n");
esp_printf(putc, "\n");


esp_printf(putc, "\n");
esp_printf(putc, "\n");
esp_printf(putc, "\n");


esp_printf(putc, "\n");
esp_printf(putc, "\n");
esp_printf(putc, "\n");


esp_printf(putc, "\n");
esp_printf(putc, "\n");
esp_printf(putc, "\n");


esp_printf(putc, "Hello World");
esp_printf(putc, "\n");

esp_printf(putc, "A");

















    while(1) {
        uint8_t status = inb(0x64);

        if(status & 1) {
            uint8_t scancode = inb(0x60);
        }
    }
}
