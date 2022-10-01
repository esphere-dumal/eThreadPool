#include <iostream>
#include "threadPool.hh"

void func1(int id) {
    std::cout<< "[cout from func1]:" << id <<std::endl;
}

void func2(int id) {
    std::cout<< "[cout from func2]:" << id <<std::endl;
}

const size_t kSize   = 8;
const size_t kNumber = 114514;

int main() {
    ThreadPool tp(kSize);

    for(size_t i=0; i<kNumber; i++) {
        if(i%2) tp.enqueue(func1, i);
        else tp.enqueue(func2, i);
    }

    return 0;
}