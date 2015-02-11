CC = gcc
CXX = g++ -std=c++11
# Additional include directory
INCLUDES = 

# Compilation options:
# -g for degging infor and -Wall enables all warnings
CFLAGS   = -g -Wall $(INCLUDES)
CXXFLAGS = -g -Wall $(INCLUDES)

# Linking options:
# -g for debugging info
LDFLAGS = -g

# List the libraries you need to link with in LDLIBS
# -lm for the math library
LDLIBS = -L../prob_verify -lpverify

SOURCES=main.cpp channel.cpp competitor.cpp controller.cpp lock.cpp lock_service.cpp lock_utils.cpp

OBJECTS=$(SOURCES:.cpp=.o)

EXECUTABLE=verify-lock

# The 1st target gets build when typing "make"
$(EXECUTABLE): $(OBJECTS) 
	$(CXX) $(LDFLAGS) $(OBJECTS) $(LDLIBS) -o $(EXECUTABLE)

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $< 

.PHONY: clean
clean:
	rm -f *.o *.a *.gch a.out core $(EXECUTABLE)

.PHONY: all
all: clean $(EXECUTABLE)

