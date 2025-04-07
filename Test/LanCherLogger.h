//
// Created by LanCher on 25-4-7.
//
#pragma once
#include<iostream>
using namespace std;

class LanCherLogger {
public:
    static void log(const string& tag, const string& message) {
        cout << "[" << tag << "] " << message << endl;
    }
};