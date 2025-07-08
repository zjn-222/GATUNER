set(LLVM_LINK_COMPONENTS support)

add_clang_executable(loop-convert18
  LoopConvert18.cpp
  )
target_link_libraries(loop-convert18
  PRIVATE
  clangAST
  clangASTMatchers
  clangBasic
  clangFrontend
  clangSerialization
  clangTooling
  )