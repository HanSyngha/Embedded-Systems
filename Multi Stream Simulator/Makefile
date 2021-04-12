#---------------------------------------------------------
#
# Lab #6 : Multi-Streamed SSD Simulator
#  - Embedded System Design, ICE3028 (Fall, 2019)
#
# Nov. 07, 2019.
# 
# TA: Junho Lee, Somm Kim
# Prof: Dongkun Shin
# Embedded Software Laboratory
# Sungkyunkwan University
# http://nyx.skku.ac.kr
#
#---------------------------------------------------------

STUDENT_ID = 2016311821

PREFIX		=
CC			= $(PREFIX)gcc
CXX			= $(PREFIX)g++

CFLAGS		= -g -O2 -Wall
LIBS		= 
RM			= rm
TAR			= tar

TARGET		= ftl_sim
CSRCS		= ftl_sim.c nand.c ftl.c
CXXSRCS		= 
HEADERS		= nand.h ftl.h
OBJS		= $(CSRCS:.c=.o) $(CXXSRCS:.cpp=.o)

all: $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LIBS) 

.c.o: $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

.cpp.o: $(HEADERS)
	$(CXX) $(CFLAGS) -c $< -o $@

tar:
	$(RM) -f $(STUDENT_ID).tar.gz
	$(TAR) cvzf $(STUDENT_ID).tar.gz $(CSRCS) $(HEADERS) Makefile report.pdf
	ls -l $(STUDENT_ID).tar.gz

clean:
	$(RM) -f $(TARGET) $(STUDENT_ID).tar.gz $(OBJS) bank* *~
