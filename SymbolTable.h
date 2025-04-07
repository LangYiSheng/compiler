//
// Created by LanCher on 25-4-5.
//

#pragma once

#include <utility>
#include<vector>
#include<unordered_map>
#include<string>
#include<iostream>

#include "Test/LanCherLogger.h"
using namespace std;


enum SymbolType { SYMBOL_INT, SYMBOL_FLOAT, SYMBOL_FUNCTION };

// 一个符号
struct Symbol {
    string name;

    SymbolType type ; // 0: int, 1: float ,2: function
    bool isUsed = false; // 用于检查未使用的变量
    bool isConst = false; // 是否为常量
    int col, row; // 用于输出错误信息
    int repeat = 0; // 用于局部与全局变量重名的情况
    // only 函数 
    vector<SymbolType> params; // 函数参数列表，检测形参和实参个数是否匹配。 你说得对，虽然可以只用一个数字来表示个数，但是我先用字符串来表示函数参数类型，方便后续扩展
    Symbol() = default;
    // 为了方便变量定义
    Symbol(const string& name, const SymbolType type, const int row, const int col, const bool isConst = false) {
        this->name = name;
        this->type = type;
        this->isConst = isConst;
        this->row = row;
        this->col = col;
    }
    // 为了方便函数定义
    Symbol(const string& name,const vector<Symbol>& params,const int row, const int col) {
        this->name = name;
        this->params.reserve(params.size());
        for (const auto& param : params) {
            this->params.push_back(param.type);
        }
        this->type = SYMBOL_FUNCTION;
        this->row = row;
        this->col = col;
    }
};



// 符号表类，
class SymbolTable {
    vector<unordered_map<string, Symbol>> scopes; // 符号表栈，栈顶为当前作用域

public:
    SymbolTable() {
        enterScope(); // 全局作用域
    }


    // 进入新作用域（压栈）
    void enterScope() {
        // cout<<"[符号表] 进入新作用域"<<endl;
        scopes.push_back({});
    }

    // 退出当前作用域（弹栈）
    void exitScope() {
        // cout<<"[符号表] 退出当前作用域"<<endl;
        if (!scopes.empty()) {
            for (const auto& pair : scopes.back()) {
                const string& name = pair.first;
                const Symbol & sym = pair.second;
                if (!sym.isUsed) {
                    if (sym.isConst)
                        cerr << "语义警告: 常量 '" << name << "' 定义但未使用, 在 (" << sym.row << "," << sym.col << ")\n";
                    else
                        cerr << "语义警告: 变量 '" << name << "' 定义但未使用, 在 (" << sym.row << "," << sym.col << ")\n";
                }
            }
            scopes.pop_back();
        }
    }

    // 添加符号到当前作用域
    bool addSymbol(Symbol symbol) {
        LanCherLogger::log("符号表", static_cast<string>("添加") + (symbol.type == SYMBOL_FUNCTION ? "函数" : symbol.isConst ? "常量" : "变量") + ": " + symbol.name);
        auto& currentScope = scopes.back();
        const string& name = symbol.name;
        if (currentScope.count(name)) {
            const auto& existingSymbol = currentScope[name];
            if (existingSymbol.type == SYMBOL_FUNCTION) {
                cerr << "语义错误: 已存在函数 '" << name << "' ,发生重定义, 原有的位于 (" << existingSymbol.row << "," << existingSymbol.col << "), 新定义的位于 (" << symbol.row << "," << symbol.col << ")\n";
            } else {
                if (existingSymbol.isConst)
                    cerr << "语义警告: 已存在常量 '" << name << "' ,发生重定义, 原有的位于 (" << existingSymbol.row << "," << existingSymbol.col << "), 新定义的位于 (" << symbol.row << "," << symbol.col << ")\n";
                else
                    cerr << "语义警告: 已存在变量 '" << name << "' ,发生重定义, 原有的位于 (" << existingSymbol.row << "," << existingSymbol.col << "), 新定义的位于 (" << symbol.row << "," << symbol.col << ")\n";
            }
            return false;
        }
        // 至此，在本作用域内没有重名的变量
        Symbol* existingSymbol = lookup(name);
        if (existingSymbol == nullptr) {
            // 在全局作用域内没有重名的变量
            symbol.repeat = 0;
        } else {
            // 在全局作用域内有重名的变量
            symbol.repeat = existingSymbol->repeat + 1;
            existingSymbol->repeat++;
        }
        currentScope[name] = symbol;
        return true;
    }

    // 查找符号 返回nullptr表示未找到 会查找到全局作用域 返回最近的
    Symbol* lookup(const string& name) {
        for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
            if (it->count(name)) {
                return &((*it)[name]);
            }
        }
        return nullptr;
    }

    // 标记符号为已使用
    void markUsed(const string& name, const int row, const int col) {
        Symbol* sym = lookup(name);
        if (sym) {
            sym->isUsed = true;
        } else {
            cerr << "语义错误: 使用了未声明的符号 '" << name << "', 在 (" << row << "," << col << ")\n";
        }
    }

    // 检查函数调用
    bool checkFunctionCall(const string& funcName, const vector<SymbolType>& argTypes , const int row, const int col) {
        Symbol* sym = lookup(funcName);
        if (!sym) {
            cerr << "语义错误: 函数 '" << funcName << "' 未定义, 在 (" << row << "," << col << ")\n";
            return false;
        }
        if (sym->type != 2) {
            cerr << "语义错误: '" << funcName << "' 不是函数, 在 (" << row << "," << col << ") ,曾作为 " << (sym->isConst ? "常量" : "变量") << " 定义, 位于 (" << sym->row << "," << sym->col << ")\n";
            return false;
        }
        if (sym->params.size() != argTypes.size()) {
            cerr << "语义错误: 函数 '" << funcName << "' 参数数量不匹配, 期望 " << sym->params.size() << " 个参数, 实际 " << argTypes.size() << " 个参数, 在 (" << row << "," << col << ")\n";
            return false;
        }
        // 类型检查 但是实验没有要求
        // for (size_t i = 0; i < argTypes.size(); ++i) {
        //     if (sym->params[i] != argTypes[i]) {
        //         cerr << "语义错误: 函数 '" << funcName << "' 参数类型不匹配。\n";
        //         return false;
        //     }
        // }
        return true;
    }

};
