CC := g++ -std=c++11 -pedantic
IFLAGS := -I../stb/
SRC := ImgUtils.cpp 

all: lib/libutils.so 

lib/libutils.so: $(SRC)
	$(CC) -fPIC -shared -o $@ $(IFLAGS) $^

clean:
	rm -rf lib/*