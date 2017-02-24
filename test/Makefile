test-spopen: test-spopen.o ../lib/spopen.cpp.o
	gcc -o $@ $< $(word 2,$^) -lstdc++


%.cpp.o: %.cpp
	g++ -o $@ -Wall -g -I. -I../include -c $<

%.o: %.c
	gcc -o $@ -Wall -g -I. -I../include -c $<
