CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
TARGET = server
SRC = server.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all run clean
