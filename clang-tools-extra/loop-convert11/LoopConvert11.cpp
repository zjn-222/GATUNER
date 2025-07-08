// 匹配到CompoudStmt节点的一级子节点




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
#include <unordered_map>
#include <unordered_set>

#include "/usr/include/nlohmann/json.hpp"
using json=nlohmann::json;

using namespace std;
using namespace llvm;
using namespace clang;
using namespace clang::driver;
using namespace clang::tooling;
using namespace clang::ast_matchers;



std::ifstream file;

//results是从Json文件中读取的函数信息
std::vector<json> results;


// 定义 Edge 结构体
struct Edge {
    std::string src;
    std::string dest;
    int weight;
};

//定于表示"函数的信息"结构体
struct FunInformation {
  std::string callname;
  std::string returnname;
  std::vector<std::string> shican;
  std::vector<std::string> xingcan;
};

struct note_and_edges {
    std::vector<std::string> note;
    std::vector<Edge> edges;  
};

//存储每个函数执行一次时，边的信息
std::unordered_map<std::string,std::vector<Edge>> once_fun_edges;

// 定义一个包含所有库函数名称的集合
std::unordered_set<std::string> library_functions = {
    // 数学函数
    "sin", "cos", "tan", "asin", "acos", "atan", "atan2", "sinh", "cosh", "tanh", "asinh", "acosh", "atanh",
    "exp", "exp2", "expm1", "log", "log10", "log2", "log1p", "sqrt", "cbrt", "hypot",
    "ceil", "floor", "fmod", "trunc", "round", "rint", "nearbyint", "remainder", "remquo",
    "pow", "abs", "fabs", "modf", "frexp", "ldexp", "scalbn", "scalbln", "ilogb", "logb",
    "copysign", "nextafter", "nexttoward", "fdim", "fmax", "fmin", "fpclassify", "isfinite", "isinf", "isnan", "isnormal", "signbit",
    "isgreater", "isgreaterequal", "isless", "islessequal", "islessgreater", "isunordered","tgamma"
};

class MyVariableASTVisitor2 : public RecursiveASTVisitor<MyVariableASTVisitor2>
{
public:
  MyVariableASTVisitor2(Rewriter &R) : TheRewriter(R) {}


  bool VisitFunctionDecl(FunctionDecl *f) {
      SourceManager &SM = f->getASTContext().getSourceManager();
      if (!SM.isInMainFile(f->getLocation()))
      {
          // 如果不在主文件中，跳过该变量节点
          return true;
      }
      std::vector<Edge> s1;
      if (f->getNameAsString()=="main") {
        llvm::outs() << "FunctionDecl: " << f->getNameAsString() << "\n";
        // Get the body of the function (CompoundStmt)
        if (CompoundStmt *CS = dyn_cast<CompoundStmt>(f->getBody())) {
          s1=HandleFirstLevelChildren(CS);
        }
      }



      //写入csv文件
      std::ofstream file("/home/zjn/mytool/edges.csv");
      if (file.is_open()) {
          file << "source,destination,weight\n";
          for (const auto& edge : s1) {
              file << edge.src << "," << edge.dest << "," << edge.weight << "\n";
          }
          file.close();
      } else {
          std::cerr << "Unable to open file";
      }
      cout<<"~~~~~~~~~~~~~~~~~~~~~~~~~第二次遍历完成！~~~~~~~~~~~~~~~~~~~~~~~"<<endl;
      return true; // returning true to continue the traversal
  }


  //分析每一个Edge，把形如sin:a:b,b,1这一个Edge给分解成4个Edge
  // std::vector<Edge> unfold(std::vector<Edge> s1) {
  //   std::vector<Edge> s2;
  //   for (const auto& edge : s1) {
  //     // 检查 src有：
  //     if (edge.src.find(':') != std::string::npos) {
  //         std::vector<std::string> splitSrc = splitString(edge.src, ':');
  //         s2.push_back({splitSrc[0],edge.dest,edge.weight});
  //         int i=1;
  //         int j=1;
  //         for (const auto&item1 : splitSrc) {
  //           for (const auto&item2 : splitSrc) {
  //             if (j>i) {
  //               s2.push_back({item1,item2,edge.weight});
  //             }
  //             j++;
  //           }
  //           i++;
  //           j=1;
  //         }
  //     }
  //     // 检查 dest有：
  //     else if (edge.dest.find(':') != std::string::npos) {
  //         std::vector<std::string> splitDest = splitString(edge.dest, ':');
  //         s2.push_back({edge.src,splitDest[0],edge.weight});
  //         int i=1;
  //         int j=1;
  //         for (const auto&item1 : splitDest) {
  //           for (const auto&item2 : splitDest) {
  //             if (j>i) {
  //               s2.push_back({item1,item2,edge.weight});
  //             }
  //             j++;
  //           }
  //           i++;
  //           j=1;
  //         }
  //     }
  //     //该边不含: 
  //     else {
  //       s2.push_back({edge.src,edge.dest,edge.weight});
  //     }
  //   }
  //   return s2;
  // }

  // // 函数：将字符串按':'分割并存储到 std::vector<std::string> 中
  // std::vector<std::string> splitString(const std::string& str, char delimiter) {
  //     std::vector<std::string> result;
  //     std::stringstream ss(str);
  //     std::string item;

  //     while (std::getline(ss, item, delimiter)) {
  //         result.push_back(item);
  //     }

  //     return result;
  // }

  //返回的是合并后的(s1=fun(a)+fun(b),把RIGHT=fun,fun合并为RIGHT=fun)，函数CompoundStmt内部的边信息
  std::vector<Edge> HandleFirstLevelChildren(CompoundStmt *CS) {
    std::vector<Edge> s1={};

    //用prevNote记录child的上一个节点，为forStmt做准备
    Stmt* prevNote=nullptr;


    for (Stmt *child : CS->body()) {
        if (isa<BinaryOperator>(child)) {
          cout<<"进入BinaryOperator节点"<<endl;
          std::vector<Edge> s=external_BinaryOperator(cast<BinaryOperator>(child),1);
          cout<<"打印该BinaryOperator节点得到的边信息"<<endl;
          for (const auto& edge : s) {
            cout<<"src:"<<edge.src<<",dest:"<<edge.dest<<",weight:"<<edge.weight<<endl;
          }
          cout<<"该BinaryOperator节点处理结束"<<endl;
          for (const auto& edge : s) {
              s1.push_back({edge.src,edge.dest,edge.weight});
          }
        }
        else if (isa<ForStmt>(child)) {
          cout<<"进入ForStmt节点"<<endl;
          std::vector<Edge> s=external_ForStmt(cast<ForStmt>(child),1,prevNote);
          cout<<"打印该ForStmt节点得到的边信息"<<endl;
          for (const auto& edge : s) {
            cout<<"src:"<<edge.src<<",dest:"<<edge.dest<<",weight:"<<edge.weight<<endl;
          }
          cout<<"该ForStmt节点处理结束"<<endl;
          for (const auto& edge : s) {
              s1.push_back({edge.src,edge.dest,edge.weight});
          }
        }
        prevNote=child;
    }
    return s1;
  }

private:
  Rewriter &TheRewriter;

  void listFirstLevelChildren(CompoundStmt *CS) {
      for (Stmt *child : CS->body()) {  // 这是一个范围for循环（range-based for loop），
                                      //  用于遍历 CS->body() 返回的所有子语句。在每次迭代中，child 将指向当前的子语句。
          if (isa<DeclStmt>(child)) {   //这个代码段用于检查 child 是否是一个声明语句（DeclStmt）的实例
              llvm::outs() << " |-DeclStmt\n";
          } else if (isa<BinaryOperator>(child)) {
              llvm::outs() << " |-BinaryOperator\n";
          } else if (isa<ReturnStmt>(child)) {
              llvm::outs() << " |-ReturnStmt\n";
          } else {
              llvm::outs() << " |-Other Stmt\n";
          }
      }
  }



  std::vector<Edge> external_ForStmt(ForStmt* forOp,int multiple,Stmt* prevNote) {
    //获取该for循环的循环次数multiple2
    BinaryOperator *prev=dyn_cast<BinaryOperator>(prevNote);
    Stmt* rhs = prev->getRHS();
    int multiple2;
    // 检查右侧操作数是否为 IntegerLiteral，并获取其值
    if (isa<IntegerLiteral>(rhs)) {
        IntegerLiteral* intLiteral = cast<IntegerLiteral>(rhs);
        int64_t value = intLiteral->getValue().getSExtValue();
        std::cout << "获取当前for循环的次数成功: " << value << std::endl;
        multiple2=value;
    } else {
        std::cout << "右侧操作数不是 IntegerLiteral,获取当前for循环的次数失败" << std::endl;
    }


     // 进入CompoundStmt语句
    CompoundStmt *CS = dyn_cast<CompoundStmt>(forOp->getBody());
    std::vector<Edge> s=multiple_HandleFirstLevelChildren(CS,multiple2);

    //把结果扩大multiple倍
    std::vector<Edge> S1;
    for (const auto& edge:s) {
      S1.push_back({edge.src,edge.dest,(edge.weight*multiple)});
    }
    return S1;
  }

  std::vector<Edge> multiple_HandleFirstLevelChildren(CompoundStmt *CS,int multiple2) {
    //用S1来累计结果
    std::vector<Edge> S1;

    //用prevNote记录child的上一个节点，为forStmt做准备
    Stmt* prevNote=nullptr;
    
    //遍历节点
    for (Stmt *child : CS->body()) {
      //遇到BinaryOperator节点
      if (isa<BinaryOperator>(child)) {
        BinaryOperator* binaryOp=dyn_cast<BinaryOperator>(child);
        std::vector<Edge> temp=external_BinaryOperator(binaryOp,multiple2);
        for (const auto& edge:temp) {
          S1.push_back({edge.src,edge.dest,edge.weight});
        }
      }
      //遇到ForStmt节点
      else if (isa<ForStmt>(child)) {
        ForStmt* forOp=dyn_cast<ForStmt>(child);
        std::vector<Edge> temp=external_ForStmt(forOp,multiple2,prevNote);
        for (const auto& edge:temp) {
          S1.push_back({edge.src,edge.dest,edge.weight});
        }
      }
      prevNote=child;
    }
    return S1;
  }

  std::vector<Edge> external_BinaryOperator(BinaryOperator *binaryOp,int multiple) {
    note_and_edges LEFT,RIGHT;
    cout<<"进入左节点"<<endl;
    LEFT = find_DeclRefExpr(binaryOp->getLHS());
    std::cout<<"打印LEFT的内容如下"<<std::endl;
    for (const auto& i : LEFT.note) {
      std::cout<<i<<std::endl;
    }

    cout<<"进入右节点"<<endl;
    RIGHT = find_DeclRefExpr(binaryOp->getRHS());

    //把函数名替换成return值，例如把fun替换成result
    for (auto& item1:RIGHT.note) {
      for (const auto& item2:results) {
        if (item1==item2["function_name"]) {
          item1=item2["return_name"];
        }
      }
    }
    std::cout<<"打印RIGHT.note的内容如下"<<std::endl;
    for (const auto& i : RIGHT.note) {
      std::cout<<i<<std::endl;
    }

    cout<<"调用GenerativeDependency"<<endl;
    std::vector<Edge> edges = GenerativeDependency(LEFT.note, RIGHT.note, multiple);


    std::cout<<"打印RIGHT.edges的内容如下"<<std::endl;
    for (const auto& item : RIGHT.edges) {
      cout<<"src:"<<item.src<<"dest:"<<item.dest<<"weight:"<<item.weight<<endl;
    }

    //把RIGHT.edges的权重扩大multiple倍数
    for (auto& item: RIGHT.edges) {
      item.weight=item.weight*multiple;
    }
    cout<<"！！！！！！！！！！！！！！！！！！！RIGHT.edges的权重扩大multiple倍数后的结果!!!!!!!!!!!!!!!!!"<<endl;
    for (auto& item: RIGHT.edges) {
      cout<<"src:"<<item.src<<"dest:"<<item.dest<<"weight:"<<item.weight<<endl;
    }
    //把edges信息加上RIGHT.edges信息
    for (const auto& item : RIGHT.edges) {
      edges.push_back(item);
    }
    cout<<"！！！！！！！！！！！！！！！！！！！把edges信息加上RIGHT.edges信息的结果!!!!!!!!!!!!!!!!!"<<endl;
    for (const auto &item: edges) {
      cout<<"src:"<<item.src<<"dest:"<<item.dest<<"weight:"<<item.weight<<endl;
    }

    return edges;
  }
  
  // 遍历当前节点，找到当前节点的所有double类型的DeclRefExpr节点,加入到NOTE，以及实参和形参得到的边、函数内部整体的边+1、sin:a:b分解得到的边
  note_and_edges find_DeclRefExpr(Stmt *stmt) {
    //合并重复函数前的NOTE
    std::vector<std::string> NOTE;
    //合并重复函数后的NOTE2
    std::vector<std::string> NOTE2;

    //实参和形参得到的边,假设已经获得
    std::vector<Edge> edges;

    if (isa<DeclRefExpr>(stmt)) {
      DeclRefExpr *declRefExpr = cast<DeclRefExpr>(stmt);
      // 对DeclRefExpr节点进行处理（例如打印引用的变量名）
      clang::QualType qualType = declRefExpr->getType();
      cout<<"遇到"<<declRefExpr->getNameInfo().getAsString()<<"节点"<<endl;
      if (qualType->isSpecificBuiltinType(clang::BuiltinType::Double)) {
        NOTE.push_back(declRefExpr->getNameInfo().getAsString());
        llvm::outs() << "找到double的变量: " << declRefExpr->getNameInfo().getAsString() << "\n";
        cout<<"节点"<<declRefExpr->getNameInfo().getAsString()<<"被push进NOTE"<<endl;
      }
    }
    else if (isa<CallExpr>(stmt)) {
      CallExpr* callexpr=dyn_cast<CallExpr>(stmt);
      FunInformation callname_and_returnname_and_shican=handle_CallExpr(callexpr);
      cout<<"正在处理"<<callname_and_returnname_and_shican.callname<<"函数"<<endl;

      // 把实参与fun链接一次
      if (callname_and_returnname_and_shican.shican.size()==callname_and_returnname_and_shican.xingcan.size()) {
        cout<<"形参和实参数量一样"<<endl;
      } else cout<< "形参和实参数量不一样"<<endl;

      cout<<"看看callname_and_returnname_and_shican.shican"<<endl;
      for (const auto &item: callname_and_returnname_and_shican.shican) {
        cout<<"shican:"<<item<<endl;
      }
      cout<<"看看callname_and_returnname_and_shican.xingcan"<<endl;
      for (const auto &item: callname_and_returnname_and_shican.xingcan) {
        cout<<"xingcan:"<<item<<endl;
      }

      int l1=1;
      int l2=1;
      for (auto& item1 : callname_and_returnname_and_shican.shican) {
        for (auto& item2 : callname_and_returnname_and_shican.xingcan) {
          if (l1==l2) {
            edges.push_back({item1,item2,1});
          }
          l2++;
        }
        l1++;
        l2=1;
      }
      // 把函数内部整体的边+1,如果遇到fun函数，把函数内部的边+1，如果遇到sin库函数，就跳过
      if (library_functions.find(callname_and_returnname_and_shican.callname) == library_functions.end()) {
        for (const auto& item : once_fun_edges[callname_and_returnname_and_shican.callname]) {
          edges.push_back(item);
          cout<<"src:"<<item.src<<"dest:"<<item.dest<<"weight:"<<item.weight<<"被push_back进去了"<<endl;
        }
        cout<<"匹配到"<<callname_and_returnname_and_shican.callname<<"函数，fun+1"<<endl;
      }
      

      // 如果是fun函数，则正常返回fun;如果是sin等库函数，也返回sin，但是把sin的边给分解一下，加入到edges
      if (library_functions.find(callname_and_returnname_and_shican.callname) != library_functions.end()) {
        //匹配到库函数，处理库函数
        // for (const auto& item:callname_and_returnname_and_shican.shican) {
        //   callname_and_returnname_and_shican.callname+=":";
        //   callname_and_returnname_and_shican.callname+=item;
        // }
        std::vector<std::string> temp;
        temp.push_back(callname_and_returnname_and_shican.callname);
        for (const auto& item:callname_and_returnname_and_shican.shican) {
          temp.push_back(item);
        }
        //如果是单独的sin库函数，就跳过,只处理类似于sin、a、b
        if (temp.size()>=1) {
          int i=1;
          int j=1;
          for (const auto& item1 : temp) {
            for (const auto& item2 : temp) {
              if (j>i) {
                edges.push_back({item1,item2,1});
              }
              j++;
            }
            i++;
            j=1;
          }
        }
      }

      NOTE.push_back(callname_and_returnname_and_shican.callname);
    }



    // 递归访问当前语句的所有子语句
    else {
      for (Stmt::child_iterator ci = stmt->child_begin(), ce = stmt->child_end(); ci != ce; ++ci) {
        cout<<"进入一层递归"<<endl;
        note_and_edges childNotesandEdges = find_DeclRefExpr(*ci);
        cout<<"离开一层递归"<<endl;
        NOTE.insert(NOTE.end(), childNotesandEdges.note.begin(), childNotesandEdges.note.end());
        edges.insert(edges.end(), childNotesandEdges.edges.begin(), childNotesandEdges.edges.end());
      }
    }

    //打印看看NOTE是什么
    cout<<"=====开始NOTE===="<<endl;
    for (const auto& note : NOTE) {
      cout<<"节点："<<note<<endl;
    }
    cout<<"=====结束NOTE===="<<endl;

    // 初始化results,全部恢复成false
    for (auto& item:results) {
      item["if_delete"]=false;
    }

    //合并NOTE节点种重复的函数名(s1=fun(a)+fun(b),把RIGHT=fun,fun合并为RIGHT=fun),并把return加入到NOTE2
    for (const auto& item1:NOTE) {
      bool flag=true;
      for (auto& item2:results) {
        if (item1==item2["function_name"]&&item2["if_delete"]==false) {
          item2["if_delete"]=true;
          flag=true;
        }
        else if (item1==item2["function_name"]&&item2["if_delete"]==true) {
          flag=false;
        }
      }
      if (flag) {
        NOTE2.push_back(item1);
      }
    }

    //打印看看NOTE2是什么
    cout<<"=====修改前的NOTE2===="<<endl;
    for (const auto& note : NOTE2) {
      cout<<"节点："<<note<<endl;
    }
    cout<<"=====结束NOTE2===="<<endl;

    return {NOTE2,edges};
  }



  //把CallExpr函数相关的所有信息收集起来
  FunInformation handle_CallExpr(CallExpr* callexpr) {
    FunInformation callname_and_returnname_and_shican;
    int index=1;
    for (CallExpr::child_iterator ci=callexpr->child_begin(),ce=callexpr->child_end();ci!=ce;ci++) {
      if (index==1) {
        if (isa<DeclRefExpr>(*ci)) {
          DeclRefExpr* s=dyn_cast<DeclRefExpr>(*ci);
          callname_and_returnname_and_shican.callname=s->getNameInfo().getAsString();
        }
        else {
          DeclRefExpr* s=dyn_cast<DeclRefExpr>(*ci->child_begin());
          callname_and_returnname_and_shican.callname=s->getNameInfo().getAsString();
        }
      }
      else {
        note_and_edges ParameterNameVector=find_DeclRefExpr(*ci);
        for (const auto& ParameterName : ParameterNameVector.note) {
          callname_and_returnname_and_shican.shican.push_back({ParameterName});
        }
      }
      index++;
    }

    //找到形参
    for (const auto& item : results) {
      if (callname_and_returnname_and_shican.callname==item["function_name"]) {
        for (auto& s:item["xingcan"]) {
          callname_and_returnname_and_shican.xingcan.push_back(s);
        }
        cout<<"找到返回值了"<<endl;
      }
    }

    //找到返回值
    for (const auto& item : results) {
      if (callname_and_returnname_and_shican.callname==item["function_name"]) {
        callname_and_returnname_and_shican.returnname=item["return_name"];
        cout<<"找到返回值了"<<endl;
      }
    }

    return callname_and_returnname_and_shican;
  }

  std::vector<Edge> GenerativeDependency(std::vector<std::string>& LEFT,std::vector<std::string>& RIGHT,int multiple) {
    std::vector<Edge> edges={
    };

    int index1=1;
    for (const auto& note1:RIGHT) {
      int index2=1;
      for (const auto& note2:RIGHT) {
        if (index2 > index1) {
            // std::cout << "进入第一个" << std::endl;
            edges.push_back({note1, note2, multiple});
        }
        index2++;
      }
      index1++;
    }

    for (const auto& note1:LEFT) {
      for (const auto& note2:RIGHT) {
        // cout<<"进入第二个"<<endl;
        edges.push_back({note1,note2,multiple});
      }
    }


    return edges;
  }
};




// 遍历所有的变量
class MyVariableASTVisitor1 : public RecursiveASTVisitor<MyVariableASTVisitor1> {
public:
  //funEdges映射来记录所有函数和其边信息
  std::unordered_map<std::string,std::vector<Edge>> funEdges;

  MyVariableASTVisitor1(Rewriter &R, MyVariableASTVisitor2 &visitor2) : TheRewriter(R), Visitor2(visitor2) {}
  bool VisitFunctionDecl(FunctionDecl *f) {
    SourceManager &SM = f->getASTContext().getSourceManager();
    if (!SM.isInMainFile(f->getLocation()))
    {
        // 如果不在主文件中，跳过该变量节点
        return true;
    }


    if (f->getNameAsString()!="main") {
      llvm::outs() << "FunctionDecl: " << f->getNameAsString() << "\n";
      // 进入函数体(CompoundStmt)
      if (CompoundStmt *CS = dyn_cast<CompoundStmt>(
        f->getBody())) {
        std::vector<Edge> temp=Visitor2.HandleFirstLevelChildren(CS);
        //把sin展开？？？？？？？？？？？？？？？？？？？
        //这里写展开边的函数udfold，main和这个都需要调用

        once_fun_edges[f->getNameAsString()]=temp;
        
      }
      cout<<"打印一下once_fun_edges的内容"<<endl;
      for (const auto& edge:once_fun_edges[f->getNameAsString()]) {
        cout<<"src:"<<edge.src<<",dest:"<<edge.dest<<",weight:"<<edge.weight<<endl;
      }
      cout<<"打印完毕"<<endl;


    }

    cout<<"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~第一次遍历完成！~~~~~~~~~~~~~~~~~~~~~~~~~"<<endl;
    return true;
  }
private:
  Rewriter &TheRewriter;
  MyVariableASTVisitor2 &Visitor2;

};


// ASTConsumer接口的实现，用于读取由Clang分析器产生的AST
class MyASTConsumer : public ASTConsumer
{
public:
  MyASTConsumer(Rewriter &R) : Visitor1(R, Visitor2),Visitor2(R)  {}
  // 覆盖每个被解析的顶层声明被调用的方法
  bool HandleTopLevelDecl(DeclGroupRef DR) override
  {
    for (DeclGroupRef::iterator b = DR.begin(), e = DR.end(); b != e; ++b)
    {
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
  MyVariableASTVisitor1 Visitor1;
  MyVariableASTVisitor2 Visitor2;
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
  // 打开 JSON 文件
  file.open("/home/zjn/mytool/temp.json");
  if (!file.is_open()) {
      std::cerr << "Failed to open file." << std::endl;
      return 1;
  }

    // 读取文件内容到 JSON 对象中
  json j;
  file >> j;
  // 确保文件关闭
  file.close();

  // 将 JSON 对象转换为 vector<json>
  results = j.get<std::vector<json>>();


  llvm::cl::OptionCategory ToolingCategory("my-tool options");
  auto ExpectedParser = clang::tooling::CommonOptionsParser::create(argc, argv, ToolingCategory);
  CommonOptionsParser &OptionsParser = ExpectedParser.get();
  ClangTool Tool(OptionsParser.getCompilations(), OptionsParser.getSourcePathList());
  return Tool.run(newFrontendActionFactory<MyFrontendAction>().get());
}





