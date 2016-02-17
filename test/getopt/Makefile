#Makefile  for svnsrv

CC?=clang
CXX?=clang++

BUILDTYPE?=Debug

ifeq ($(BUILDTYPE),Debug)
ADDRESSFIX= -fsanitize=address -fno-omit-frame-pointer
ADDRESSLINK= -fsanitize=address
endif

ifeq ($(CXX),clang++)
CXXFLAGS= -std=c++11 -g -Wall  -Wunused-variable -pedantic -Weffc++ -Wno-gnu-anonymous-struct \
-Wno-unused-local-typedef -Wno-c99-extensions $(ADDRESSFIX)
else
CXXFLAGS= -std=c++11 -g -Wall  -Wunused-variable $(ADDRESSFIX)
endif
CFLAGS= -g -Wunused-variable -Wimplicit-function-declaration



PROJECTNAME=getopt

OBJECTS=getopt.o GetOptInc.o


all:$(PROJECTNAME)


$(PROJECTNAME): $(OBJECTS)
	$(CXX) $(ADDRESSLINK) $(OBJECTS)  -o $@

$(OBJECTS):%.o:%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@


clean:
	rm *.o $(PROJECTNAME)
