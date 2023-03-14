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



//struct page_directory_entry* pde[0xF]; //16 length array that points to ped structs
//struct page first_page_table[1024] __attribute__((aligned(4096)));

extern int _end_kernel;


struct page_directory_entry pd[1024] __attribute__((aligned(4096)));
struct page first_page_table[1024] __attribute__((aligned(4096)));
struct page second_page_table[1024] __attribute__((aligned(4096)));


//uint32_t page_directory[1024] __attribute__((aligned(4096)));


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
    void* last_kern_page  = &_end_kernel;
    last_kern_page = (unsigned int)last_kern_page & 0xFFFFF000;//start of the last page of kernel 
    esp_printf(putc, "Hello\n");
    init_pfa_list();



    for(int i =0; i < 1024; i ++){ //zero out the page directory
	pd[i].present = 0;
	pd[i].rw = 0;
	pd[i].user = 0;
	pd[i].writethru = 0;
	pd[i].cachedisabled = 0;
	pd[i].accessed = 0;
	pd[i].pagesize = 0;
	pd[i].ignored = 0;
	pd[i].os_specific = 0;
	pd[i].frame = 0;
    }


 

    for(int i =0; i<1024; i++){ //zero out the first page table
	first_page_table[i].present = 0;
	first_page_table[i].rw = 0;
	first_page_table[i].user = 0;
	first_page_table[i].accessed = 0;
	first_page_table[i].dirty = 0;
	first_page_table[i].unused = 0;
	first_page_table[i].frame = 0;
    }


    for(int i =0; i<1024; i++){ //zero out the second page table
	second_page_table[i].present = 0;
	second_page_table[i].rw = 0;
        second_page_table[i].user = 0;
	second_page_table[i].accessed = 0;
	second_page_table[i].dirty = 0;
	second_page_table[i].unused = 0;
	second_page_table[i].frame = 0;
    }

    void* kernCurrent = 0x0;
    while(kernCurrent <= last_kern_page){
	unsigned int vpn = ((unsigned int)kernCurrent) >> 12;
	unsigned int pd_idx = (vpn>>10);
	unsigned int pt_idx = vpn & 0x3ff;
	pd[pd_idx].frame = (unsigned int)first_page_table >> 12;
	pd[pd_idx].rw = 1;
	pd[pd_idx].present = 1;

	struct page* temp = pd[pd_idx].frame << 12;
	temp[pt_idx].frame = (unsigned int)kernCurrent >> 12;
	temp[pt_idx].rw = 1;
	temp[pt_idx].present = 1;

	kernCurrent = (unsigned int)kernCurrent + 0x1000;
    }

    pd[1].frame = (unsigned int)second_page_table >> 12;
    pd[1].present = 1;
    pd[1].rw = 1;

    	asm("mov %0,%%cr3"
	    :
            : "r"(pd)
            :);	    

	 asm("mov %cr0, %eax\n"
 	"or $0x80000001, %eax\n"
 	"mov %eax,%cr0");


	struct ppage* pageList = allocate_physical_pages(2);
	free_physical_pages(pageList);
	
	//void* va = 0x300000;
        //man_pages(va, pageList, pd);	


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
