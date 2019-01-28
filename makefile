#Compiler
CC = gcc

#Compiler Flags
CFLAGS = -Iinclude
LDFLAGS =

#Directories & Directory Names
INCLUDES = -F/System/Library/Frameworks -framework OpenGL -framework GLUT -lm -mmacosx-version-min=10.8
LIBDIRS = 
LIBS = 
BINDIR =
SRCDIR = src/

#File Names
SOURCE = $(SRCDIR)tankcraft.c $(SRCDIR)networkFunctions.c $(SRCDIR)worldGen.c $(SRCDIR)graphics.c $(SRCDIR)visible.c
OBJS = tankcraft.o networkFunctions.o worldGen.o graphics.o visible.o
PROGNAME = $(BINDIR)tankcraft

default: all

all:	
	$(CC) $(CFLAGS) $(SOURCE) -o $(PROGNAME) $(INCLUDES)
		
clean:	
	@rm *.o
	@rm *.dSYM