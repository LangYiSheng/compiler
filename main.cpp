#include"lexicalAnalysis.h"
#include"grammaticalAnalysis.h"
#include <iostream>
#include <fstream>
using namespace std;
// 主函数
int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "参数错误: " << argv[0] << " <文件名>\n";
        return 1;
    }
    if (!initLexicalAnalysis(argv[1])) {
        cerr << "打开文件失败: " << argv[1] << endl;
        return 1;
    }

    Parser parser;
    vector<Quadruple> quadruples = parser.parse();
    for (int i = 0; i < quadruples.size(); ++i) {
        cout<<i+1<<" "<<quadruples[i]<<";"<<endl;
    }
    closeLexicalAnalysis();
    return 0;
}

