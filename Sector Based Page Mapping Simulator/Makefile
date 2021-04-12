#---------------------------------------------------------
#
# Lab #5: Sector-based Page Mapping FTL Simulator
#  - Embedded System Design, ICE3028 (Fall, 2019)
#
# Oct. 17, 2019.
#
# TA: Junho Lee, Somm Kim
# Prof: Dongkun Shin
# Embedded Software Laboratory
# Sungkyunkwan University
# http://nyx.skku.ac.kr
#
#---------------------------------------------------------

STUDENT_ID = 2016311821

PREFIX	=
CC		= $(PREFIX)gcc
CXX		= $(PREFIX)g++

CFLAGS	= -g -O2 -Wall -std=c99 $(WORKLOAD) $(FTL)
LIBS	= 
RM		= rm
TAR		= tar

TARGET	= ftl_sim
CSRCS	= ftl_sim.c nand.c ftl.c
CXXSRCS	= 
HEADERS	= nand.h ftl.h
OBJS	= $(CSRCS:.c=.o) $(CXXSRCS:.cpp=.o)

all: $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LIBS) 

.c.o: $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

.cpp.o: $(HEADERS)
	$(CXX) $(CFLAGS) -c $< -o $@

tar:
	$(RM) -f $(TARGET).tar.gz
	$(TAR) cvzf $(TARGET).tar.gz $(CSRCS) $(HEADERS) Makefile report.pdf
	ls -l $(TARGET).tar.gz

clean:
	$(RM) -f $(TARGET) $(TARGET).tar.gz $(OBJS) bank* *~
