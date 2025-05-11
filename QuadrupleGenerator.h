//
// Created by LanCher on 25-4-7.
//

#pragma once
#include <string>
#include <vector>
#include <sstream>

using namespace std;

// 四元式结构
struct Quadruple {
    string op;      // 操作符
    string arg1;    // 第一个操作数
    string arg2;    // 第二个操作数
    string result;  // 结果

    Quadruple(string op, string arg1, string arg2, string result)
        : op(std::move(op)), arg1(std::move(arg1)), arg2(std::move(arg2)), result(std::move(result)) {}

    friend ostream& operator<<(ostream& os, const Quadruple& quad) {
        os << "(" << quad.op << ", " << quad.arg1 << ", " << quad.arg2 << ", " << quad.result << ")";
        return os;
    }

};

// 四元式生成器
class QuadrupleGenerator {
    vector<Quadruple> quadruples;  // 存储所有四元式
    int tempVarCounter = -1;        // 临时变量计数器
    int labelCounter = 0;          // 标签计数器

public:
    // 生成新的临时变量
    string newTemp() {
        return "t" + to_string(++tempVarCounter);
    }

    // 生成新的标签 用于if/while等跳转
    string newLabel() {
        return "L" + to_string(++labelCounter);
    }

    // 添加四元式
    void add(const string& op, const string& arg1, const string& arg2, const string& result) {
        quadruples.emplace_back(op, arg1, arg2, result);
    }

    // 获取最后生成的临时变量
    string getLastTemp() const {
        return "t" + to_string(tempVarCounter);
    }

    // 获取当前四元式的索引
    int nextQuad() const {
        return quadruples.size();
    }

    // 获取所有四元式
    const vector<Quadruple>& getQuadruples() const {
        return quadruples;
    }

    // 回填四元式的跳转地址
    void backPatch(int quadIndex, const string& label) {
        if (quadIndex >= 0 && quadIndex < quadruples.size()) {
            quadruples[quadIndex].result = label;
        }
    }
};