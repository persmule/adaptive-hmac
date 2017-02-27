test-spopen: test-spopen.o ../lib/spopen.cpp.o
	gcc -o $@ $< $(word 2,$^) -lstdc++

test-basepair: test-basepair.o
	gcc -o $@ $< $(word 2,$^)

test-filter: test-filter.o ../lib/spopen.cpp.o ../lib/filter.o ../lib/rcv_concat.o
	gcc -o $@ $< $(word 2,$^) $(word 3,$^) $(word 4,$^) -lstdc++

%.cpp.o: %.cpp
	g++ -o $@ -Wall -g -I. -I../include -c $<

%.o: %.c
	gcc -o $@ -Wall -g -I. -I../include -c $<
