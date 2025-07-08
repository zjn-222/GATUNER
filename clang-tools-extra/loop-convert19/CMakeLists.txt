set(LLVM_LINK_COMPONENTS support)

add_clang_executable(loop-convert19
  LoopConvert19.cpp
  )
target_link_libraries(loop-convert19
  PRIVATE
  clangAST
  clangASTMatchers
  clangBasic
  clangFrontend
  clangSerialization
  clangTooling
  )