#pragma once
#include <iostream>
using namespace std;

void error_message(string msg) {
    cerr << msg << endl;
    exit(EXIT_FAILURE);
}