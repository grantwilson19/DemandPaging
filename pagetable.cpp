/* Authors: James Marsh, Grant Wilson
 * REDID: 826209118m 824810214
 * CS480 Operating Systems
 * PA3 Virtual memory management
 */



#include <iostream>
#include "circular_list.h"
#include "log_helpers.h"
#include "pagetable.h"
#include <random>
using namespace std;


//NOT optimized...
//PageTable object. Constructs a (multi)level page table TREE according to user-specific input. Tries to optimize space


    //dynamic allocation of pageTable data per user input.
    void PageTable::pageTableSetup(int levelCount){

        frames = createCirlist();//new cirlist with one pfn 0
        root = nullptr;
        lvlCount = levelCount;
        inputBits = new int[lvlCount];
        bitmaskAry = new unsigned int[lvlCount];
        shiftAry = new int[lvlCount];
        entryCount = new int[lvlCount];//2^n bits for each level
        vpns = new unsigned int[lvlCount];


        //configure log options
        //NOTE: logging would have been better configured as another function with dynamic data usage....
        logOptions = new LogOptionsType;
        logOptions->pagetable_bitmasks = false;
        logOptions->addressTranslation = false;
        logOptions->offset = false;
        logOptions->summary = false;
        logOptions->vpns_pfn = false;
        logOptions->vpn2pfn_with_pagereplace = false;

        if(logLvl == "bitmasks") {
            logOptions->pagetable_bitmasks = true;
        }
        else if(logLvl == "va2pa") {
            logOptions->addressTranslation = true;
        }
        else if(logLvl == "vpns_pfn") {
            logOptions->vpns_pfn = true;
        }
        else if (logLvl == "vpn2pfn_pr") {
            logOptions->vpn2pfn_with_pagereplace = true;

        }
        else if (logLvl ==  "offset") {
            logOptions->offset = true;
        }
        else{
            //prints summary by default, including for any typo in logLvl
            logOptions->summary = true;
        }




        //set arrays to null
        for(int i = 0; i<lvlCount; i++) {
            inputBits[i] = 0;
            bitmaskAry[i] = 0;
            shiftAry[i] = 0;
            entryCount[i] = 0;
        }

        //use delete[] operator to free the pointer arrays UNNEEDED

    }



    //configures the root of a tree once entryCount has been determined
    void PageTable::setRoot() {
        //determine virtual page size
        page_size = 1 << (CPU_BITS - totalBitsInput);

        //root = new LEVEL[entryCount[0]];
        root = new LEVEL;
        root->pageTable = this;//points back to the pageTable
        root->depth = 0;
        root->pfn = new int(-1); //invalidates map

        root->next = new LEVEL*[entryCount[0]];
        for (int i = 0; i < entryCount[0]; i++){
            root->next[i]=nullptr;
        }
        //root doesn't handle any mapping, maps would be assigned after inserting addresses...
    }




    void PageTable::setEntryCount() {
        for(int i = 0; i<lvlCount; i++) {
            entryCount[i] = 1 << inputBits[i];
        }

    }

    void PageTable::createShiftAry() {
        int totalBitsProcessed=0;
        for(int i = 0; i < lvlCount; i++) {
            shiftAry[i] = CPU_BITS - totalBitsProcessed - inputBits[i];

            // Update the totalBitsProcessed for the next iteration
            totalBitsProcessed += inputBits[i];
        }
    }

    void PageTable::createBitmaskAry() {
        //int totalBitsProcessed = 0; // Initialize the total processed bits
        for (int i = 0; i < lvlCount; i++) {
            bitmaskAry[i] = (1 << inputBits[i]) - 1;
            bitmaskAry[i] = bitmaskAry[i] << shiftAry[i];//this should work if createShiftAry() is done first
        }


        offsetMask = (1 << (CPU_BITS-totalBitsInput)) - 1;//generate offset mask
        vpnMask = ~(((1 << CPU_BITS) - 1) & offsetMask);

        //PRINT LOGGING FOR BITMASKS
        if(logOptions->pagetable_bitmasks){
            log_bitmasks(lvlCount,bitmaskAry);
        }
    }



//isolate each vpn chunk using bitmask and shift..masks actual address to get LEVELspeciifc vpn chunk
    //does this for each level for a specific address`
    //insert a virtual address
    unsigned int PageTable::getVPNFromVirtualAddress(unsigned int virtualAddress, unsigned int mask, int shift){
        return (virtualAddress & mask) >> shift;
    }


    //RECURSIVE
    //extract vpn at a level, see if exists, then go deeper or create the entry..recursive   ... check if leaf for mapping inserts.
    //pass frame as -1 to mark as invalid during pageReplacement, otherwise use frameCount
    void PageTable::insertVpn2PfnMapping(LEVEL* currentLvl, unsigned int virtualAddress, int frame, bool rw) {
        //get vpn chunk
        unsigned int vpn = getVPNFromVirtualAddress(virtualAddress,(bitmaskAry[currentLvl->depth]),(shiftAry[currentLvl->depth]));

        //store vpn at this level for logging
        vpns[currentLvl->depth] = vpn;

        //set the offset and the VPN for each address
        if(currentLvl->depth == 0){
            //shift for offset
            int shft = CPU_BITS - totalBitsInput;
            wholeVpn = this->getVPNFromVirtualAddress(virtualAddress,vpnMask,shft );
            offset = virtualAddress&offsetMask;
            //log offset
            if(logOptions->offset){
                print_num_inHex(offset);
            }
        }


        //check leaf case to handle PFN mapping
        if(currentLvl->depth == lvlCount) {
            //set mapping of VPN to PFN?

            //INVALIDATE MAPPING CASE
            if (frame == -1) {
                //mark as invalid
                *(currentLvl->pfn) = -1;
                return;
            }

            //check if already mapped frame is available for inserting
            //frame already mapped..no changes made to page table
            if (*(currentLvl->pfn) != -1) {
                pageTableHits = pageTableHits + 1;//mark as a hit!

                updateList(frames,wholeVpn,*(currentLvl->pfn),countNumOfAccesses,rw);//update frame list for recent access


                //LOGGING
                if (logOptions->vpns_pfn) {
                    log_vpns_pfn(lvlCount, vpns, *(currentLvl->pfn));
                } else if (logOptions->addressTranslation) {
                    log_va2pa(virtualAddress, va2pa(*(currentLvl->pfn)));
                } else if (logOptions->vpn2pfn_with_pagereplace) {
                    log_mapping(wholeVpn, *(currentLvl->pfn), -1, true);
                }
                return;

            }

                //frame not mapped, but there is an available frame (pageTable Miss)
            else if (frameCount < frameLimit) {
                //assign next frame
                currentLvl->pfn = (new int(frameCount));

                //update list with the new frame
                updateList(frames,wholeVpn,*(currentLvl->pfn),countNumOfAccesses,rw);




                //LOGGING
                if (logOptions->vpns_pfn) {
                    log_vpns_pfn(lvlCount, vpns, *currentLvl->pfn);
                } else if (logOptions->addressTranslation) {
                    va2pa(*(currentLvl->pfn));//get physical address
                    log_va2pa(virtualAddress, physicalAddress);
                } else if (logOptions->vpn2pfn_with_pagereplace) {
                    log_mapping(wholeVpn, *(currentLvl->pfn), -1, false);//no page hit and no page replacement
                }

                //increment frame count
                frameCount++;
                return;
            }
            //NO AVAILABLE FRAMES...pageFault
            //PAGE REPLACEMENT CASE

            //mark pagefault
            numOfPageReplaces = numOfPageReplaces + 1;

            //select victim.... holds the pfn of the page to be replaced. invoked with "this" pagetable and the frames within it.
            //WSCLOCK simulation
            CIRLISTNODE* victim = pageReplace();

            //maps to the victim frame using victim PFN instead of frameCount
            currentLvl->pfn = new int( (victim->pfn) );

            //invalidate victim's previous mapping...
            // leftshift vpn by size of offset to mock virtualAddress of victim
            unsigned int mockVirtualAddress = ((victim->vpn) << (CPU_BITS - totalBitsInput) );

            //LOGGING
            if(logOptions->vpn2pfn_with_pagereplace){
                //use  just the VPN for this, not VIRTUAL ADDRESS
                log_mapping(wholeVpn,victim->pfn, victim->vpn, false);//no page hit and no page replacement..uses current VPN as src. victim PFN as dest.
            }

            //update frame list with the replaced frame
            updateList(frames,wholeVpn,victim->pfn,countNumOfAccesses,rw);

            //invalidate previous mapping of the victim frame with -1
            insertVpn2PfnMapping(root,mockVirtualAddress,-1,rw);

            return;
        }

            //checks if VPN chunk is mapped, and inserts a level for that chunk if it is not
        else if(currentLvl->next[vpn] == nullptr){

            //insert new level
            currentLvl->next[vpn] = new LEVEL;//create level new level for VPN
            currentLvl->next[vpn]->depth = (currentLvl->depth)+1;//updates depth for next level

            currentLvl->next[vpn]->next = new LEVEL*[entryCount[(currentLvl->depth)+1]];//create next[]

            currentLvl->next[vpn]->pageTable = currentLvl->pageTable;//assigns new level pagetable root pointer

            currentLvl->next[vpn]->pfn = new int(-1); //invalidates map

            for(int i = 0;i<entryCount[(currentLvl->depth)+1];i++){
                currentLvl->next[vpn]->next[i]=nullptr;//updates next array entries to be null
            }


        }//skips new level insert if the vpn chunk was mapped
        //recursive insert call with next level....
        insertVpn2PfnMapping(currentLvl->next[vpn],virtualAddress,frame,rw);
        return;

        //no need to interact with the frames list at this point because it only is touched at the frame level
    }





    //use virtual address here instead of VPN?
    //ignores the offset so only uses the VPN portion of Virtual Address.
    //returns Physical frame number of the virtual page mapping, returns -1 if invalid/unmapped (-1 in two's complement, so FFFFFFFF)
    //has some redundant use of pointers here since it is a member function
    int* PageTable::findVpn2PfnMapping(unsigned int virtualAddress) {
        LEVEL* currentLvl = root;

        while(currentLvl->depth < lvlCount){
            unsigned int vpn = getVPNFromVirtualAddress(virtualAddress,bitmaskAry[currentLvl->depth],shiftAry[currentLvl->depth]);

            //return pfn mapping from the leaf level
            if(currentLvl->depth == lvlCount-1){
                //NEVER Makes IT HERE BECAUSE THINKS PFN ALWAYS -1
                return currentLvl->pfn;
            }

            if(currentLvl->next[vpn] == nullptr){
                return new int(-1);//missing vpn mapping
            }
            currentLvl=currentLvl->next[vpn];

        }
        return new int(-1);
    }




    //returns the physical address translation for a virtual address using the offset and page number
    unsigned int PageTable::va2pa(unsigned int pfn){
        int offsetSize = CPU_BITS - totalBitsInput;
        physicalAddress = (pfn << offsetSize) + offset;//obtain physical address
        return physicalAddress;
    }



    //counts the number of bytes used in the page table...more levels should yield less bytes.
    void PageTable::byteCount(LEVEL* current){
        //  cout << "CURRENT LVL DEPTH " << current->depth << endl;
        //called on root initially
        if(current->depth == lvlCount) {
            //base case of a leaf level
            totalBytesUsed = totalBytesUsed + sizeof( *(current->pfn) );
        }

        //getSize of the level array... capacity() for vector
        for(int i = 0; i<entryCount[current->depth];i++) {

            totalBytesUsed = totalBytesUsed + sizeof(current->next[i]);

            if(current->next[i] != nullptr) {
                byteCount(current->next[i]);
            }

        }
        return;
    }



//this method is invoked when a pageFault happens and we must select a vpn to evict
//should return the pfn of the victim frame!
//unsigned int clock(PageTable* pageTable, CIRLIST* frames){
//"wsclock implementation"
    CIRLISTNODE* PageTable::pageReplace(){
        CIRLISTNODE* victim = nullptr;//victim frame to return
        //"clock walk" through the frames until a victim is selected. if the entire list is walked through, select a random one?


        //this loop should go twice...to check again for any pages that were "written"
        int counter = 0;
        while (frames->currentNode->next != frames->head || (counter<(frames->size * 2))) {
            //"Find a page frame whose last access time is older than an age threshold
            //uses the pageTable access counter as a simulated "virtual time"
            if ((countNumOfAccesses - frames->currentNode->lastAccessTime) > ageLimit) {

                //if dirty flag is true, "schedule write"
                if (frames->currentNode->dirty) {
                    frames->currentNode->dirty = false;//simulate a scheduled write
                }
                else {
                    //select the frame as a victim frame if it was clean.
//                victim = frames->currentNode->pfn;
                    victim = frames->currentNode;

                    //advance clock for future page replacements
                    frames->currentNode = frames->currentNode->next;
                    return victim;
                }
            }

            //advance clock to next page
            frames->currentNode = frames->currentNode->next;
            counter++;
        }//endwhile
        //pseudo random selection of victim frame if all pages are in the working set
        //all pages in working set, select a random page as victim


        //scared if this doesn't work with root
        int bound = frames->size -1;
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<int> dist(1,bound);
        int rand = dist(gen);
        victim = frames->head;
        for(int i = 0; i<=rand; i++){
            victim=victim->next;
        }
        return victim;
    }
//no wsclock class..just functions that levarage a pagetable

