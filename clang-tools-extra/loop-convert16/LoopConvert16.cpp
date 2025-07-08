// loop-convert16   根据精度配置生成对应的混合精度程序
// 输入： 源程序文件路径
// 固定输入：一个精度json文件  、变量列表
// 输出：对应的混合精度cpp文件


//可以替换变量，也可以替换sin等库函数
//可以接受参数，读取某个染色体chromosomeN来生成对应的程序
//配个run.sh实现依次生成多个混合精度程序

//按照配置要求，无脑替换为half,float,double


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



// 遍历所有的变量
class MyVariableASTVisitor : public RecursiveASTVisitor<MyVariableASTVisitor>
{
public:
    MyVariableASTVisitor(ASTContext &Ctx, Rewriter &R)
        : Context(Ctx), TheRewriter(R) {}  // 初始化列表
  // 访问变量节点
  bool VisitVarDecl(VarDecl *Variable)
  {
    SourceManager &SM = Variable->getASTContext().getSourceManager();
    // 需要跳过头文件中的变量，有一个是否在主文件中的函数可以判断变量是否为用户文件的变量
    if (!SM.isInMainFile(Variable->getLocation()))
    {
        // 如果不在主文件中，跳过该变量节点
        return true;
    }
    QualType VarType = Variable->getType();
    // llvm::outs() << "Variable is: " << Variable->getDeclName() << "\n";
    // llvm::outs() << "Variable Type: " << VarType.getAsString() << "\n";
    replaceVariable(Variable);
    return true;
  }
  // 新增函数调用处理
  bool VisitCallExpr(CallExpr *Call) {
      if (FunctionDecl *FuncDecl = Call->getDirectCallee()) {
          std::string FuncName = FuncDecl->getNameAsString();
          // 现在可以直接使用Context成员
          replaceFunctionCall(Call, FuncName);
      }
      return true;
  }

private:
  Rewriter &TheRewriter;
      ASTContext &Context;  // 新增成员
  // 替换变量类型
  void replaceVariable(VarDecl *Variable)
  {
    QualType VarType = Variable->getType();
    if (VarType.getTypePtr()->isSpecificBuiltinType(BuiltinType::Double)||VarType.getTypePtr()->isSpecificBuiltinType(BuiltinType::Float))
    {
      std::string VarName = Variable->getDeclName().getAsString();
        SourceLocation VariableTypeLoc = Variable->getBeginLoc().getLocWithOffset(-2);
        TypeSourceInfo *TInfo = Variable->getTypeSourceInfo();
        SourceRange TypeRange = TInfo->getTypeLoc().getLocalSourceRange();
        
        if (chromosome_json[VarName]==2) {
          TheRewriter.ReplaceText(TypeRange, "double");
        }
        else if (chromosome_json[VarName]==1) {
          TheRewriter.ReplaceText(TypeRange, "float");
        }
        else if (chromosome_json[VarName]==0) {
          TheRewriter.ReplaceText(TypeRange, "half");
        }
        // else cout<<"ConfigMerged.json没有变量:"<<VarName<<endl;

    }
  }


  // 新增函数调用替换逻辑
  void replaceFunctionCall(CallExpr *Call, const std::string &FuncName) {
    // 只处理在配置中声明的函数
    if (!chromosome_json.contains(FuncName)) return;

    // 获取精度配置（1表示float，2表示double）
    int precision = chromosome_json[FuncName];
    
    if (precision == 1) {
      // 生成新的函数名（添加'f'后缀）
      std::string NewName = FuncName + "f";
      
      // 获取函数名在源码中的位置
      SourceRange NameRange = getFunctionNameRange(Call);
      if (NameRange.isValid()) {
        TheRewriter.ReplaceText(NameRange, NewName);
      }
    }
  }

SourceRange getFunctionNameRange(CallExpr *Call) {
    // 直接使用CallExpr的源码范围
    return Call->getCallee()->getSourceRange();
}


};


// ASTConsumer接口的实现，用于读取由Clang分析器产生的AST
class MyASTConsumer : public ASTConsumer
{
public:
    MyASTConsumer(ASTContext &Ctx, Rewriter &R)
        : Visitor(Ctx, R) {}  // 传入ASTContext
  // 覆盖每个被解析的顶层声明被调用的方法
  bool HandleTopLevelDecl(DeclGroupRef DR) override
  {
    for (DeclGroupRef::iterator b = DR.begin(), e = DR.end(); b != e; ++b)
    {
      // 使用我们的AST访问者遍历声明
      Visitor.TraverseDecl(*b);
    }
    return true;
  }
private:
  MyVariableASTVisitor Visitor;
};


// 对于提供给工具的每个源文件，都会创建一个新的FrontendAction
class MyFrontendAction : public ASTFrontendAction
{
public:
  MyFrontendAction() {}
  // 类里面的两个输出语句可以不要
  void EndSourceFileAction() override
  {
    SourceManager &SM = TheRewriter.getSourceMgr();
    // llvm::errs() << "** EndSourceFileAction for: "
    //              << SM.getFileEntryForID(SM.getMainFileID())->getName() << "\n";
    // 现在发出重写的缓冲区
    TheRewriter.getEditBuffer(SM.getMainFileID()).write(llvm::outs());
  }

    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef) override {
        TheRewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
        // 传递CompilerInstance的ASTContext
        return std::make_unique<MyASTConsumer>(CI.getASTContext(), TheRewriter);
    }
private:
  Rewriter TheRewriter;
};

static llvm::cl::OptionCategory ToolingCategory("my-tool options");
static llvm::cl::opt<std::string> ChromosomeNum("chromosome", llvm::cl::desc("Specify the chromosome number"), llvm::cl::Required, llvm::cl::cat(ToolingCategory));

int main(int argc, const char **argv)
{

    //读取ConfigMerged.json文件
    std::ifstream i("/home/zjn/mytool4/src/ConfigMerged.json");
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
