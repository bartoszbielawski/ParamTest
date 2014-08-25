OBJS=main.o

TARGET=main

CXXFLAGS=-g -std=c++11

CXX=clang++-mp-3.5

all: $(OBJS)
		$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET)
		
clean:
		rm -rf $(TARGET) $(OBJS)