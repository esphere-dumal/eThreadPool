main:
	clang++ -o main src/main.cc -lpthread

clean:
	rm main

.PHONY: clean