# Compiler and Flags
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2

# Source Files
SRCS = inputbuf.cc lexer.cc parser.cc
OBJS = $(SRCS:.cc=.o)

# Executable Name
TARGET = parser

# Rule to Build the Executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

# Rule to Compile `.cc` Files into `.o` Files
%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean Up Compiled Files
clean:
	rm -f $(OBJS) $(TARGET)
