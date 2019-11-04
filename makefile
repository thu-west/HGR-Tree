CC = clang++
CXXFLAGS += -std=c++11 -w -O3

all: topk
.PHONY: clean

topk = main.o \
ds/Index.o\
utils/utils.o\
utils/munkres.o\
utils/matrix.o\
ds/InvertedList.o\
ds/QGramer.o\
algo/search.o\

topk: $(topk)
	$(CC) -o $@ $^

%.o: %.cpp
	$(CC) $(CXXFLAGS) -c -o $@ $<

clean:
	find . -type f -name '*.o' -delete
	rm topk
