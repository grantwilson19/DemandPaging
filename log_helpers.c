#include <stdio.h>
#include "log_helpers.h"

/* Handle C++ namespaces, ignore if compiled in C 
 * C++ usually uses this #define to declare the C++ standard.
 * It will not be defined if a C compiler is used.
 */
#ifdef __cplusplus
using namespace std;
#endif

/**
 * @brief Print out a number in hex, one per line
 * @param number 
 */
void print_num_inHex(uint32_t number) {
  printf("%08X\n", number);
  fflush(stdout);
}

/**
 * @brief Print out bitmasks for all page table levels.
 * 
 * @param levels - Number of levels
 * @param masks - Pointer to array of bitmasks
 */
void log_bitmasks(int levels, uint32_t *masks) {
  printf("Bitmasks\n");
  for (int idx = 0; idx < levels; idx++) 
    /* show mask entry and move to next */
    printf("level %d mask %08X\n", idx, masks[idx]);

  fflush(stdout);
}

/**
 * @brief log a virtual address to physical address mapping 
 * Example usages:
 * 
 * @param va 
 * @param pa 
 */
void log_va2pa(uint32_t va, uint32_t pa) {
  fprintf(stdout, "%08X -> %08X\n", va, pa);
  fflush(stdout);
}

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
                 bool pthit) {
  
  fprintf(stdout, "%08X -> %08X, ", src, dest);

  fprintf(stdout, "pagetable %s", pthit ? "hit" : "miss");
  
  if (vpnreplaced != -1) // vpn was replaced due to page replacement
    fprintf(stdout, ", %08X page was replaced\n", vpnreplaced);
  else {
    fprintf(stdout, "\n");
  }

  fflush(stdout);
}

/**
 * @brief log vpns at all levels and the mapped physical frame number
 * 
 * @param levels - specified number of levels in page table
 * @param vpns - vpns[idx] is the virtual page number associated with 
 *	              level idx (0 < idx < levels)
 * @param frame - page is mapped to specified physical frame
 */
void log_vpns_pfn(int levels, uint32_t *vpns, uint32_t frame) {
  /* output pages */
  for (int idx=0; idx < levels; idx++)
    printf("%X ", vpns[idx]);
  /* output frame */
  printf("-> %X\n", frame);

  fflush(stdout);
}

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
                 unsigned long totalBytesUsed) {
  unsigned int misses;
  double hit_percent;

  printf("Page size: %d bytes\n", page_size);
  /* Compute misses (page faults) and hit percentage */
  misses = numOfAddresses - pageTableHits;
  hit_percent = (double) (pageTableHits) / (double) numOfAddresses * 100.0;
  printf("Addresses processed: %d\n", numOfAddresses);
  printf("Page hits: %d, Misses: %d, Page Replacements: %d\n", 
         pageTableHits, misses, numOfPageReplaces);
  printf("Page hit percentage: %.2f%%, miss percentage: %.2f%%\n", 
         hit_percent, 100 - hit_percent);
  printf("Frames allocated: %d\n", numOfFramesAllocated);
  printf("Bytes used:  %ld\n", totalBytesUsed);

  fflush(stdout);
}

