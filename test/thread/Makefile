
GXX=g++
CXX=clang++

CXXFLAGS= -std=c++11 -g -Wall  -Wunused-variable -pedantic -Weffc++ \
-Wno-gnu-anonymous-struct -Wno-unused-local-typedef -Wno-c99-extensions  -fsanitize=address -fno-omit-frame-pointer
GXXFLAGS= -std=c++11 -g -Wall  -Wunused-variable -fsanitize=address -fno-omit-frame-pointer

BOOSTBIN=boostthread
LIBCXXBIN=libcxxthread
LIBSTDCXXBIN=libstdcxxthread


all:$(BOOSTBIN) $(LIBCXXBIN) $(LIBSTDCXXBIN)

$(BOOSTBIN):boostthread.cc
	$(GXX) $(GXXFLAGS) boostthread.cc -o $@  -lboost_thread  -lboost_system

$(LIBCXXBIN):libcxxthread.cc
	$(CXX) $(CXXFLAGS) -stdlib=libc++ libcxxthread.cc -o $@ -lpthread -lc++ -lc++abi

$(LIBSTDCXXBIN):libstdcxxthread.cc
	$(GXX) $(GXXFLAGS)   libstdcxxthread.cc -o $@ -lpthread

clean:
	-rm $(BOOSTBIN) $(LIBCXXBIN) $(LIBSTDCXXBIN)
