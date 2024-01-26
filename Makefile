# 1 Billion Row Contest for Binghamton University CS 547.
# see: https://github.com/gunnarmorling/1brc
#
# author: Gregory Maldonado
# email : gmaldonado@cs.binghamton.edu
# date  : 2024-01-25
#
# Graduate student @ Thomas J. Watson College of Engineering and Applied
# Sciences, Binghamton University.

CC = g++
CFLAGS = -std=c++17 -Wall -Wextra -pedantic -g -c
TARGET = 1brc

all: $(TARGET)
	$(RM) *.o

$(TARGET): $(TARGET).o
	g++ $(CFLAGS) -o build/$(TARGET) $(TARGET).cpp

$(TARGET).o: $(TARGET).cpp $(TARGET).hpp
	g++ $(CFLAGS) -c $(TARGET).cpp

clean:
	$(RM) build/$(TARGET)