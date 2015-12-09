#Makefile  for svnsrv

CC?=clang
CXX?=clang++

ifeq ($(CXX),clang++)
CXXFLAGS= -std=c++11 -g -Wall  -Wunused-variable -pedantic -Weffc++ \
-Wno-gnu-anonymous-struct -Wno-unused-local-typedef -Wno-c99-extensions
else
CXXFLAGS= -std=c++11 -g -Wall  -Wunused-variable
endif
CFLAGS= -g -Wunused-variable -Wimplicit-function-declaration

ADDLIB= -lboost_coroutine -lboost_date_time -lboost_context -lboost_system  -lunwind -lpthread

PROJECTNAME=svnsrv

OBJECTS=main.o Daemon.o SubversionServer.o SubversionSession.o SubversionSyntactic.o URLTokenizer.o \
SubversionStorage.o FowardCompatible.o klog.o


all:$(PROJECTNAME)


$(PROJECTNAME): $(OBJECTS)
	$(CXX) $(ADDLIBDIR) $(OBJECTS)  $(ADDLIB) -o $@

$(OBJECTS):%.o:%.cc
	$(CXX) $(CXXFLAGS) $(ADDINCDIR) -c $< -o $@

clean:
	rm *.o svnsrv
