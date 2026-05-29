//
// Created by William Zhao on 4/6/26.
//

#include "pch.h"
#include "Physics.h"

int main(int arc, char* argv[]){
    bool physics = Physics::UnitTest();
    if (!physics){
        DbgAssert(false, "Physics Unit Tests failed.");
        return -1;
    }
    return 0;
}