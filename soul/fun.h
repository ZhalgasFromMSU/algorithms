#pragma once
#include <iostream>

//Recursion thta always stops at 91 if arg is lesser than 102
//or returns arg - 10
uint64_t rec_91(uint64_t n) {
    if (n > 100) {
        return n - 10;
    } else {
        return m(m(n + 11));
    }
}
