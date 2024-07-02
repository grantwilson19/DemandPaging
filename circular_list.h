//
// Created by james on 10/23/2023.
//

#ifndef CIRCULAR_LIST_H
#define CIRCULAR_LIST_H

#include <iostream>
//#include "pagetable.h"
//using namespace std;

//circular list structure used by the WSclock
//node
typedef struct CIRLISTNODE{
    CIRLISTNODE* next;//pointer to the next node in the circularlist
    int pfn;
    unsigned int vpn;//track which virtual page is mapped to which physical page
    int lastAccessTime;//age of last access to PFN ....SET USING pageTable->countNumOfAcesses as a "virtual time"
    bool dirty;//flag indicates if a page had write access(1 to 1 mapping from readwrites and trace.tr)
};


//circular list structure STRUCTURE MUST BE CREATED AND INITIALIZED WITH FRAME 0 IN THE PAGETABLE
typedef struct CIRLIST{
    CIRLISTNODE* head;//start of the cirlist, the last entry points here
    int size;//number of nodes in the list
    CIRLISTNODE* currentNode;//represents the last node visited during list iteration.., points to head upon creation
    //int currentTime;//CONTINUOSLY UPDATED.. uses pageTable->countNumOfAccesses to simulate "virtualTime"
};


//CREATES A NEW CIRLIST WITH A FRAME 0
CIRLIST* createCirlist();




//insert node into the tail of the cirlist
void insertTail(CIRLIST* list, CIRLISTNODE* newNode);





// update list to add a node if pfn isn't found....otherwise updates existing node. For convenience, returns the VPN of the node being updated
void updateList(CIRLIST* list, unsigned int vpn, int pfn, int lastAccessTime, bool dirty);



//this method is invoked when a pageFault happens and we must select a vpn to evict
//should return the pfn of the victim frame! "wsclock implementation"
//unsigned int clock(PageTable* pageTable, CIRLIST* frames){
//CIRLISTNODE* pageReplace(PageTable *pageTable, CIRLIST *frames);



#endif //CIRCULAR_LIST_H
