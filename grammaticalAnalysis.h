//
// Created by LanCher on 25-3-12.
//

#pragma once
#include "lexicalAnalysis.h"
#include <iostream>

#include "SymbolTable.h"
#include "QuadrupleGenerator.h"

//语法分析器 采用递归下降分析
class Parser {
    struct TempVar {
        int type = 0; // 0为整数，1为小数，2为目标标识符的名字，3为临时变量名
        string var = "";

    };
    QuadrupleGenerator generator;// 四元式生成器
    SymbolTable symbolTable;// 符号表
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
        const SymbolType type = Type();
        Symbol s = VarDeclaration(); s.type = type; // 获取常量声明
        symbolTable.addSymbol(s);
        while (currentToken.type == DELIMITER && currentToken.value == ",") {
            match(DELIMITER,",");
            s = VarDeclaration(); s.type = type; // 获取常量声明
            symbolTable.addSymbol(s);
        }
        match(DELIMITER,";");
    }

    Symbol VarDeclaration() {
        // 常量声明 -> 标识符 = 数字(整数或小数)
        string varName = currentToken.value;
        int row = currentToken.row, col = currentToken.col;
        match(IDENTIFIER);
        match(ASSIGN);
        if (currentToken.type == INT || currentToken.type == FLOAT) {
            match(currentToken.type);
        } else {
            cerr << "语法错误: 期望整数或小数但得到 " << currentToken.value << " 在 (" << currentToken.row << "," << currentToken.col <<')'<< endl;
        }
        return {varName,SYMBOL_INT, row, col, true};
    }

    void VarDefinition() {
        // 定义变量 -> 类型 变量名 { , 变量名 } ;
        const SymbolType type = Type();
        symbolTable.addSymbol(Symbol(currentToken.value,type,currentToken.row,currentToken.col)); // 添加变量到符号表
        match(IDENTIFIER);
        while (currentToken.type == DELIMITER && currentToken.value == ",") {
            match(DELIMITER,",");
            symbolTable.addSymbol(Symbol(currentToken.value,type,currentToken.row,currentToken.col));
            match(IDENTIFIER);
        }
        match(DELIMITER,";");
    }

    SymbolType Type() {
        SymbolType type = SYMBOL_INT; // 防止出错，默认类型为int
        // 数据类型 -> int | float
        if (currentToken.value == "int" || currentToken.value == "float") {
            type = currentToken.value == "int" ? SYMBOL_INT : SYMBOL_FLOAT;
            match(KEYWORD);
        } else {
            cerr << "语法错误: 期望类型 'int' 或 'float' 但得到 " << currentToken.value  << " 在 (" << currentToken.row << "," << currentToken.col <<')'<< endl;
        }
        return type;
    }

    void Method() {
        // 定义函数 -> def 标识符 ( 参数列表 ) 复合语句
        match(KEYWORD,"def");
        string funcName = currentToken.value; // 函数名
        int row = currentToken.row, col = currentToken.col;
        match(IDENTIFIER);
        match(DELIMITER,"(");
        vector<Symbol> params = ParamList();
        symbolTable.addSymbol(Symbol(funcName,params,row,col)); // 添加函数到符号表
        match(DELIMITER,")");
        generator.add("procedure","","",funcName); // 生成函数开始的四元式
        CompSt(params);
        generator.add("return","","",""); // 生成函数结束的四元式 隐式返回
    }

    vector<Symbol> ParamList() {
        vector<Symbol> params;
        // 形参参数列表 -> 类型 标识符 { , 类型 标识符 }
        if (currentToken.type == KEYWORD && (currentToken.value == "int" || currentToken.value == "float")) {
            SymbolType type = Type();
            params.emplace_back(currentToken.value,type,currentToken.row,currentToken.col); // 添加参数到符号表
            match(IDENTIFIER);
            while (currentToken.type == DELIMITER && currentToken.value == ",") {
                match(DELIMITER,",");
                type = Type();
                params.emplace_back(currentToken.value,type,currentToken.row,currentToken.col); // 添加参数到符号表
                match(IDENTIFIER);
            }
        }
        return params;
    }

    void CompSt(const vector<Symbol> & params = {}) {
        // 复合语句块 -> begin 语句列表 end
        match(KEYWORD,"begin");
        symbolTable.enterScope(); // 进入新作用域
        for (const auto& param : params) {
            symbolTable.addSymbol(param); // 添加参数到符号表
        } // 参数需要在函数体内定义
        StmtList();
        symbolTable.exitScope(); // 退出作用域
        match(KEYWORD,"end");
    }

    void StmtList() {
        // 语句列表 -> 语句 语句列表|空
        while (currentToken.type != KEYWORD || currentToken.value != "end") {
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
        SymbolType type = Type();
        symbolTable.addSymbol(Symbol(currentToken.value,type,currentToken.row,currentToken.col)); // 添加变量到符号表
        match(IDENTIFIER);
        while (currentToken.type == DELIMITER && currentToken.value == ",") {
            match(DELIMITER,",");
            symbolTable.addSymbol(Symbol(currentToken.value,type,currentToken.row,currentToken.col));
            match(IDENTIFIER);
        }
        match(DELIMITER,";");
    }

    void CallStmt() {
        // 函数调用语句 -> call 标识符 ( 实参列表 ) ;
        int row = currentToken.row, col = currentToken.col;
        match(KEYWORD,"call");
        string funcName = currentToken.value;
        match(IDENTIFIER);
        match(DELIMITER,"(");
        vector<pair<string,SymbolType>> args = ActParamList();
        for (int i = static_cast<int>(args.size())-1; i >= 0; --i) {
            generator.add("push", "", "", args[i].first);
        }
        // 这里的args[i].first是参数名，args[i].second是参数类型，为了不改checkFunctionCall的参数类型，使用一些语法糖
        symbolTable.checkFunctionCall(funcName,
            [&] {
                vector<SymbolType> t;
                for(auto &p:args)
                    t.push_back(p.second);
                return t;
        }(), row,col);
        generator.add("call", "", "", funcName); // 生成函数调用的四元式
        match(DELIMITER,")");
        match(DELIMITER,";");
    }

    vector<pair<string,SymbolType>> ActParamList() {
        // 实参列表 -> 表达式 { , 表达式 } | 空
        vector<pair<string,SymbolType>> args;
        if (currentToken.type == IDENTIFIER || currentToken.type == INT || currentToken.type == FLOAT) {
             TempVar tv = Exp();
             args.emplace_back(tv.var, tv.type==0?SYMBOL_INT:SYMBOL_FLOAT);
             while (currentToken.type == DELIMITER && currentToken.value == ",") {
                 match(DELIMITER,",");
                 TempVar tv2 = Exp();
                 args.emplace_back(tv2.var, tv2.type==0?SYMBOL_INT:SYMBOL_FLOAT);
             }
         }
        return args;
    }

    void AssignmentStmt() {
        // 赋值语句 -> let 标识符 = 表达式 ;
        match(KEYWORD,"let");
        string id = currentToken.value;
        match(IDENTIFIER);
        match(ASSIGN);
        TempVar exp = Exp();
        generator.add("=", exp.var, "", id);
        match(DELIMITER,";");
    }

    void ConditionalStmt() {
        // 条件语句 -> if 条件表达式 then 语句 [ else 语句 ] fi

        match(KEYWORD, "if");
        int elseLabel = generator.newLabel();  // 条件不满足跳转
        ConditionalExp(elseLabel);
        match(KEYWORD, "then");
        Stmt();
        if (currentToken.type == KEYWORD && currentToken.value == "else") {
            match(KEYWORD);
            // 有 else 分支，才需要跳出 if 的 endLabel
            int endLabel = generator.newLabel();
            generator.addJump("","","",endLabel); // 跳出 if 的结束标签
            generator.addLabel(elseLabel);
            Stmt();  // else 分支语句
            generator.addLabel(endLabel);
        } else {
            // 没有 else，直接贴上 elseLabel 作为结束点
            generator.addLabel(elseLabel);
        }
        match(KEYWORD, "fi");
    }

    void LoopStmt() {
        // 循环语句 -> while 条件表达式 do 语句 endwh
        match(KEYWORD,"while");
        int startLabel = generator.newLabel(); // 循环跳回
        int endLabel = generator.newLabel(); // 结束

        generator.addLabel(startLabel);

        ConditionalExp(endLabel);

        match(KEYWORD,"do");
        Stmt();

        generator.addJump("","","",startLabel);

        generator.addLabel(endLabel);
        match(KEYWORD,"endwh");
    }

    void ReturnStmt() {
        match(KEYWORD,"return");
        generator.add("return","","",""); // 生成函数结束的四元式 显式返回
        match(DELIMITER,";");
    }

    TempVar Exp() {
        // 表达式 -> 项 { + 项 | - 项 }
        TempVar left = Term();
        while (currentToken.type == ARITHMETIC && (currentToken.value == "+" || currentToken.value == "-")) {
            string op = currentToken.value;
            match(ARITHMETIC);
            TempVar right = Term();
            string temp = generator.newTemp();
            generator.add(op, left.var, right.var, temp);
            left = {3, temp};
        }
        return left;
    }

    TempVar Term() {
        // 项 -> 因子 { * 因子 | / 因子 }
        TempVar left = Factor();
        while (currentToken.type == ARITHMETIC && (currentToken.value == "*" || currentToken.value == "/")) {
            string op = currentToken.value;
            match(ARITHMETIC);
            TempVar right = Factor();
            string temp = generator.newTemp(); // 创建一个新的临时变量
            generator.add(op, left.var, right.var, temp); // 生成四元式
            left = {3, temp};  // 3表示临时变量
        }
        return left;
    }

    TempVar Factor() {
        // 因子 -> 标识符 | 数字 | ( 表达式 )
        TempVar tempVar;
        if (currentToken.type == IDENTIFIER) {
            tempVar = {2, currentToken.value}; // 2表示标识符
            symbolTable.markUsed(currentToken.value,currentToken.row,currentToken.col); // 标记变量为已使用
            match(IDENTIFIER);
        } else if (currentToken.type == INT || currentToken.type == FLOAT) {
            tempVar = {currentToken.type == INT ? 0 : 1, currentToken.value}; // 0表示整数，1表示小数
            match(currentToken.type);
        } else if (currentToken.type == DELIMITER && currentToken.value == "(") {
            match(DELIMITER,"(");
            tempVar = Exp();
            match(DELIMITER,")");
        } else {
            cerr << "语法错误: 无效的因子 " << currentToken.value << " 在 (" << currentToken.row << "," << currentToken.col <<')'<< endl;
        }
        return tempVar;
    }

    void ConditionalExp(int falseLabel) {
        // 条件表达式 -> 关系表达式 {or 关系表达式}
        int trueLabel = generator.newLabel();
        int orLabel = generator.newLabel();
        //如果RelationExp为真，跳到正确的分支，如果为假，则接着执行后续的or
        RelationExp(orLabel);
        generator.addJump("", "", "", trueLabel); // 没跳转，说明条件为真，跳到trueLabel
        while (currentToken.type == KEYWORD && currentToken.value == "or") {
            // 这里的orLabel是上一个or的标签，打上标签并开新的标签
            match(KEYWORD, "or");
            generator.addLabel(orLabel);
            orLabel = generator.newLabel();
            RelationExp(orLabel);
            generator.addJump("", "", "", trueLabel);  // 没跳转，说明条件为真，跳到trueLabel
        }
        generator.addLabel(orLabel);
        generator.addJump("","","",falseLabel);
        // 如果为真，跳到trueLabel，继续执行真代码块
        generator.addLabel(trueLabel);
    }

    void RelationExp(int falseLabel) {
        // 关系表达式 -> 组合表达式 {and 组合表达式}
        // 如果没有跳转到falseLabel，说明条件为真，继续执行
        int trueLabel = generator.newLabel();
        CompExp(trueLabel);// 如果为真，则继续执行
        generator.addJump("", "", "", falseLabel); // 如果为假，跳到falseLabel
        while (currentToken.type == KEYWORD && currentToken.value == "and") {
            match(KEYWORD, "and");
            generator.addLabel(trueLabel);
            trueLabel = generator.newLabel();
            CompExp(trueLabel);
            generator.addJump("", "", "", falseLabel); // 如果为假，跳到falseLabel
        }
        generator.addLabel(trueLabel);
    }

    void CompExp(int trueLabel) {
        // 组合表达式 -> 表达式 关系符 表达式
        // 如果没有跳转到trueLabel，说明条件为假（继续执行的为假）
        TempVar left = Exp();
        string op = currentToken.value;
        CmpOp();
        TempVar right = Exp();
        generator.addJump(op, left.var, right.var, trueLabel);
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
    vector<Quadruple> parse() {
        nextToken();
        Program();
        // 如果没有到文件末尾，说明语法错误
        if (currentToken.type != ERROR || currentToken.value != "EOF") {
            cerr << "语法错误: 期望文件结束但得到 " << currentToken.value << " 在 (" << currentToken.row << "," << currentToken.col <<')'<< endl;
        }
        cout << "[喜报] 四元式生成完成" << endl;
        return generator.getQuadruples();
    }
};
