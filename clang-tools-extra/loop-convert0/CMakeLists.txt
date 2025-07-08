set(LLVM_LINK_COMPONENTS support)

add_clang_executable(loop-convert0
  LoopConvert0.cpp
  )
target_link_libraries(loop-convert0
  PRIVATE
  clangAST
  clangASTMatchers
  clangBasic
  clangFrontend
  clangSerialization
  clangTooling
  )




