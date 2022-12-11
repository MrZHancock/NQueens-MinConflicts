CXX = g++-12
CXXFLAGS = -std=c++20 -O3 -Wall -pedantic
TARGET = Queens
OBJFILES = Queens.o
all: $(TARGET)
queens: queens.cpp
	$(CXX) $(CXXFLAGS) -o Queens queens.cpp
run: queens
	./Queens
clean:
	rm -rf $(OBJFILES) $(TARGET) *~
	rm -rf *.exe
	rm -rf *.out
