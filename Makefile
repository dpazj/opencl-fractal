
CXX = g++

CFLAGS = -Wall
CPPFLAGS = -std=c++14 

LDFLAGS = -lOpenCL
OBJS=fract.o \
	lodepng.o

%.o: %.cpp $(DEPS)
	$(CXX) -c -o $@ $< $(CPPFLAGS)

fract: $(OBJS)
	$(CXX) $(OBJS) -o fract $(LDFLAGS)

run:
	./fract

clean: 
	rm -rf $(OBJS) fract