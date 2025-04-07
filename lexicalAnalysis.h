//
// Created by LanCher on 25-3-12.
// 简单的词法分析器实现，提供getNextToken函数，返回下一个单词
//
#pragma once

#include <unordered_set>
#include <utility>
#include <fstream>
using namespace std;
// 关键字集合
extern unordered_set<string> keywords;
// 单词类别
enum Type
{
    // 关键字, 标识符, 整数,小数,分隔符,四则运算符,比较运算符,赋值运算符
    ERROR,
    KEYWORD,
    IDENTIFIER,
    INT,
    FLOAT,
    DELIMITER,
    ARITHMETIC,
    COMPARE,
    ASSIGN
};
// 输出单词名
inline string TypeToString(const Type type)
{
    switch (type)
    {
    case KEYWORD:
        return "关键字";
    case IDENTIFIER:
        return "标识符";
    case INT:
        return "整数";
    case FLOAT:
        return "小数";
    case DELIMITER:
        return "分隔符";
    case ARITHMETIC:
        return "算术运算符";
    case COMPARE:
        return "比较运算符";
    case ASSIGN:
        return "赋值运算符";
    case ERROR:
        return "错误";
    default:
        return "未知类型";
    }
}
// 单词
struct Token
{
    Type type;
    string value;
    int row, col;
    Token(const Type type, string value, const int row, const int col) : type(type), value(std::move(value)), row(row), col(col) {}
    Token() {}
};
// 在使用前需要全局初始化词法分析器，最后关闭
bool initLexicalAnalysis(const char *filename);
void closeLexicalAnalysis();
Token getNextToken();
void printToken(const Token &token);