#include <stddef.h>
#include <stdint.h>
#include "page.h"

struct ppage physical_page_array[128];//allocates memory in the kernel for the ppage structs. The ppage structs will contain an address that will point after end kernel 

struct ppage *freeList;

extern int _end_kernel;



int sizeOfList(struct ppage* list){
	int size = 0;
	while(list!=NULL){
		size++;
		list = list -> next;
	}

	return size;
}
 
void init_pfa_list(void){
	//get the address of the physical page array, that is where we will create the list 
	//need pointer to end_kern
	void *pagePointer = &_end_kernel; //physical address of the pages
	pagePointer = (unsigned int)pagePointer & 0xFFFFF000;
	pagePointer = pagePointer + 0x1000;
	freeList = physical_page_array;
	*(struct ppage *)freeList= (struct ppage){freeList + 1, NULL, pagePointer};
	freeList++; //increments freeList by one size of struct ppage 
	pagePointer+= 4096;	    //	
	for(int i = 1; i < 127; i++){ //sets up every element except first and last
	  *(struct ppage *)freeList = (struct ppage){freeList+1, freeList-1, pagePointer};
	  pagePointer+= 4096;
	  freeList++;
	}	

	*(struct ppage *) freeList = (struct ppage){NULL, freeList - 1, pagePointer};

	freeList = physical_page_array; //resets pointer to the beginning of the array 
}

struct ppage *allocate_physical_pages(unsigned int npages) { //number of page blocks to move from free to allocated list 
	if(npages == 0) {
		return NULL;
	}

	if(freeList == NULL){
		return NULL;
	}


	struct ppage* allocatedList = freeList;
	struct ppage* curr = freeList;
	
	int counter = 1;

	while(counter < npages && (curr->next != NULL)){
		curr = curr -> next;
		counter++;
	}

	freeList = curr->next;
	curr->next = NULL;
	if(freeList != NULL){
		freeList-> prev = NULL;
	}

	return allocatedList;
}

void free_physical_pages(struct ppage *ppage_list){ //returns allocated blocks back to free list
	if(ppage_list == NULL){
		return;
	}

	if(freeList == NULL){
		return ppage_list;
	}	

	struct ppage* curr = ppage_list;

	while(curr->next != NULL){
		curr = curr-> next;
	}

	curr -> next = freeList;
      	freeList -> prev = curr;
	freeList = ppage_list;	
}


void* man_pages(void *vaddr, struct ppage *ppage_list, struct page_directory_entry* pd){
	
	struct ppage* curr = ppage_list;
	while(curr != NULL){
		unsigned int vpn = ((unsigned int)vaddr) >> 12;
		//unsigned int pd_idx = (vpn >> 10);
		unsigned int pt_idx = vpn & 0x3ff;

		/*if(pd[pd_idx].present == 0){ //pulls a new page to hold the page table, identity map the page and creates page table
		    struct ppage* temp = allocate_physical_pages(1);
		    unsigned int temp_vpn = ((unsigned int)temp->physical_addr) >> 12;
		    unsigned int temp_pd_idx = (temp_vpn >> 10);
		    unsigned int temp_pt_idx = temp_vpn & 0x3ff; 

		    pd[temp_pd_idx].frame = ((unsigned int)temp_vpn) >> 12;
		    pd[temp_pd_idx].rw = 1;
		    pd[temp_pd_idx].present = 1;
		    
		    struct page* temp_page_table = pd[temp_pd_idx].frame << 12;
		    temp_page_table[pt_idx].frame = ((unsigned int)temp->physical_addr) << 12;
		    temp_page_table[pt_idx].rw = 1;
		    temp_page_table[pt_idx].present = 1;
		

		   // struct page page_table[1024] __attribute__((aligned(4096)));//needs to line up with physical addr of temp
		   // temp->physical_addr

		}*/
		struct page* pageTable = pd[1].frame << 12;
		pageTable[pt_idx].frame = ((unsigned int)curr->physical_addr) >> 12;
		pageTable[pt_idx].rw = 1;
		pageTable[pt_idx].present = 1;

		curr = curr->next;
		vaddr = vaddr + 0x1000;
	}

	return vaddr;
}



