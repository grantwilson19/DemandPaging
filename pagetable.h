/* Authors: James Marsh, Grant Wilson
 * REDID: 826209118m 824810214
 * CS480 Operating Systems
 * PA3 Virtual memory management
 */







#ifndef PAGETABLE_H
#define PAGETABLE_H

#include <map>
#include <iostream>
#include "circular_list.h"
#include "log_helpers.h"
#include <random>


using namespace std;


#define NORMAL_EXIT 0
#define NUM_OF_FILES 2
#define TR_FILE_IDX 0
#define RW_FILE_IDX 1
#define CPU_BITS 32
#define MAX_BITS 28
#define DEFAULT_AGE 10
#define DEFAULT_FRAMES 999999


//NOT optimized...
//PageTable object. Constructs a (multi)level page table TREE according to user-specific input. Tries to optimize space
class PageTable{


	//defines level structure for each level of a pageTable
	typedef struct LEVEL {

        //index of level[] is the VPN
            //each array is 2^n size from entryCount[]

		LEVEL** next;//array of LEVEL pointers corresponding to the next level in the PageTable tree... NULL ON LEAF NODES
            //if depth ==lvlCount

            //NOT NEEDED    MAP* maps[];//contains VPN <--> PFN mapping.. THIS IS ONLY ACTIVATED ON LEAF NODES....otherwise null...
        int* pfn;//maping to pfn. -1 if invalid,,, only used on leaf nodes

		PageTable* pageTable;//points to the root of the page table so that all data is accesible from any level

		int depth;//indicates the depth of a level within the page tree


		//initialize LEVEL struct elements to null / 0 after creating it. CALL THIS ONLY AFTER ARRAY SIZES HAVE BEEN SPECIFIED

	};




	private:
	public:
	LEVEL* root;//pointer to level 0 of the pageTable

    CIRLIST* frames;//pointer to the dynamic frame list used during page replacement

	int lvlCount = 0; //represents how many levels are contained in the PageTable.
                //last level contains a MAP[] instead of nextLevel[]

    int frameCount = 0;//tracks number of occupied frames...
    int frameLimit = DEFAULT_FRAMES;//sets frame limit to default, updated if user supplies a new limit
    int ageLimit = DEFAULT_AGE;//age of last access for pageReplacement...updated if user supplies a new limit
    int numOfAccesses = -1;//if not set to an integer >= 1, then all references in the trace file will be processed
    string logLvl;//stores the logging level for the page table
    LogOptionsType* logOptions;//stores the flag for which log mode is enabled
    unsigned int* vpns;//stores vpn(s) for each virtual address for logging
    unsigned int wholeVpn = 0;//stores the "vpn" for each address being processed
    unsigned int offset = 0;//stores offset for each virtual address for logging
    unsigned int offsetMask = 0;
    unsigned int vpnMask = 0;
    unsigned int physicalAddress= 0;//stores the physical addresss translation for each virtual address for logging


    //data for the summary log
    unsigned int page_size = 0;
    unsigned int numOfPageReplaces = 0;
    unsigned int pageTableHits = 0;
    unsigned int countNumOfAccesses = 0;//THIS IS ALSO USED AS THE VIRTUAL TIME FOR THE PageReplacement
    unsigned int numOfFramesAllocated = 0;
    unsigned long totalBytesUsed = 0;


    int totalBitsInput = 0;

    int* inputBits;//contains the specified number of bits used in each level from input


	unsigned int* bitmaskAry;//contains the corresponding hexadecimal bitmask to obstain the proper bits from  each level in the tree
	int* shiftAry;//used with the bitMask, this will align properly align the VPN portion from each level
		      //includes the shift for the leaf level for the offset?!

	int* entryCount;//number of possible entries for each level. (2^n bits where n is the input bits for a level)

	const char* file[NUM_OF_FILES];//store file names for I/O access

    //dynamic allocation of pageTable data per user input.
	void pageTableSetup(int levelCount);

    //configures the root of a tree once entryCount has been determined
    void setRoot();


    //determines the size of each specified from user input
    void setEntryCount();

    //determines the amount to shift an isolate vpn chunk for each level
    void createShiftAry();



    //creates the proper bitmask to isolate the VPN bits for each level
    void createBitmaskAry();

//isolate each vpn chunk using bitmask and shift..masks actual address to get LEVELspeciifc vpn chunk
    //does this for each level for a specific address`
    //insert a virtual address
    unsigned int getVPNFromVirtualAddress(unsigned int virtualAddress, unsigned int mask, int shift);

    //RECURSIVE
    //extract vpn at a level, see if exists, then go deeper or create the entry..recursive   ... check if leaf for mapping inserts.
    //pass frame as -1 to mark as invalid during pageReplacement, otherwise use frameCount
    void insertVpn2PfnMapping(LEVEL* currentLvl, unsigned int virtualAddress, int frame, bool rw);



    //use virtual address here instead of VPN?
    //ignores the offset so only uses the VPN portion of Virtual Address.
    //returns Physical frame number of the virtual page mapping, returns -1 if invalid/unmapped (-1 in two's complement, so FFFFFFFF)
            //has some redundant use of pointers here since it is a member function
    int* findVpn2PfnMapping(unsigned int virtualAddress);



    //returns the physical address translation for a virtual address using the offset and page number
    unsigned int va2pa(unsigned int pfn);


    //counts the number of bytes used in the page table...more levels should yield less bytes.
    void byteCount(LEVEL* current);


//this method is invoked when a pageFault happens and we must select a vpn to evict
//should return the pfn of the victim frame!
//unsigned int clock(PageTable* pageTable, CIRLIST* frames){
//"wsclock implementation"
    CIRLISTNODE* pageReplace();

























};






#endif // PAGETABLE_H
