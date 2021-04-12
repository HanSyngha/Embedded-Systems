#---------------------------------------------------------
#
# Project #1 : NAND Simulator
#  - Embedded Systems Design, ICE3028 (Fall, 2019)
#
# Sep. 19, 2019.
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

CFLAGS	= -g -O2 -Wall -std=c99
LIBS	= 
RM		= rm
TAR		= tar

TARGET	= nand_sim
CSRCS	= nand_sim.c nand.c
CXXSRCS	= 
HEADERS	= nand.h
OBJS	= $(CSRCS:.c=.o) $(CXXSRCS:.cpp=.o)

all: $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LIBS) 

.c.o: $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

.cpp.o: $(HEADERS)
	$(CXX) $(CFLAGS) -c $< -o $@

tar:
	$(RM) -f $(STUDENT_ID).tar.gz
	$(TAR) cvzf $(STUDENT_ID).tar.gz $(CSRCS) $(HEADERS) Makefile
	ls -l $(STUDENT_ID).tar.gz

clean:
	$(RM) -f $(TARGET) $(STUDENT_ID).tar.gz $(OBJS) bank* *~
