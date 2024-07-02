//
// Created by james on 10/24/2023.
//

#include "circular_list.h"
#include "pagetable.h"
using namespace std;

//
// Created by james on 10/23/2023.
//


//circular list structure used by the WSclock
//node

//CREATES A NEW CIRLIST WITH A FRAME 0
CIRLIST* createCirlist(){
    CIRLIST* newList = new CIRLIST;
    newList->size = 1;
    CIRLISTNODE* headNode = new CIRLISTNODE;
    headNode->pfn = 0;
    headNode->vpn = 0;
    headNode->lastAccessTime = 0;
    headNode->dirty = false;
    headNode->next = headNode;
    newList->head = headNode;
    newList->currentNode = newList->head;
    return newList;
}




//insert node into the tail of the cirlist
void insertTail(CIRLIST* list, CIRLISTNODE* newNode){
    CIRLISTNODE* current = list->head;

    //traverse so that current points to tail
    while( list->size != 1 && current->next != list->head ){
        current = current->next;
    }

    //when current->next would be the head, set it to be the new node instead.
   current->next = newNode;

    //advance to new node, and set its next to be the head
    current = current->next;
    current->next = list->head;//sets new tail node to point to the head
    list->size = list->size + 1;
    return;
}





// update list to add a node if pfn isn't found....otherwise updates existing node. For convenience, returns the VPN of the node being updated
void updateList(CIRLIST* list, unsigned int vpn, int pfn, int lastAccessTime, bool dirty){
   bool notFound = false;

    //need to search for node to be updated. start from head
    CIRLISTNODE* current = list->head;
    //find the node to update according to pfn
    while(current->pfn != pfn){

        if(current->next == list->head){
            notFound = true;
            break;
        }

        current = current->next;
        //if the list circles around and couldn't find the frame to update....mark notFound and exit loop
    }

    //add new node
    if(notFound){
        CIRLISTNODE* newNode = new CIRLISTNODE;
        newNode->pfn = pfn;
        newNode->vpn = vpn;
        newNode->lastAccessTime = lastAccessTime;
        newNode->dirty = dirty;
        newNode->next = nullptr;//set to head in the insert function.

        insertTail(list, newNode);//insert the new node
    }
        //frame with pfn param was found in list. update member data.
    else{
        current->lastAccessTime = lastAccessTime;
        current->vpn = vpn;
        //if the current page was not dirty, it will either be marked dirty, or kept the same
        if(!(current->dirty) ){
          current->dirty = dirty;//if the dirty param was false, this would be preserved on the page, otherwise it will be set to true and need a write schedule during page replacement
            //updating clean to clean preserves
        }
    }
}




