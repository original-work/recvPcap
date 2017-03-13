
TARGET = Recv
OBJECT =	\
		Recv.o


CCTOOL = gcc
CXXTOOL = g++
LINKTOOL = g++
DEBUGFLAGS = -g
CCFLAGS = -I/usr/include -I/include -I../inc
LINKFLAGS = -L/usr/lib -L../inc -lrt


CCFLAGS += $(DEBUGFLAGS)



CXXFLAGS = $(CCFLAGS)

%.o:%.C
	$(CCTOOL) -c $< $(CCFLAGS)

%.o:%.cpp

	$(CXXTOOL) -c $< $(CXXFLAGS)



all: $(TARGET)

$(TARGET): $(OBJECT)
	$(LINKTOOL) -o $(TARGET) $(OBJECT) $(LINKFLAGS)

clean: 
	rm -f *.o $(TARGET)
