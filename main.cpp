#include <iostream>
#include "threadPool.h"

void foo1(int id){
    std::cout<<id<<std::endl;
}
void foo2(int id){
    printf("%d\n",id);
}

int main(){
    ThreadPool tp(4);
    for(size_t i=0;i<10;i++) {
        if(i%2) tp.enqueue(foo1, i);
        else tp.enqueue(foo2, i);
    }
    return 0;
}