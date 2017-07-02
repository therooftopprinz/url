export TD=../../../..
g++ -I $TD -std=c++14 -ggdb3 -O0 -c sender.cpp -o sender.o
g++ -I $TD -std=c++14 -ggdb3 -O0 -c receiver.cpp -o receiver.o
g++ sender.o $TD/bin/url.a -lpthread -o sender
g++ receiver.o $TD/bin/url.a -lpthread -o receiver