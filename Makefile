# 1 Billion Row Contest for Binghamton University CS 547.
# see: https://github.com/gunnarmorling/1brc
#
# author: Gregory Maldonado
# email : gmaldonado@cs.binghamton.edu
# date  : 2024-01-25
#
# Graduate student @ Thomas J. Watson College of Engineering and Applied
# Sciences, Binghamton University.

CC     = g++
CFLAGS = -std=c++17 -Wall -Wextra -pedantic -g -O
TARGET = 1brc

.pre:
	mkdir -p build

all: $(TARGET)
	make .post
	$(RM) *.o

$(TARGET): $(TARGET).o
	$(CC) $(CFLAGS) -o build/$(TARGET) $(TARGET).cpp -pthread -lstdc++

$(TARGET).o: .pre $(TARGET).cpp $(TARGET).hpp
	$(CC) $(CFLAGS) -c $(TARGET).cpp -pthread -lstdc++

.post:
	chmod +x build/$(TARGET)

clean:
	$(RM) build/$(TARGET)
