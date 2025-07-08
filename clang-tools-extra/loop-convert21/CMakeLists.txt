set(LLVM_LINK_COMPONENTS support)

add_clang_executable(loop-convert21
  LoopConvert21.cpp
  )
target_link_libraries(loop-convert21
  PRIVATE
  clangAST
  clangASTMatchers
  clangBasic
  clangFrontend
  clangSerialization
  clangTooling
  )