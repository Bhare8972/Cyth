cmake_minimum_required(VERSION 3.3)

project("cyth")

##make LLVM???? (does this build Clang?)
add_subdirectory(llvm37)

#set(LLVM_LINK_COMPONENTS
 # Core
  #ExecutionEngine
 # MC
 # MCJIT
  #Support
 # native
 # )

##include utilities
set(utils ./utilities)
include_directories ("${utils}")

##include LLVM headers
set(llvm_include ./llvm37/include)
include_directories ("${llvm_include}")

##include Clang headers
set(clang_include ./llvm37/tools/clang/include)
include_directories ("${clang_include}")

##C++ settings
SET(CMAKE_CXX_FLAGS "-std=c++0x -Wall") #this is wrong, and needs to be fixed
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}") #I don't know what this line, or the next, does
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
add_definitions(-D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS)

##now, somehow, we need to link-in LLVM and Clang


add_executable(cyth
              ./clang_interp.cpp
              
              ${utils}/lexer.cpp
              ${utils}/logger.cpp
              ${utils}/regex.cpp
              ${utils}/UTF8.cpp
              ${utils}/parser.cpp)
              

add_dependencies(cyth
  clang-headers
  )

##clang libraries
target_link_libraries(cyth
  clangBasic
  clangCodeGen
  clangDriver
  clangFrontend
  )
  
##LLVMlibraries
#target_link_libraries(cyth
#  LLVMCore
#  LLVMSupport
#  LLVMExecutionEngine
#  LLVMMC
#  LLVMMCJIT
#  ) 
  
  
  
  
  #-I/usr/local/include  -fPIC -fvisibility-inlines-hidden -Wall -W -Wno-unused-parameter 
  #-Wwrite-strings -Wcast-qual -Wno-missing-field-initializers -pedantic -Wno-long-long 
  #-Wno-maybe-uninitialized -Wno-comment -std=c++11 -ffunction-sections -fdata-sections   
  #-D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS

  
  
  #system libs
  #-lrt -ldl -ltinfo -lpthread -lz -lm

#LLVM libraries
target_link_libraries(cyth
  LLVMLTO
  LLVMObjCARCOpts
  LLVMLinker
  LLVMBitWriter
  LLVMIRReader 
  LLVMXCoreDisassembler
  LLVMXCoreCodeGen
  LLVMXCoreDesc
  LLVMXCoreInfo
  LLVMXCoreAsmPrinter
  LLVMSystemZDisassembler
  LLVMSystemZCodeGen
  LLVMSystemZAsmParser
  LLVMSystemZDesc
  LLVMSystemZInfo
  LLVMSystemZAsmPrinter
  LLVMSparcDisassembler
  LLVMSparcCodeGen
  LLVMSparcAsmParser
  LLVMSparcDesc
  LLVMSparcInfo 
  LLVMSparcAsmPrinter
  LLVMR600CodeGen
  LLVMR600AsmParser
  LLVMR600Desc
  LLVMR600Info
  LLVMR600AsmPrinter
  LLVMPowerPCDisassembler
  LLVMPowerPCCodeGen 
  LLVMPowerPCAsmParser
  LLVMPowerPCDesc
  LLVMPowerPCInfo
  LLVMPowerPCAsmPrinter 
  LLVMNVPTXCodeGen
  LLVMNVPTXDesc
  LLVMNVPTXInfo
  LLVMNVPTXAsmPrinter 
  LLVMMSP430CodeGen
  LLVMMSP430Desc
  LLVMMSP430Info
  LLVMMSP430AsmPrinter
  LLVMMipsDisassembler
  LLVMMipsCodeGen
  LLVMMipsAsmParser
  LLVMMipsDesc
  LLVMMipsInfo
  LLVMMipsAsmPrinter
  LLVMHexagonDisassembler
  LLVMHexagonCodeGen 
  LLVMHexagonDesc
  LLVMHexagonInfo
  LLVMCppBackendCodeGen
  LLVMCppBackendInfo
  LLVMARMDisassembler
  LLVMARMCodeGen
  LLVMARMAsmParser
  LLVMARMDesc
  LLVMARMInfo
  LLVMARMAsmPrinter
  LLVMAArch64Disassembler
  LLVMAArch64CodeGen
  LLVMAArch64AsmParser
  LLVMAArch64Desc
  LLVMAArch64Info
  LLVMAArch64AsmPrinter
  LLVMAArch64Utils
  LLVMMIRParser
  LLVMAsmParser
  LLVMDebugInfoPDB
  LLVMTableGen
  LLVMOrcJIT
  LLVMInstrumentation
  LLVMOption
  LLVMX86Disassembler
  LLVMX86AsmParser
  LLVMX86CodeGen
  LLVMSelectionDAG
  LLVMAsmPrinter
  LLVMX86Desc
  LLVMMCDisassembler
  LLVMX86Info
  LLVMX86AsmPrinter
  LLVMX86Utils
  LLVMMCJIT
  LLVMDebugInfoDWARF
  LLVMLineEditor
  LLVMPasses
  LLVMipo
  LLVMVectorize
  LLVMInterpreter
  LLVMExecutionEngine
  LLVMRuntimeDyld
  LLVMCodeGen
  LLVMTarget
  LLVMScalarOpts
  LLVMProfileData
  LLVMObject
  LLVMMCParser
  LLVMBitReader
  LLVMInstCombine
  LLVMTransformUtils
  LLVMipa
  #LLVMM
  LLVMAnalysis
  LLVMCore
  LLVMSupport)

  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
