# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -Wall -std=c++17 -MMD -MP -g

# Boost libraries
LIBS = -lboost_system -lpthread

# Source files
SRCS = service.cpp proxy.cpp cache.cpp

# Object files
OBJS = $(SRCS:.cpp=.o)

# Dependency files
DEPS = $(OBJS:.o=.d)

# Executable name
EXEC = server

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(EXEC) $(OBJS) $(LIBS)

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $<  -o $@

clean:
	$(RM) $(OBJS) $(EXEC) $(DEPS) log.txt

cleanlog:
	$(RM) log.txt

-include $(DEPS)