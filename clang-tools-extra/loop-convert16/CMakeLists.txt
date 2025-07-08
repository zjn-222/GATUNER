set(LLVM_LINK_COMPONENTS support)

add_clang_executable(loop-convert16
  LoopConvert16.cpp
  )
target_link_libraries(loop-convert16
  PRIVATE
  clangAST
  clangASTMatchers
  clangBasic
  clangFrontend
  clangSerialization
  clangTooling
  )