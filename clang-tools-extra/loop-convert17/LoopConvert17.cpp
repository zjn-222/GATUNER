//识别half精度，给需要的half添加__half2float()
// double h = b - (a);必须分成两行写！！！！！

//处理逻辑:检测half变量，然后添加__half2float(),检测如果“=”右边全部都是half，则不添加__half2float()


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
// 遍历所有的变量
class MyVariableASTVisitor : public RecursiveASTVisitor<MyVariableASTVisitor>
{
public:
  MyVariableASTVisitor(Rewriter &R) : TheRewriter(R) {}

// bool VisitDeclRefExpr(DeclRefExpr *expr) {
//     SourceManager &SM = TheRewriter.getSourceMgr();
//     SourceLocation loc = expr->getLocation();
    
//     // 过滤非主文件的引用
//     if (!SM.isInMainFile(loc)) return true;

//     // 获取声明和变量名
//     if (ValueDecl *decl = expr->getDecl()) {
//         // 获取类型信息
//         QualType type = decl->getType().getCanonicalType();
//         string typeStr = type.getAsString();
//         string varName = decl->getNameAsString();  // [2](@ref)

//         // 获取源码位置
//         unsigned line = SM.getExpansionLineNumber(loc);
//         unsigned col = SM.getExpansionColumnNumber(loc);

//         if (typeStr=="struct __half") {
//           // 增强输出
//           llvm::outs() << "Found DeclRefExpr at " 
//                       << line << ":" << col
//                       << " - Name: " << varName    // 新增变量名输出
//                       << " - Type: " << typeStr 
//                       << "\n";
//         }

//     }
//     return true;
// }
  bool VisitBinaryOperator(BinaryOperator *binaryOp) {
    // 获取操作符的位置
    SourceLocation loc = binaryOp->getExprLoc();
    SourceManager &SM = TheRewriter.getSourceMgr();
    // 过滤非主文件的代码
    if (!SM.isInMainFile(loc)) {
        return true; // 跳过头文件中的操作符
    }

    //获取左节点
    Stmt *left=binaryOp->getLHS();
    if (isa<DeclRefExpr>(left)||isa<UnaryOperator>(left)) {
      DeclRefExpr *declRefExpr = cast<DeclRefExpr>(left);
      // 对DeclRefExpr节点进行处理（例如打印引用的变量名）
      // cout<<"遇到左节点----------------："<<declRefExpr->getNameInfo().getAsString()<<endl;
        // 获取并打印类型
        QualType type = declRefExpr->getType();
        // cout << "左节点的类型为：" << type.getAsString() << endl;


    // 定义一个flag，
    // 0表示全half，没有double等,不需要管
    // 1表示检测到非Half的类型，需要添加__half2float()
      Stmt *right=binaryOp->getRHS();
      int flag=0;
      // printf("处理右节点\n");
      find_DeclRefExpr(right,SM,loc,flag);
      if (flag==1) {
        // printf("这个=的flag为1--------------------------------\n");
        handle_half(binaryOp->getRHS());
        // printf("进行一次half替换\n");
      }
      else {
        // printf("这个=的flag为0--------------------------------\n");
      }
    }




    return true;
  }


private:
  Rewriter &TheRewriter;

//查找=右边部分的DeclRefExpr节点，有没有非half类型的变量
  bool find_DeclRefExpr(Stmt *right,SourceManager &SM,SourceLocation loc,int &flag) {
    if (isa<DeclRefExpr>(right)) {
      DeclRefExpr *declRefExpr = cast<DeclRefExpr>(right);
      // 对DeclRefExpr节点进行处理（例如打印引用的变量名）

      // 获取声明和变量名
      if (ValueDecl *decl = declRefExpr->getDecl()) {
          // 获取类型信息
          QualType type = decl->getType().getCanonicalType();
          string typeStr = type.getAsString();
          string varName = decl->getNameAsString();  // [2](@ref)

          // 获取源码位置
          unsigned line = SM.getExpansionLineNumber(loc);
          unsigned col = SM.getExpansionColumnNumber(loc);

          if (typeStr=="struct __half"||typeStr == "__half") {
            // llvm::outs() << "发现half节点 " 
                        // << line << ":" << col
                        // << " - Name: " << varName    // 新增变量名输出
                        // << " - Type: " << typeStr 
                        // << "\n";
          }
          else if(
            typeStr == "double" || 
            typeStr == "float" || 
            typeStr == "int" || 
            typeStr == "bool" || 
            typeStr == "char" || 
            typeStr == "dim3" ||                   // CUDA线程块/网格维度类型
            typeStr == "float2" || typeStr == "float4" ||   // CUDA向量类型
            typeStr == "double2" || typeStr == "int4" || 
            typeStr == "cudaError_t" ||            // CUDA错误类型（本质是枚举）
            typeStr == "cudaError" ||              // 枚举类型（如cudaSuccess的底层类型）
            typeStr == "cudaStream_t" ||           // CUDA流句柄（通常是结构体指针）
            typeStr == "cudaEvent_t" ||            // CUDA事件句柄
            typeStr == "thrust::device_vector<int>" ||   // Thrust模板实例化类型
            typeStr == "thrust::complex<float>" || 
            typeStr == "float*" || typeStr == "int**"     // 指针类型
          ) {
            flag=1;
            // llvm::outs() << "发现别的非half节点 " 
            //             << line << ":" << col
            //             << " - Name: " << varName    // 新增变量名输出
            //             << " - Type: " << typeStr 
            //             << "\n";
          }

      }
      // else llvm::outs() <<"获取变量声明失败"<<"\n";
    }
    // 递归访问当前语句的所有子语句
    else {
      // llvm::outs() <<"不是DeclRefExpr节点"<<"\n";
      for (Stmt::child_iterator ci = right->child_begin(), ce = right->child_end(); ci != ce; ++ci) {
        // cout<<"进入一层递归"<<endl;
        find_DeclRefExpr(*ci,SM,loc,flag);
        // cout<<"离开一层递归"<<endl;
      }
    }

  return true;
  }

//将此=节点右边部分的half变量添加__half2float()
// bool handle_half(Stmt *right) {
//     if (isa<DeclRefExpr>(right)) {
//         DeclRefExpr* declRefExpr = cast<DeclRefExpr>(right);
//         if (ValueDecl* decl = declRefExpr->getDecl()) {
//             QualType type = decl->getType().getCanonicalType();
//             string typeStr = type.getAsString();
            
//             if (typeStr == "struct __half") {
//                 // 获取变量名和源码范围
//                 string varName = decl->getNameAsString();
//                 SourceLocation startLoc = declRefExpr->getBeginLoc();
//                 SourceLocation endLoc = declRefExpr->getEndLoc();
                
//                 // 避免重复转换（检查父节点是否已经是转换函数）
//                 if (!isa<CallExpr>(declRefExpr->getParent())) {
//                     // 构建替换表达式
//                     string replacement = "__half2float(" + varName + ")";
//                     TheRewriter.ReplaceText(SourceRange(startLoc, endLoc), replacement);
                    
//                     llvm::outs() << "已转换变量：" << varName 
//                                << " -> " << replacement << "\n";
//                 }
//             }
//         }
//     }
//     // 递归处理子节点
//     for (auto child : right->children()) {
//         if (child) handle_half(child);
//     }
//     return true;
// }
  bool handle_half(Stmt *right) {
    if (isa<DeclRefExpr>(right)) {
      DeclRefExpr *declRefExpr = cast<DeclRefExpr>(right);
      // 对DeclRefExpr节点进行处理（例如打印引用的变量名）

      // 获取声明和变量名
      if (ValueDecl *decl = declRefExpr->getDecl()) {
          // 获取类型信息
          QualType type = decl->getType().getCanonicalType();
          string typeStr = type.getAsString();
          string varName = decl->getNameAsString();  // [2](@ref)

          if (typeStr=="struct __half") {
            // llvm::outs() << "发现half节点 " ;
                //添加处理逻辑
                // 获取变量名和源码范围
                string varName = decl->getNameAsString();
                SourceLocation startLoc = declRefExpr->getBeginLoc();
                SourceLocation endLoc = declRefExpr->getEndLoc();
                    // 构建替换表达式
                    string replacement = "__half2float(" + varName + ")";
                    TheRewriter.ReplaceText(SourceRange(startLoc, endLoc), replacement);
                    
                    // llvm::outs() << "已转换变量：" << varName 
                    //            << " -> " << replacement << "\n";
          }
      }
      // else llvm::outs() <<"获取变量声明失败"<<"\n";
    }
    // 递归访问当前语句的所有子语句
    else {
      // llvm::outs() <<"不是DeclRefExpr节点"<<"\n";
      for (Stmt::child_iterator ci = right->child_begin(), ce = right->child_end(); ci != ce; ++ci) {
        // cout<<"进入一层递归"<<endl;
        handle_half(*ci);
        // cout<<"离开一层递归"<<endl;
      }
    }

  return true;
  }
};


// ASTConsumer接口的实现，用于读取由Clang分析器产生的AST
class MyASTConsumer : public ASTConsumer
{
public:
  MyASTConsumer(Rewriter &R) : Visitor(R) {}
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
  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef file) override
  {
    TheRewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
    // 从该函数传到MyASTConsumer类里面TheRewriter
    return std::make_unique<MyASTConsumer>(TheRewriter);
  }
private:
  Rewriter TheRewriter;
};
int main(int argc, const char **argv)
{
  llvm::cl::OptionCategory ToolingCategory("my-tool options");
  auto ExpectedParser = clang::tooling::CommonOptionsParser::create(argc, argv, ToolingCategory);
  CommonOptionsParser &OptionsParser = ExpectedParser.get();

  
  ClangTool Tool(OptionsParser.getCompilations(), OptionsParser.getSourcePathList());

  // 添加CUDA参数调整器，cuda添加cuda添加cuda添加cuda添加cuda添加cuda添加cuda添加
  auto adjuster = getInsertArgumentAdjuster(
      {"-x", "cuda", "--cuda-gpu-arch=sm_86"},  // 设置CUDA架构，如sm_50
      ArgumentInsertPosition::BEGIN);
  Tool.appendArgumentsAdjuster(adjuster);

  return Tool.run(newFrontendActionFactory<MyFrontendAction>().get());
}