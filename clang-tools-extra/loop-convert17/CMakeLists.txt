set(LLVM_LINK_COMPONENTS support)

add_clang_executable(loop-convert17
  LoopConvert17.cpp
  )
target_link_libraries(loop-convert17
  PRIVATE
  clangAST
  clangASTMatchers
  clangBasic
  clangFrontend
  clangSerialization
  clangTooling
  )