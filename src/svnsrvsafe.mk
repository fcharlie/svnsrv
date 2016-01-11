#Makefile  for svnsrv

CC=clang
CXX=clang++

CHECKTYPE?=ADDR

PREFIX?=/opt/svnsrv


ifeq ($(CHECKTYPE),ADDR)
OPCHECK= -fsanitize=address -fno-omit-frame-pointer
OPLINK= -fsanitize=address
OPTLEVEL=-O1
endif

ifeq ($(CHECKTYPE),THREAD)
OPCHECK= -fsanitize=thread
OPLINK= -fsanitize=thread
OPTLEVEL=-O2
endif

ifeq ($(CHECKTYPE),MEMORY)
OPCHECK= -fsanitize=memory -fPIE
OPLINK= -fsanitize=memory -pie
OPTLEVEL=-O1
endif

ifeq ($(CHECKTYPE),UNDEFINED)
OPCHECK= -fsanitize=undefined
OPLINK= -fsanitize=undefined
OPTLEVEL=-O1
endif


ifeq ($(CXX),clang++)
CXXFLAGS= -std=c++11 -g $(OPTLEVEL) -Wall  $(OPCHECK) -Wunused-variable -pedantic -Weffc++ \
-Wno-gnu-anonymous-struct -Wno-unused-local-typedef -Wno-c99-extensions
else
CXXFLAGS= -std=c++11 -g $(OPTLEVEL) -Wall  -Wunused-variable $(OPCHECK)
endif

CFLAGS= -g -Wunused-variable $(OPTLEVEL) -Wimplicit-function-declaration $(OPCHECK)

ADDLIB= -lboost_thread -lboost_system -lpthread

PROJECTNAME=svnsrv

OBJECTS=main.o Daemonize.o SubversionServer.o SubversionSession.o SubversionSyntactic.o URLTokenizer.o \
SubversionStorage.o klog.o FowardCompatible.o


all:$(PROJECTNAME)


$(PROJECTNAME):$(OBJECTS)
	$(CXX) $(OPLINK)  $(OBJECTS)  $(ADDLIB) -o $@

$(OBJECTS):%.o:%.cc
	$(CXX) $(CXXFLAGS) $(ADDINCDIR) -c $< -o $@

clean:
	-rm *.o svnsrv
