all: start

start: a.out
	./a.out 1:50 2:30 3:40 4:10

.PHONY: clean format

a.out: main.cpp
	g++ -std=c++17 -g3 -Wall main.cpp

test.out: main.cpp
	g++ -std=c++17 -DUNIT_TEST=1 -g3 -Wall main.cpp -o test.out

test: test.out
	./test.out

format:
	clang-format -i main.cpp

clean:
	-rm -f a.out test.out
