bin=HttpServer
cc=g++ -std=c++11
$(bin):HttpServer.cc
	$(cc) -o $@ $^ -lpthread #-D_DEBUG_

.PHONY:clean
clean:
	rm -f $(bin)
