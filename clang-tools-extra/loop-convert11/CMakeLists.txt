set(LLVM_LINK_COMPONENTS support FrontendOpenMP)

add_clang_executable(loop-convert11
  LoopConvert11.cpp
  )
target_link_libraries(loop-convert11
  PRIVATE
  clangAST
  clangASTMatchers
  clangBasic
  clangFrontend
  clangSerialization
  clangTooling
  )