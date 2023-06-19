all:
	g++ -Wall -DWITH_OPENSSL -DWITH_DOM -O0 -g3 -D_DEBUG -o ./bin/main.o -c ./src/main.cpp -I ./include/
	g++ -Wall -DWITH_OPENSSL -DWITH_DOM -O0 -g3 -D_DEBUG -o ./bin/ISAPI.o -c ./src/ISAPI.cpp -I ./include/
	g++ -Wall -DWITH_OPENSSL -DWITH_DOM -O0 -g3 -D_DEBUG -o ./bin/INIParser.o -c ./src/INIParser.cpp -I ./include/
	g++  -o ./testOnvifD ./bin/main.o ./bin/ISAPI.o ./bin/INIParser.o -L. -lcurl

clean:
	rm ./bin/main.o
	