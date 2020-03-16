freader.dll: reader.o
	g++ -shared -fPIC -o freader.dll reader.o
reader.o: reader.cpp ../../include/interfacedevicestream.hpp
	g++ -c -fPIC -DBUILDING_PLUGIN reader.cpp

.PHONY: clean
clean:
	rm reader.o reader.dll