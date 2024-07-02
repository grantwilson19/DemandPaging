# CXX Make variable for compiler
CC=g++
# -std=c++11  C/C++ variant to use, e.g. C++ 2011
# -Wall       show the necessary warning files
# -g3         include information for symbolic debugger e.g. gdb 

#hide debug flag to improve performance 
CCFLAGS=-std=c++11 -Wall -g3 -c
#CCFLAGS=-std=c++11 -O0 -Wall -c

# object files
OBJS = pagetable.o circular_list.o log_helpers.o vaddr_tracereader.o main.o

# Program name
PROGRAM = demandpaging

# The program depends upon its object files -lpthread not needed because not multithreaded
$(PROGRAM) : $(OBJS)
	$(CC) -o $(PROGRAM) $(OBJS)

main.o : main.cpp
	$(CC) $(CCFLAGS) main.cpp

vaddr_tracereader.o : vaddr_tracereader.c vaddr_tracereader.h
	$(CC) $(CCFLAGS) vaddr_tracereader.c


circular_list.o : circular_list.h circular_list.cpp
	$(CC) $(CCFLAGS) circular_list.cpp

log_helpers.o : log_helpers.h  log_helpers.c
	$(CC) $(CCFLAGS) log_helpers.c

pagetable.o : pagetable.h pagetable.cpp
	$(CC) $(CCFLAGS) pagetable.cpp


# Once things work, people frequently delete their object files.
# If you use "make clean", this will do it for you.
# As we use gnuemacs which leaves auto save files termintating
# with ~, we will delete those as well.
clean :
	rm -f *.o *~ $(PROGRAM)


