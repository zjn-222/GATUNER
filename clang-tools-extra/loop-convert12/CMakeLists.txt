set(LLVM_LINK_COMPONENTS support FrontendOpenMP)

add_clang_executable(loop-convert12
  LoopConvert12.cpp
  )
target_link_libraries(loop-convert12
  PRIVATE
  clangAST
  clangASTMatchers
  clangBasic
  clangFrontend
  clangSerialization
  clangTooling
  )