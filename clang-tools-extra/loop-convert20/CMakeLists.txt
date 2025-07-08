set(LLVM_LINK_COMPONENTS support)

add_clang_executable(loop-convert20
  LoopConvert20.cpp
  )
target_link_libraries(loop-convert20
  PRIVATE
  clangAST
  clangASTMatchers
  clangBasic
  clangFrontend
  clangSerialization
  clangTooling
  )