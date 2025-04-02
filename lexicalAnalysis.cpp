//
// Created by LanCher on 25-3-12.
//

#include "lexicalAnalysis.h"

#include <iostream>
#include <fstream>
#include <unordered_set>
#include <utility>

using namespace std;

unordered_set<string> keywords = {
    "int", "float", "program", "const", "def", "begin", "end", "call",
    "let", "if", "then", "else", "fi", "while", "do", "endwh",
    "return", "and", "or"
};

char currentChar;
int row = 1, col = 0;
ifstream fin;

// 读取一个字符
void getNextChar() {
    if (currentChar == '\n') {
        row++;
        col = 0;
    }
    currentChar = fin.get();
    col++;
}

// 跳过空白字符
void skipBlank() {
    while (isspace(currentChar)) {
        getNextChar();
    }
}

// 跳过注释 （确保此时currentChar为/，在skipBlankAndComment已判断）
void skipComment() {
    getNextChar();
    if (currentChar == '/') {
        while (currentChar != '\n' && currentChar != EOF) {
            getNextChar();
        }
    } else if (currentChar == '*') {
        getNextChar();
        getNextChar();
        while (!(currentChar == '*' && fin.peek() == '/')) {
            if (currentChar == EOF) {
                cerr << "错误: 注释未结束\n";
                return;
            }
            getNextChar();
        }
        getNextChar();
        getNextChar();
    }
}

// 跳过空白字符和注释
void skipBlankAndComment() {
    while (true) {
        if (currentChar=='/'&&(fin.peek()=='/'||fin.peek()=='*')) {
            skipComment();
            continue;
        }
        if (isspace(currentChar)) {
            skipBlank();
            continue;
        }
        break;
    }
}

// 识别标识符/关键字
Token identifyWords() {
    string word;
    int row = ::row, col = ::col;
    while (isalnum(currentChar)||currentChar=='_') {
        word += currentChar;
        getNextChar();
    }
    if (word.length()>32)
        return {ERROR, "标识符最大长度应不超过32个字符", row, col};
    if (keywords.find(word) != keywords.end())
        return {KEYWORD, word, row, col};
    // 标识符不区分大小写
    for (char& c : word)
        c = tolower(c);
    return {IDENTIFIER, word, row, col};
}

// 识别常数
Token identifyNum() {
    string num;
    int row = ::row, col = ::col;
    if (currentChar == '0') {
        num += currentChar;
        getNextChar();
        if (isdigit(currentChar))
            return {ERROR, "整数的首位数字不为0", row, col};
    } else {
        while (isdigit(currentChar)) {
            num += currentChar;
            getNextChar();
        }
    }
    if (currentChar == '.') {
        num += currentChar;
        getNextChar();
        while (isdigit(currentChar)) {
            num += currentChar;
            getNextChar();
        }
        //前后必须都有数字
        if (num.back()=='.')
            return {ERROR, "小数点后必须有数字", row, col};
        return {FLOAT, num, row, col};
    }
    if (num.length()>5||stoi(num) >= 65536) {
        return {ERROR, "整数应在16bits位以内", row, col};
    }
    return {INT, num, row, col};
}

// 识别分隔符
Token identifyDelimiter() {
    string sep;
    int row = ::row, col = ::col;
    sep += currentChar;
    getNextChar();
    return {DELIMITER, sep, row, col};
}

// 识别四则运算符
Token identifyArithmetic() {
    string arith;
    int row = ::row, col = ::col;
    arith += currentChar;
    getNextChar();
    return {ARITHMETIC, arith, row, col};
}

// 识别比较运算符和赋值运算符
Token identifyCompare() {
    string comp;
    int row = ::row, col = ::col;
    comp += currentChar;
    getNextChar();
    if (currentChar == '='||currentChar == '>') {
        comp += currentChar;
        getNextChar();
    }
    if (comp == "=")
        return {ASSIGN, comp, row, col};
    return {COMPARE, comp, row, col};
}

// 获取下一个单词
Token getNextToken() {
    skipBlankAndComment();
    if (currentChar == EOF)
        return {ERROR, "EOF", row, col};
    if (isalpha(currentChar))
        return identifyWords();
    if (isdigit(currentChar))
        return identifyNum();
    if (currentChar=='('||currentChar==')'||currentChar==';'||currentChar==',')
        return identifyDelimiter();
    if (currentChar=='+'||currentChar=='-'||currentChar=='*'||currentChar=='/')
        return identifyArithmetic();
    if (currentChar=='='||currentChar=='>'||currentChar=='<')
        return identifyCompare();
    char c = currentChar;
    getNextChar();
    return {ERROR, "未知字符"+string(1, c), row, col};
}

// 输出单词
void printToken(const Token& token) {
    if (token.type==ERROR) {
        cerr << "词法错误: " << token.value << " 在 (" << token.row << "," << token.col <<')'<< endl;
        return;
    }
    cout<<"("<<token.type<<","<<token.value<<","<<token.row<<","<<token.col<<")"<<endl;
}

// 初始化词法分析器
bool initLexicalAnalysis(const char* filename) {
    fin.open(filename);
    if (!fin.is_open()) {
        return false;
    }
    getNextChar();
    return true;
}

// 关闭词法分析器
void closeLexicalAnalysis() {
    fin.close();
}