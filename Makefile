# Uso:  make test      (no Windows/MinGW: mingw32-make test LDFLAGS=-static)
CXX      ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra
LDFLAGS  ?=

REGEX = 02_regex.cpp
AFN   = 04_afn.cpp
AFD   = 05_afd.cpp

.PHONY: test demo clean

test: 04_afn_teste 05_afd_teste
	./04_afn_teste casos_afn.txt
	./05_afd_teste casos_afn.txt

demo: 05_demo
	./05_demo

04_afn_teste: 04_afn_teste.cpp $(REGEX) $(AFN) 04_afn.h 02_regex.h
	$(CXX) $(CXXFLAGS) $(LDFLAGS) 04_afn_teste.cpp $(REGEX) $(AFN) -o $@

05_afd_teste: 05_afd_teste.cpp $(REGEX) $(AFN) $(AFD) 05_afd.h 04_afn.h 02_regex.h
	$(CXX) $(CXXFLAGS) $(LDFLAGS) 05_afd_teste.cpp $(REGEX) $(AFN) $(AFD) -o $@

05_demo: 05_demo.cpp $(REGEX) $(AFN) $(AFD) 05_afd.h 04_afn.h 02_regex.h
	$(CXX) $(CXXFLAGS) $(LDFLAGS) 05_demo.cpp $(REGEX) $(AFN) $(AFD) -o $@

clean:
	rm -f 04_afn_teste 05_afd_teste 05_demo *.exe
