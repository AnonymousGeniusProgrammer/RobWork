# -*- cmake -*-

set(RW_INCLUDE_DIRECTORIES
  ${RW_ROOT}/ext
  ${RW_ROOT}/ext/boostbindings
  ${RW_ROOT}/src
  ${RW_ROOT}/ext/lua/src
  ${RW_ROOT}/ext/tolua/include)

include_directories(${RW_INCLUDE_DIRECTORIES})
