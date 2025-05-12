//
// Created by LanCher on 25-4-7.
//

#pragma once
#include <queue>
#include<algorithm>
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

    static void printQuadruples(const vector<Quadruple>& quadruples) {
        for (int i = 0; i < quadruples.size(); ++i) {
            cout<<i+1<<" "<<quadruples[i]<<";"<<endl;
        }
    }

    static vector<Quadruple> resolveLabels(
        const vector<Quadruple>& quads
    ) {
        size_t n = quads.size();
        // 1. 去掉 label 并计算每条指令的新行号映射
        vector<int> newLine(n, 0);
        int cnt = 0;
        for (size_t i = 0; i < n; ++i) {
            if (quads[i].op != "label") {
                ++cnt;
                newLine[i] = cnt;
            }
        }

        // 2. 构造 label->行号 映射
        unordered_map<string,int> labelTarget;
        for (size_t i = 0; i < n; ++i) {
            if (quads[i].op == "label") {
                string lab = quads[i].result;
                size_t j = i + 1;
                while (j < n && quads[j].op == "label") ++j;
                if (j < n) labelTarget[lab] = newLine[j];
            }
        }

        // 3. 去掉 label，修 jump->行号
        vector<Quadruple> res;
        res.reserve(cnt);
        for (size_t i = 0; i < n; ++i) {
            const auto& q = quads[i];
            if (q.op == "label") continue;
            Quadruple tmp = q;
            if (!tmp.op.empty() && tmp.op[0]=='j' && !tmp.result.empty()) {
                auto it = labelTarget.find(tmp.result);
                if (it != labelTarget.end())
                    tmp.result = to_string(it->second);
            }
            res.push_back(move(tmp));
        }

        return res;
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
    int newLabel() {
        return ++labelCounter;
    }

    // 添加跳转四元式
    void addJump(const string &relop, // relop为比较运算符,可以为<,>,<=,>=,==,!=,也可以为空字符串
                 const string &arg1,
                 const string &arg2,
                 int targetLabel) {
        // 四元式：(j<, a, b, 3)
        quadruples.emplace_back("j" + relop, arg1, arg2, std::to_string(targetLabel));
    }

    void addLabel(int label) {
        // 你可以把它当做一条特别的四元式，也可以单独存标签表
        quadruples.emplace_back("label", "", "", std::to_string(label));
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