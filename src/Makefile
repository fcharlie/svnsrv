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


ADDLIB=  -lboost_system -lboost_thread  -lpthread

PROJECTNAME=svnsrv

OBJECTS=main.o Daemonize.o SubversionServer.o SubversionSession.o SubversionSyntactic.o URLTokenizer.o \
SubversionStorage.o FowardCompatible.o klog.o


all:$(PROJECTNAME)


$(PROJECTNAME): $(OBJECTS)
	$(CXX) $(ADDRESSLINK) $(OBJECTS)  $(ADDLIB) -o $@

$(OBJECTS):%.o:%.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

install:$(PROJECTNAME) ../config/svnsrv.toml ../config/router.toml
	cp $(PROJECTNAME) ../bin
	cp ../config/svnsrv.toml ../bin
	cp  ../config/router.toml ../bin


clean:
	rm *.o svnsrv
