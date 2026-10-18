CXX = g++
CXXFLAGS = -O3 -std=c++17 -Wall -Wextra

TARGET = solution
SRC = main.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)