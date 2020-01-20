#
# Makefile for the AT24Cxx.
#


VERSION 	=1.00
CC			=gcc	#arm-hisiv300-linux-gcc
DEBUG 		=
CFLAGS		=-Wall
SOURCES	 	=$(wildcard *.c) $(wildcard ../24cxx/*.c)
INCLUDES  	=-I../24cxx
LIB_NAMES 	=
LIB_PATH 	=
OBJ			=$(patsubst %.c, %.o, $(SOURCES))
TARGET		=at24

#links
$(TARGET):$(OBJ)
	@mkdir -p output
	$(CC) $(OBJ) $(LIB_PATH) $(LIB_NAMES) -o output/$(TARGET)$(VERSION)
	@rm -rf $(OBJ)
	
#compile
%.o: %.c
	$(CC) $(INCLUDES) $(DEBUG) -c $(CFLAGS) $< -o $@

.PHONY:clean
clean:
	@echo "Remove linked and compiled files......"
	rm -rf $(OBJ) $(TARGET) output 


