CC=g++
# LIBS on osx are in the framework
#LIBS= -lglut -lGL -lGLU
INCLUDES=  
CCFLAGS= -O0 -g -gstabs -I. -DOSX -framework OpenGL -framework GLUT
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
