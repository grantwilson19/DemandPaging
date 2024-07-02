#include <iostream>
#include <fstream>
#include <cstdio>
#include <unistd.h>
#include <string>
#include "pagetable.h"
#include "circular_list.h"
#include "log_helpers.h"
#include "vaddr_tracereader.h"

using namespace std;



//TODO: byte counts




//main function for program execution.... to .... . . . . .
int main(int argc, char* argv[]){

	//***********************************
	//PROGRAM DATA: variables to store argument info..
    int lvlCount = 0;
	//test pg table
	PageTable* pageTable = new PageTable();//= new PageTable();
	//***********************************







//TODO: handle optional inputs	**********************
	//integer for optional args.
	int option = 0;

	int val = 0;//handle optional integer arguments

    while((option = getopt(argc,argv,"n:f:a:l:")) != -1){
		switch(option){
			//number of accesses to process
            case 'n':
				val = atoi(optarg);
					if(val < 1){
                        printf("Number of memory accesses must be a number, greater than 0\n");
                        exit(NORMAL_EXIT);
                    }
					 pageTable->numOfAccesses = val;//set number of accesses

				break;

            //physical frame limit for the page table
			case 'f':
				val = atoi(optarg);
                if(val < 1){
                    printf("Number of available frames must be a number, greater than 0\n");
                    exit(NORMAL_EXIT);
                }
                pageTable->frameLimit = val;//set frameLimit to user input
				break;

            //age of last access
			case 'a':
				val = atoi(optarg);
                if(val < 1){
                    printf("Age of last access considered recent must be a number, greater than 0\n");
                    exit(NORMAL_EXIT);
                }
                pageTable->ageLimit = val;
				break;

			//program logging mode
			case 'l':       
				pageTable->logLvl=optarg;
				break;
			default:
				printf("invalid argument\n");
				exit(NORMAL_EXIT);
		}
	}



	//INDEX path to mandatory arguments after the opt. args
	int optidx = optind;

	//verify that min. num. of args was given. (two filenames,and bit amnt for level 0.)	
	if(optidx+2>argc){
		exit(NORMAL_EXIT);
	}


	/*
		idx, and idx+1 are file names. any other args are input bit counts for each level.
		level0 bit must be atleast 1. all other bit level must also be atleast 1. total bits must be <=28
		offset = 32-totalBitsInput;	offset minimum is 4 bits
	*/

//get filenames from arguments
	int tr_idx =optidx;
	optidx++;

	int rw_idx =optidx;
	optidx++;


	//set level count to the amount of different bit-specific levels in remaining args.
    lvlCount = argc-optidx;


    //construct page table dynamically using the lvlCount.
	pageTable->pageTableSetup(lvlCount);


    //TODO TEST TEST TEST
  //cout << "LEVEL COUNT: " << pageTable->lvlCount << " OR " << lvlCount << endl;


    //set the file names
	pageTable->file[TR_FILE_IDX] = argv[tr_idx];
	pageTable->file[RW_FILE_IDX] = argv[rw_idx];


    //fetch the user specified bits.
	for(int i = 0;optidx<argc;i++){

        //check for valid input bits
		if(atoi(argv[optidx]) <= 0){
			printf("Level %d page table must be at least 1 bit\n",i);
			exit(NORMAL_EXIT);	
		}

        //load pageTable with the input data
		pageTable->inputBits[i] = atoi(argv[optidx]);


        //TODO TEST TEST TEST
        //cout << "INPUT BITS: " << pageTable->inputBits[i] << endl;


        pageTable->totalBitsInput+=atoi(argv[optidx]);

	    //increment to next arg
        optidx++;
	}

//TODO TEST
//cout <<"total bits: " << pageTable->totalBitsInput << endl;


	//validate the total user input bits
	if(pageTable->totalBitsInput>MAX_BITS){
		printf("Too many bits used in page tables\n");
		exit(NORMAL_EXIT);
	}



    //set up pageTable parameters
    pageTable->setEntryCount();
    pageTable->createShiftAry();
    pageTable->createBitmaskAry();
    pageTable->setRoot();//initializes pageTable root level.

    //calculate offset DECLARE THIS IN PAGE TABLE...32bit system MINUS bits specified for each level
    //offset = CPU_BITS-totalBitsInput;





    //FILE I/O HANDLING
	FILE* tr_file = fopen(pageTable->file[TR_FILE_IDX],"r");//open trace file for reading
	if(tr_file == nullptr){
		printf("Unable to open <<trace.tr>>\n");
		exit(NORMAL_EXIT);
	}
	FILE* rw_file = fopen(pageTable->file[RW_FILE_IDX],"r");//open readw rites file for reading
	if(rw_file == nullptr){
		printf("Unable to open <<readswrites.txt>>\n");
		exit(NORMAL_EXIT);
	}






	//create address struct found in the vaddr_tracereader.h file
	p2AddrTr* virtualAddressTrace = new p2AddrTr;
	virtualAddressTrace->addr=0;
	virtualAddressTrace->time=0;
    unsigned int virtualAddress = 0;

    //0 or 1 for file memory access
    int access = 0;
    bool rw = false;



    //iterate through each virtual address in the trace file. Function returns 0 when it cannot complete
	while(NextAddress(tr_file,virtualAddressTrace)!=0){
            //get the rw access..negates edge case since there should be 1-1 mapping for mem addr.
            access = fgetc(rw_file);
            if(access == 48){//ascii for 0, which means no write access
                rw=false;
            }
            else{
                rw=true;//write access was granted
            }
       //cout << "RW ACC " << access << "  " << rw << endl;

        virtualAddress = virtualAddressTrace->addr;
          //  cout << "(main.cpp) VIRTUAL ADDRESS: " << hex << virtualAddress << endl;
           // if(!(pageTable->logOptions->summary)){
             //   pageTable->printLogs();
            //}//DONT NEED THIS ONE

            //4 6 8 bits, offset is 14.
           // 004758a0 --> VA to VPN --> 0000011D

           // 0001 0001 1101      offset0001 100010100000
            //vpn should be 11D
           // 1101

            //TODO page replacement not working!
            //TODO byte counts for summary, but everything else is working!?
           pageTable->insertVpn2PfnMapping(pageTable->root,virtualAddress,0,rw);


            //if number of addresses to process was given, compare to this counter
            //increment and compare access counter, never needed if  numOfAccesses was not specified
            pageTable->countNumOfAccesses++;
            if(pageTable->countNumOfAccesses == pageTable->numOfAccesses){
               break;
            }

	}

    //LOG THE RUNTIME SUMMARY
    if(pageTable->logOptions->summary) {
        pageTable->byteCount(pageTable->root);
        //cout << "SUMMARY TEST totalBytes " << pageTable->totalBytesUsed << endl;
        log_summary(pageTable->page_size,pageTable->numOfPageReplaces,pageTable->pageTableHits,pageTable->countNumOfAccesses,pageTable->frameCount,pageTable->totalBytesUsed);
    }

    //close files
	fclose(tr_file);
	fclose(rw_file);



			return 0;
}
