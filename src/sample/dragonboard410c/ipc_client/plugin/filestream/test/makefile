test.exe: main.o
	g++ -g -o test.exe main.o -ldl
main.o: main.cpp  ../../include/interfacedevicestream.hpp
	g++ -g -c main.cpp

.PHONY: clean
clean:
	rm main.o test.exe