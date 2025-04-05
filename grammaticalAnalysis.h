//
// Created by LanCher on 25-3-12.
//

#pragma once
#include "lexicalAnalysis.h"
#include <iostream>
//语法分析器 采用递归下降分析
class Parser {
    Token currentToken;

    void nextToken() {
        currentToken = getNextToken();
    }

    void match(const Type expectedType,const string& expectedValue = "") {
        // match可选的参数expectedValue，用于对特定的值进行强匹配
        if (currentToken.type == expectedType && (expectedValue.empty() || currentToken.value == expectedValue)) {
            nextToken();
        } else {
            // 如果是ERROR类型，说明词法分析出错，输出此法错误信息，否则输出语法错误信息
            if (currentToken.type==ERROR) {
                if (currentToken.value=="EOF") {
                    cerr <<"语法错误: 期望 "<<(expectedValue.empty() ? TypeToString(expectedType) : expectedValue)
                    <<" 但遇到突发的文件结束在 ("<< currentToken.row << "," << currentToken.col <<')'<< endl;
                    exit(1);
                }
                cerr << "词法错误: " << currentToken.value << " 在 (" << currentToken.row << "," << currentToken.col <<')'<< endl;
                return;
            }
            cerr << "语法错误: 期望 " <<  (expectedValue.empty() ? TypeToString(expectedType) : expectedValue) << " 但得到 " << currentToken.value
                 << " 在 (" << currentToken.row << "," << currentToken.col <<')'<< endl;
            nextToken();  // 跳过错误 Token，防止死循环
        }
    }

    void Program() {
        // 程序 -> program 标识符 ; 程序体
        match(KEYWORD,"program");
        match(IDENTIFIER);
        match(DELIMITER,";");
        Procedures();
    }

    void Procedures() {
        // 程序体 -> 定义体 程序体|空
        // 定义体的first集合为{const,int,float,def},当判断到这些单词时，调用Definition()函数
        while (currentToken.type == KEYWORD && (currentToken.value == "const" ||
                                                currentToken.value == "int" ||
                                                currentToken.value == "float" ||
                                                currentToken.value == "def")) {
            Definition();
        }
    }

    void Definition() {
        // 定义 -> const 定义常量 | 定义变量 | 定义函数
        if (currentToken.value == "const") {
            ConstDefinition();
        } else if (currentToken.value == "int" || currentToken.value == "float") {
            VarDefinition();
        } else if (currentToken.value == "def") {
            Method();
        } else {
            cerr << "语法错误: 无效的定义 " << currentToken.value << " 在 (" << currentToken.row << "," << currentToken.col <<')'<< endl;
        }
    }

    void ConstDefinition() {
        // 定义常量 -> const 类型 常量声明 { , 常量声明 } ;
        match(KEYWORD,"const");
        Type();
        VarDeclaration();
        while (currentToken.type == DELIMITER && currentToken.value == ",") {
            match(DELIMITER,",");
            VarDeclaration();
        }
        match(DELIMITER,";");
    }

    void VarDeclaration() {
        // 常量声明 -> 标识符 = 数字(整数或小数)
        match(IDENTIFIER);
        match(ASSIGN);
        if (currentToken.type == INT || currentToken.type == FLOAT) {
            match(currentToken.type);
        } else {
            cerr << "语法错误: 期望整数或小数但得到 " << currentToken.value << " 在 (" << currentToken.row << "," << currentToken.col <<')'<< endl;
        }
    }

    void VarDefinition() {
        // 定义变量 -> 类型 变量声明 { , 变量声明 } ;
        Type();
        match(IDENTIFIER);
        while (currentToken.type == DELIMITER && currentToken.value == ",") {
            match(DELIMITER,",");
            match(IDENTIFIER);
        }
        match(DELIMITER,";");
    }

    void Type() {
        // 数据类型 -> int | float
        if (currentToken.value == "int" || currentToken.value == "float") {
            match(KEYWORD);
        } else {
            cerr << "语法错误: 期望类型 'int' 或 'float' 但得到 " << currentToken.value  << " 在 (" << currentToken.row << "," << currentToken.col <<')'<< endl;
        }
    }

    void Method() {
        // 定义函数 -> def 标识符 ( 参数列表 ) 复合语句
        match(KEYWORD,"def");
        match(IDENTIFIER);
        match(DELIMITER,"(");
        ParamList();
        match(DELIMITER,")");
        CompSt();
    }

    void ParamList() {
        // 形参参数列表 -> 类型 标识符 { , 类型 标识符 }
        if (currentToken.type == KEYWORD && (currentToken.value == "int" || currentToken.value == "float")) {
            Type();
            match(IDENTIFIER);
            while (currentToken.type == DELIMITER && currentToken.value == ",") {
                match(DELIMITER,",");
                Type();
                match(IDENTIFIER);
            }
        }
    }

    void CompSt() {
        // 复合语句块 -> begin 语句列表 end
        match(KEYWORD,"begin");
        StmtList();
        match(KEYWORD,"end");
    }

    void StmtList() {
        // 语句列表 -> 语句 语句列表|空
        while (currentToken.type != KEYWORD || (currentToken.value != "end")) {
            Stmt();
            if (currentToken.type == ERROR) {  // 防止死循环
                if (currentToken.value == "EOF")
                    return;
                nextToken();
            }
        }
    }

    void Stmt() {
        // 语句 -> 条件语句|循环语句|调用语句|赋值语句|返回语句|局部变量定义语句|复合语句块|分号
        if (currentToken.value == "if") {
            ConditionalStmt();
        } else if (currentToken.value == "while") {
            LoopStmt();
        } else if (currentToken.value == "call") {
            CallStmt();
        } else if (currentToken.value == "let") {
            AssignmentStmt();
        } else if (currentToken.value == "return") {
            ReturnStmt();
        }else if (currentToken.value == "int" || currentToken.value == "float") {
            LocalVariableDeclaration();
        }else if (currentToken.value == "begin") {
            CompSt();
        } else if (currentToken.type == DELIMITER && currentToken.value == ";") {
            match(DELIMITER,";");
        }
        else {
            cerr << "语法错误: 无效的语句 " << currentToken.value << " 在 (" << currentToken.row << "," << currentToken.col <<')'<< endl;
            nextToken();
        }
    }

    void LocalVariableDeclaration() {
        // 局部变量定义语句 -> 类型 变量声明 { , 变量声明 } ;
        Type();
        match(IDENTIFIER);
        while (currentToken.type == DELIMITER && currentToken.value == ",") {
            match(DELIMITER,",");
            match(IDENTIFIER);
        }
        match(DELIMITER,";");
    }

    void CallStmt() {
        // 函数调用语句 -> call 标识符 ( 实参列表 ) ;
        match(KEYWORD,"call");
        match(IDENTIFIER);
        match(DELIMITER,"(");
        ActParamList();
        match(DELIMITER,")");
        match(DELIMITER,";");
    }

    void ActParamList() {
        // 实参列表 -> 表达式 { , 表达式 } | 空
        if (currentToken.type == IDENTIFIER || currentToken.type == INT || currentToken.type == FLOAT) {
            Exp();
            while (currentToken.type == DELIMITER && currentToken.value == ",") {
                match(DELIMITER,",");
                Exp();
            }
        }
    }

    void AssignmentStmt() {
        // 赋值语句 -> let 标识符 = 表达式 ;
        match(KEYWORD,"let");
        match(IDENTIFIER);
        match(ASSIGN);
        Exp();
        match(DELIMITER,";");
    }

    void ConditionalStmt() {
        // 条件语句 -> if 条件表达式 then 语句 [ else 语句 ] fi
        match(KEYWORD,"if");
        ConditionalExp();
        match(KEYWORD,"then");
        Stmt();
        if (currentToken.type == KEYWORD&&currentToken.value == "else") {
            match(KEYWORD);
            Stmt();
        }
        match(KEYWORD,"fi");
    }

    void LoopStmt() {
        // 循环语句 -> while 条件表达式 do 语句 endwh
        match(KEYWORD,"while");
        ConditionalExp();
        match(KEYWORD,"do");
        Stmt();
        match(KEYWORD,"endwh");
    }

    void ReturnStmt() {
        match(KEYWORD,"return");
        match(DELIMITER,";");
    }

    void Exp() {
        // 表达式 -> 项 { + 项 | - 项 }
        Term();
        while (currentToken.type == ARITHMETIC && (currentToken.value == "+" || currentToken.value == "-")) {
            match(ARITHMETIC);
            Term();
        }
    }

    void Term() {
        // 项 -> 因子 { * 因子 | / 因子 }
        Factor();
        while (currentToken.type == ARITHMETIC && (currentToken.value == "*" || currentToken.value == "/")) {
            match(ARITHMETIC);
            Factor();
        }
    }

    void Factor() {
        // 因子 -> 标识符 | 数字 | ( 表达式 )
        if (currentToken.type == IDENTIFIER) {
            match(IDENTIFIER);
        } else if (currentToken.type == INT || currentToken.type == FLOAT) {
            match(currentToken.type);
        } else if (currentToken.type == DELIMITER && currentToken.value == "(") {
            match(DELIMITER,"(");
            Exp();
            match(DELIMITER,")");
        } else {
            cerr << "语法错误: 无效的因子 " << currentToken.value << " 在 (" << currentToken.row << "," << currentToken.col <<')'<< endl;
        }
    }

    void ConditionalExp() {
        // 条件表达式 -> 关系表达式 {or 关系表达式}
        RelationExp();
        while (currentToken.type == KEYWORD && currentToken.value == "or") {
            match(KEYWORD,"or");
            RelationExp();
        }
    }

    void RelationExp() {
        // 关系表达式 -> {组合表达式 and 组合表达式}
        CompExp();
        while (currentToken.type == KEYWORD && currentToken.value == "and") {
            match(KEYWORD,"and");
            CompExp();
        }
    }

    void CompExp() {
        // 组合表达式 -> 表达式 关系符 表达式
        Exp();
        CmpOp();
        Exp();
    }

    void CmpOp() {
        // 关系符 -> < | > | <= | >= | == | <>
        if (currentToken.value == "<" || currentToken.value == ">" || currentToken.value == "<=" ||
            currentToken.value == ">=" || currentToken.value == "==" || currentToken.value == "<>") {
            match(COMPARE);
        } else {
            cerr << "语法错误: 无效的关系符 " << currentToken.value << " 在 (" << currentToken.row << "," << currentToken.col <<')'<< endl;
        }
    }

public:
    //解析
    void parse() {
        nextToken();
        Program();
        // 如果没有到文件末尾，说明语法错误
        if (currentToken.type != ERROR || currentToken.value != "EOF") {
            cerr << "语法错误: 期望文件结束但得到 " << currentToken.value << " 在 (" << currentToken.row << "," << currentToken.col <<')'<< endl;
        }
    }
};
