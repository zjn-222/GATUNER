//识别half精度，给需要的half添加__half2float()
// double h = b - (a);必须分成两行写！！！！！

//给=左边的变量进行一个标记
//然后将标记以外的half，无脑添加__half2float()
//处理的输入文件一定是原始文件，无half类型的！！！！！！！！！！！！！！！！！



#include "llvm/ADT/APInt.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IRReader/IRReader.h"
//#include "llvm/Support/Host.h"
#include "llvm/Support/Error.h"
#include <llvm/Support/Path.h>
#include "llvm/Support/Casting.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/raw_os_ostream.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/CBindingWrapping.h"
#include "clang/AST/AST.h"
#include "clang/AST/Decl.h"
#include "clang/AST/Expr.h"
#include "clang/AST/DeclBase.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Basic/LangOptions.h"
#include "clang/Basic/SourceManager.h"
#include "clang/CodeGen/ModuleBuilder.h"
#include "clang/Frontend/ASTUnit.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Parse/ParseAST.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Tooling/Refactoring.h"
#include "clang/Tooling/Core/Replacement.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Lex/Lexer.h"
#include "clang/Lex/PreprocessingRecord.h"
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <filesystem>
#include <unordered_set>
#include "clang/Tooling/ArgumentsAdjusters.h" // 包含参数调整器头文件，cuda添加cuda添加cuda添加cuda添加cuda添加cuda添加cuda添加
using namespace std;
using namespace llvm;
using namespace clang;
using namespace clang::driver;
using namespace clang::tooling;
using namespace clang::ast_matchers;


#include "/usr/include/nlohmann/json.hpp"
using json=nlohmann::json;


// //声明文件的存在，这样所有函数都能访问该文件了
// std::ifstream file;
// //results是从Json文件中读取的函数信息
// std::vector<json> results;

json result_json; // 全局变量
json chromosome_json;

std::vector<std::string> library_functions = {
    // 数学函数
    "sin", "cos", "tan", "asin", "acos", "atan", "atan2", "sinh", "cosh", "tanh", "asinh", "acosh", "atanh",
    "exp", "exp2", "expm1", "log", "log10", "log2", "log1p", "sqrt", "cbrt", "hypot",
    "ceil", "floor", "fmod", "trunc", "round", "rint", "nearbyint", "remainder", "remquo",
    "pow", "abs", "fabs", "modf", "frexp", "ldexp", "scalbn", "scalbln", "ilogb", "logb",
    "copysign", "nextafter", "nexttoward", "fdim", "fmax", "fmin", "fpclassify", "isfinite", "isinf", "isnan", "isnormal", "signbit",
    "isgreater", "isgreaterequal", "isless", "islessequal", "islessgreater", "isunordered","tgamma"
};

 std::vector<std::string> IDs; // 声明全局变量

// 遍历所有的变量
class MyVariableASTVisitor1 : public RecursiveASTVisitor<MyVariableASTVisitor1>
{
public:
  MyVariableASTVisitor1(Rewriter &R,ASTContext &context) : TheRewriter(R),TheContext(context) {}



  bool VisitBinaryOperator(BinaryOperator *binaryOp) {
    // llvm::outs() << "第个访问者逻辑---------------------------------------------------------- "  << "\n";
    // 获取操作符的位置
    SourceLocation loc = binaryOp->getExprLoc();
    SourceManager &SM = TheRewriter.getSourceMgr();
    // 过滤非主文件的代码
    if (!SM.isInMainFile(loc)) {
        return true; // 跳过头文件中的操作符
    }

    //获取左节点
    Stmt *left=binaryOp->getLHS();
    if (isa<DeclRefExpr>(left)) {
      DeclRefExpr *declRefExpr = cast<DeclRefExpr>(left);
      // 对DeclRefExpr节点进行处理（例如打印引用的变量名）
      // cout<<"遇到左节点----------------："<<declRefExpr->getNameInfo().getAsString()<<endl;
        // 获取并打印类型
        // QualType type = declRefExpr->getType();
        // cout << "左节点的类型为：" << type.getAsString() << endl;

      // 修改为：
      SourceManager &sm = TheContext.getSourceManager();
    
    // 获取左侧表达式的起始位置
    SourceLocation loc = left->getBeginLoc();
    
    // 检查位置是否有效
    if (loc.isInvalid()) {
        llvm::outs() << "Invalid location\n";
        return true;
    }
    
    // 获取行号和列号（使用拼写位置，即源代码中的原始位置）
    unsigned line = sm.getSpellingLineNumber(loc);
    unsigned column = sm.getSpellingColumnNumber(loc);
    


        ValueDecl *decl = declRefExpr->getDecl();

        QualType type = decl->getType().getCanonicalType();
        string typeStr = type.getAsString();
        string varName = decl->getNameAsString();  // [2](@ref)
        // llvm::outs() << "第一个访问者逻辑---------------------------------------------------------- "  << "\n";

    // 组合成ID并打印
    std::string id = "Line" + std::to_string(line) + "_Col" + std::to_string(column);
    // llvm::outs() << "ID: " << id << "\n";
    // llvm::outs() << "name: " << varName << "\n";

    //添加到数组中,这些是被标记的节点，不可以被替换成__half2float()
    IDs.push_back(id);

    }
    return true;
  }


private:
  Rewriter &TheRewriter;
  ASTContext &TheContext;
};



class MyVariableASTVisitor2 : public RecursiveASTVisitor<MyVariableASTVisitor2> {
public:
  MyVariableASTVisitor2(Rewriter &R,ASTContext &context) : TheRewriter(R),TheContext(context) {}

  bool isIDExist(const std::vector<std::string>& IDs, const std::string& targetID) {
      return std::find(IDs.begin(), IDs.end(), targetID) != IDs.end();
  }

bool VisitDeclRefExpr(DeclRefExpr *expr) {
    SourceManager &SM = TheRewriter.getSourceMgr();
    SourceLocation loc = expr->getLocation();
    
    // 过滤非主文件的引用
    if (!SM.isInMainFile(loc)) return true;

    // 获取AST上下文中的SourceManager
    SourceManager &sm = TheContext.getSourceManager();
    
    // 转换为拼写位置（原始代码位置）
    loc = sm.getSpellingLoc(loc);

    // 检查位置有效性
    if (loc.isInvalid()) {
        // llvm::outs() << "Invalid DeclRefExpr location\n";
        return true;
    }

    // 获取声明和变量名
    if (ValueDecl *decl = expr->getDecl()) {
        // 获取类型信息
        QualType type = decl->getType().getCanonicalType();
        string typeStr = type.getAsString();
        string varName = decl->getNameAsString();  // [2](@ref)


        // 获取行和列
    unsigned line = sm.getSpellingLineNumber(loc);
    unsigned column = sm.getSpellingColumnNumber(loc);
    // 组合成ID并打印
    std::string id = "Line" + std::to_string(line) + "_Col" + std::to_string(column);
    // llvm::outs() << "ID: " << id << "\n";
    // llvm::outs() << "name: " << varName << "\n";

        if (isIDExist(IDs, id)) {
        // 存在时的逻辑
        // llvm::outs() << "发现=左边的DeclRefExpr节点 " ;
            // llvm::outs() << "ID: " << id << "\n";
        // llvm::outs() << "name: " << varName << "\n";
        // llvm::outs() << "跳过它 "<<"\n";
        return true;
    }


          // 增强输出
          // llvm::outs() << "Found DeclRefExpr at " 
          //             << line << ":" << col
          //             << " - Name: " << varName    // 新增变量名输出
          //             << " - Type: " << typeStr 
          //             << "\n";

        // llvm::outs() << "发现half节点 " ;
            //添加处理逻辑
            // 获取变量名和源码范围


      // string varName = decl->getNameAsString();
      SourceLocation startLoc = expr->getBeginLoc();
      SourceLocation endLoc = expr->getEndLoc();

      // 构建替换表达式
      if (chromosome_json[varName]==0&&!(isIDExist(library_functions, varName))) {
      string replacement = "__half2float(" + varName + ")";
      TheRewriter.ReplaceText(SourceRange(startLoc, endLoc), replacement);
                      // llvm::outs() << "已转换变量：" << varName 
                      //      << " -> " << replacement << "\n";
      }

                
                // llvm::outs() << "已转换变量：" << varName 
                //            << " -> " << replacement << "\n";
          
    

        // llvm::outs() << "第二个访问者逻辑---------------------------------------------------------- "  << "\n";


// llvm::outs() << "----------- "  << "\n";
//     for (const auto& id:IDs ) {
//       llvm::outs() << id<<"\n";
//     }
// llvm::outs() << "----------- "  << "\n";



    }
    return true;
}

private:
  Rewriter &TheRewriter;
  ASTContext &TheContext;
};








// ASTConsumer接口的实现，用于读取由Clang分析器产生的AST
class MyASTConsumer : public ASTConsumer {
public:
  MyASTConsumer(Rewriter &R,ASTContext &Context) : TheRewriter(R), Visitor1(R,Context), Visitor2(R,Context) {}
  // 覆盖每个被解析的顶层声明被调用的方法
  bool HandleTopLevelDecl(DeclGroupRef DR) override {
    // 收集所有的顶层声明
    for (DeclGroupRef::iterator b = DR.begin(), e = DR.end(); b != e; ++b) {
      TopLevelDecls.push_back(*b);
    }
    return true;
  }
  void HandleTranslationUnit(ASTContext &Context) override {
    // 第一次遍历
    for (auto *D : TopLevelDecls) {
      Visitor1.TraverseDecl(D);
    }
    // 第二次遍历
    for (auto *D : TopLevelDecls) {
      Visitor2.TraverseDecl(D);
    }
  }
private:
  std::vector<Decl*> TopLevelDecls;
  Rewriter &TheRewriter;
  MyVariableASTVisitor1 Visitor1;
  MyVariableASTVisitor2 Visitor2;
};








// 对于提供给工具的每个源文件，都会创建一个新的FrontendAction
class MyFrontendAction : public ASTFrontendAction {
public:
  MyFrontendAction() {}
  void EndSourceFileAction() override {
    SourceManager &SM = TheRewriter.getSourceMgr();
    TheRewriter.getEditBuffer(SM.getMainFileID()).write(llvm::outs());
  }
  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef file) override {
    TheRewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
// 修改为：
return std::make_unique<MyASTConsumer>(TheRewriter, CI.getASTContext());
  }
private:
  Rewriter TheRewriter;
};










static llvm::cl::OptionCategory ToolingCategory("my-tool options");
static llvm::cl::opt<std::string> ChromosomeNum("chromosome", llvm::cl::desc("Specify the chromosome number"), llvm::cl::Required, llvm::cl::cat(ToolingCategory));

int main(int argc, const char **argv)
{
    //读取ConfigMerged.json文件
    std::ifstream i("/home/zjn/mytool2/src/ConfigMerged.json");
    i >> result_json;
    chromosome_json=result_json["chromosome1"];
    i.close();


    // 解析命令行选项
    auto ExpectedParser = clang::tooling::CommonOptionsParser::create(argc, argv, ToolingCategory);
    if (!ExpectedParser) {
        llvm::errs() << "Error creating options parser\n";
        return 1;
    }
    CommonOptionsParser &OptionsParser = ExpectedParser.get();

    // 读取--chromosome选项的值
    std::string chrom_num = ChromosomeNum.getValue();
    chromosome_json = result_json["chromosome" + chrom_num];

    // 创建ClangTool并运行
    ClangTool Tool(OptionsParser.getCompilations(), OptionsParser.getSourcePathList());

  // 添加CUDA参数调整器，cuda添加cuda添加cuda添加cuda添加cuda添加cuda添加cuda添加
  auto adjuster = getInsertArgumentAdjuster(
      {"-x", "cuda", "--cuda-gpu-arch=sm_86"},  // 设置CUDA架构，如sm_50
      ArgumentInsertPosition::BEGIN);
  Tool.appendArgumentsAdjuster(adjuster);

    return Tool.run(newFrontendActionFactory<MyFrontendAction>().get());
}