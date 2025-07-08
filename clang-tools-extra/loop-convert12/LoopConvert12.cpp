//处理文件的函数信息，生成temp.json，里面包含函数名与函数return的对应关系

#include "llvm/ADT/APInt.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IRReader/IRReader.h"
// #include "llvm/Support/Host.h"
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

#include "/usr/include/nlohmann/json.hpp"    //666666666666666666
using json = nlohmann::json;                //666666666666666666

using namespace std;
using namespace llvm;
using namespace clang;
using namespace clang::driver;
using namespace clang::tooling;
using namespace clang::ast_matchers;

// 遍历所有的变量
class MyVariableASTVisitor : public RecursiveASTVisitor<MyVariableASTVisitor> {
public:
    MyVariableASTVisitor(Rewriter &R, std::vector<json> &results) : TheRewriter(R), results(results) {}//666666666666666666

    bool VisitFunctionDecl(FunctionDecl *FD) {
      SourceManager &SM = FD->getASTContext().getSourceManager();
      if (!SM.isInMainFile(FD->getLocation()))
      {
          // 如果不在主文件中，跳过该变量节点
          return true;
      }
        // 获取函数名称
        std::string functionName = FD->getNameInfo().getName().getAsString();
        std::cout << "Function name: " << functionName << std::endl;

        //获取函数形参
        std::vector<std::string> xingcan;
        for (unsigned i = 0; i < FD->getNumParams(); ++i) {
            ParmVarDecl *Param = FD->getParamDecl(i);
            QualType QT = Param->getType();
            if (QT->isSpecificBuiltinType(BuiltinType::Double)) {
              xingcan.push_back(Param->getNameAsString());
            }
        }

        // 找到返回的变量名
        // 进入CompoundStmt语句
        Stmt *functionBody = FD->getBody();
        CompoundStmt *CS = dyn_cast<CompoundStmt>(functionBody);
        std::string returnVariableName = findReturnVariable(CS);


        // 将结果存储在 json 对象中
        json result;           //666666666666666666
        result["function_name"] = functionName;//666666666666666666
        result["return_name"] = returnVariableName;//666666666666666666
        result["if_delete"]=false;
        result["xingcan"]=xingcan;
        results.push_back(result);//666666666666666666

        return true;
    }

private:
    Rewriter &TheRewriter;
    std::vector<json> &results;//666666666666666666

    std::string findReturnVariable(CompoundStmt *CS) {
      for (Stmt *child:CS->body()) {
        if (isa<ReturnStmt>(child)) {
          Stmt* stmt=dyn_cast<Stmt>(child);
          std::string name=find_DeclRefExpr(stmt);
          return find_DeclRefExpr(stmt);
        }
      }
    }


  std::string find_DeclRefExpr(Stmt *stmt) {
    std::string s="";
    if (isa<DeclRefExpr>(stmt)) {
      DeclRefExpr *declRefExpr = cast<DeclRefExpr>(stmt);
      // 对DeclRefExpr节点进行处理（例如打印引用的变量名）
      s=declRefExpr->getNameInfo().getAsString();
    }
    else {
      // 递归访问当前语句的所有子语句
      for (Stmt::child_iterator ci = stmt->child_begin(), ce = stmt->child_end(); ci != ce; ++ci) {
        std::string childNotes = find_DeclRefExpr(*ci);
        s=childNotes;
      }
    }
    return s;
  }
};




// ASTConsumer 接口的实现，用于读取由 Clang 分析器产生的 AST
class MyASTConsumer : public ASTConsumer {
public:
    MyASTConsumer(Rewriter &R, std::vector<json> &results) : Visitor(R, results) {}//666666666666666666

    // 覆盖每个被解析的顶层声明被调用的方法
    bool HandleTopLevelDecl(DeclGroupRef DR) override {
        for (DeclGroupRef::iterator b = DR.begin(), e = DR.end(); b != e; ++b) {
            // 使用我们的AST访问者遍历声明
            Visitor.TraverseDecl(*b);
        }
        return true;
    }

private:
    MyVariableASTVisitor Visitor;
};

// 对于提供给工具的每个源文件，都会创建一个新的FrontendAction
class MyFrontendAction : public ASTFrontendAction {
public:
    MyFrontendAction(std::vector<json> &results) : results(results) {}//666666666666666666

    void EndSourceFileAction() override {
        SourceManager &SM = TheRewriter.getSourceMgr();
        TheRewriter.getEditBuffer(SM.getMainFileID()).write(llvm::outs());
    }

    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef file) override {
        TheRewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
        return std::make_unique<MyASTConsumer>(TheRewriter, results);//666666666666666666
    }

private:
    Rewriter TheRewriter;
    std::vector<json> &results;//666666666666666666
};



//666666666666666666
// 自定义的FrontendActionFactory来传递参数
class MyFrontendActionFactory : public FrontendActionFactory {//666666666666666666
public://666666666666666666
    MyFrontendActionFactory(std::vector<json> &results) : results(results) {}//666666666666666666

    std::unique_ptr<FrontendAction> create() override {//666666666666666666
        return std::make_unique<MyFrontendAction>(results);//666666666666666666
    }

private://666666666666666666
    std::vector<json> &results;//666666666666666666
};
//666666666666666666



int main(int argc, const char **argv) {
    std::vector<json> results;  // 用于保存收集的结果//666666666666666666

    llvm::cl::OptionCategory ToolingCategory("my-tool options");
    auto ExpectedParser = clang::tooling::CommonOptionsParser::create(argc, argv, ToolingCategory);
    if (!ExpectedParser) {
        llvm::errs() << ExpectedParser.takeError();
        return 1;
    }
    CommonOptionsParser &OptionsParser = ExpectedParser.get();
    ClangTool Tool(OptionsParser.getCompilations(), OptionsParser.getSourcePathList());

    // 自定义FrontendActionFactory实例
    MyFrontendActionFactory Factory(results);//666666666666666666

    // 运行工具并获取结果
    int ToolResult = Tool.run(&Factory);//666666666666666666

    // 将结果写入 JSON 文件
    std::ofstream outputFile("/home/zjn/mytool/temp.json");//666666666666666666
    outputFile << json(results).dump(4);//666666666666666666
    outputFile.close();//666666666666666666

    return ToolResult;//666666666666666666
}