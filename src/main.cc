#include <iostream>
#include "threadPool.hh"

void func1(int id) {
    std::cout<< "[cout from func1]:" << id <<std::endl;
}

void func2(int id, int ids) {
    std::cout<< "[cout from func2]:" << id << "and i have 2 args " << ids <<std::endl;
}

int add(int a, int b) {
    return a+b;
}

const size_t kSize   = 8;
const size_t kNumber = 11;

int main() {
    ThreadPool tp(kSize);
    // no return value func
    for(size_t i=0; i<kNumber; i++) {
        if(i%2) tp.enqueue(func1, i);
        else tp.enqueue(func2, i, i);
    }
    // return value func
    auto res = tp.enqueue(add, 1, 2);
    std::cout << res.get() << std::endl;

    return 0;
}