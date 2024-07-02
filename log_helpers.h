#ifndef LOG_HELPERS_H
#define LOG_HELPERS_H

/*
 * Compilation notes:
 * C compilers:
 *    uses bool, must compile with -std=c99 or later (bool was introduced
 *    in the 1999 C standard.
 *
 * C++ compilers
 *    uses uint32_t, unsigned 32 bit integer type, introduced in C++11,
 *    The defaults in the g++ compiler on edoras should be fine with this
 */

/* C and C++ define some of their types in different places.
 * Check and see if we are using C or C++ and include appropriately
 * so that this will compile under C and C++
 */
#ifdef __cplusplus
/* C++ includes */
#include <stdint.h>
#else
/* C includes */
#include <inttypes.h>
#include <stdbool.h>
#endif 

/*
 * structure that can be used to maintain which output types are enabled.
 * Note that this uses the bool keyword. 
 *
 * If compiled with a C compiler, make sure that the C99 dialect or later is used.
 * (-std=c99 with a GNU C compiler)
 */
typedef struct {
  bool pagetable_bitmasks;  /* display bitmaks on all page table levels */
  bool addressTranslation;  /* show virtual to physical address translation */
  bool vpns_pfn; /* show vpns of all levels to physical frame number */
  bool vpn2pfn_with_pagereplace;  /* show mapping between vpn and pfn indicating pagereplace */
  bool offset; /* show the offset of each address */
  bool summary; /* summary statistics */
} LogOptionsType;

/**
 * @brief Print out a number in hex, one per line
 * @param number 
 */
void print_num_inHex(uint32_t number);

/**
 * @brief Print out bitmasks for all page table levels.
 * 
 * @param levels - Number of levels
 * @param masks - Pointer to array of bitmasks
 */
void log_bitmasks(int levels, uint32_t *masks);

/**
 * @brief Given a pair of numbers, output a line: 
 *        src -> dest  
 * Example usages:
 * log mapping between virtual and physical addresses
 *   e.g., log_mapping(va, pa, vpnReplaced, false)
 *         pagetable miss, replaced vpn
 * log mapping between vpn and pfn: mapping(page, frame)
 *   e.g., log_mapping(vpn, pfn, vpnReplaced, true)
 *         pagetable hit
 * 
 * if vpnReplaced is -1, there was no page replacement
 * 
 * note if vpnReplaced is bigger than 0, pthit has to be false
 * 
 * @param src 
 * @param dest 
 * @param pagereplace
 * @param pthit 
 */
void log_mapping(uint32_t src, uint32_t dest, 
                 int vpnreplaced,
                 bool pthit);

/**
 * @brief log a virtual address to physical address mapping 
 * Example usages:
 * 
 * @param va
 * @param pa 
 */


void log_va2pa(uint32_t va, uint32_t pa);

/**
 * @brief log vpns at all levels and the mapped physical frame number
 * 
 * @param levels - specified number of levels in page table
 * @param vpns - vpns[idx] is the virtual page number associated with 
 *	              level idx (0 < idx < levels)
 * @param frame - page is mapped to specified physical frame
 */
void log_vpns_pfn(int levels, uint32_t *vpns, uint32_t frame);

/**
 * @brief log summary information for the page table.
 * 
 * @param page_size - Number of bytes per page
 * @param numOfPageReplaces - Number of page replacements
 * @param pageTableHits - Number of times a virtual page was mapped
 * @param numOfAddresses - Number of addresses processed
 * @param numOfFramesAllocated - Number of frames allocated
 * @param totalBytesUsed - Total number of bytes needed for page table data structure.  
 *                         Should include all levels, allocated arrays, etc.
 */
void log_summary(unsigned int page_size, 
                 unsigned int numOfPageReplaces,
                 unsigned int pageTableHits, 
                 unsigned int numOfAddresses, 
                 unsigned int numOfFramesAllocated,
                 unsigned long totalBytesUsed);


#endif
