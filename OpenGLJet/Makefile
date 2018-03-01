CC=g++
LIBS= -lglut -lGL -lGLU
INCLUDES=  
CCFLAGS= -O0 -g -gstabs -I. -DLINUX
SOURCES=
OBJECTS=$(SOURCES:.cc=.o)
EXECUTABLE=jet

# --- targets
all:  $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): jet.cc $(OBJECTS)
	$(CC) jet.cc $(OBJECTS) -o $(EXECUTABLE) $(CCFLAGS) $(INCLUDES)  $(LIBS) 

.cc.o:
	$(CC) $(CCFLAGS) $(INCLUDES) $< -c

clean:
	rm -f $(EXECUTABLE) $(OBJECTS)
