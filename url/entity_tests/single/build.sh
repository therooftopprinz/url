export TD=../../..
g++ -I $TD -std=c++14 -ggdb3 -O0 -c sender.cpp -o sender.o
g++ -I $TD -std=c++14 -ggdb3 -O0 -c receiver.cpp -o receiver.o
g++ -I $TD -std=c++14 -ggdb3 -O0 -c channel_quality_sim.cpp -o channel_quality_sim.o
g++ sender.o $TD/bin/url.a -lpthread -o sender
g++ receiver.o $TD/bin/url.a -lpthread -o receiver
g++ channel_quality_sim.o $TD/bin/url.a -lpthread -o channel_quality_sim