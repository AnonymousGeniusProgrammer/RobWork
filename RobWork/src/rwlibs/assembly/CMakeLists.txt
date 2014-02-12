SET(SUBSYS_NAME rw_assembly)
set(SUBSYS_DESC "Assembly task and result formats and strategies.")
SET(SUBSYS_DEPS rw rw_task)

if( RW_HAVE_XERCES )
    set(DEFAULT TRUE)
    set(REASON)
else()
    set(DEFAULT FALSE)
    set(REASON "Xerces not found.")
endif()


SET(build TRUE)
RW_SUBSYS_OPTION(build ${SUBSYS_NAME} ${SUBSYS_DESC} ${DEFAULT} ${REASON})
RW_SUBSYS_DEPEND(build ${SUBSYS_NAME} DEPS ${SUBSYS_DEPS} EXT_DEPS xerces)
RW_ADD_DOC(${SUBSYS_NAME})

IF(build)
    SET(SRC_FILES
      ./AssemblyControlResponse.cpp
      ./AssemblyControlStrategy.cpp
      ./AssemblyParameterization.cpp
      ./AssemblyRegistry.cpp
      ./AssemblyResult.cpp
      ./AssemblyState.cpp
      ./AssemblyTask.cpp
      ./CircularPiHControlStrategy.cpp
      ./CircularPiHParameterization.cpp
    )
    
    SET(SRC_FILES_HPP
      ./AssemblyControlResponse.hpp
      ./AssemblyControlStrategy.hpp
      ./AssemblyParameterization.hpp
      ./AssemblyRegistry.hpp
      ./AssemblyResult.hpp
      ./AssemblyState.hpp
      ./AssemblyTask.hpp
      ./CircularPiHControlStrategy.hpp
      ./CircularPiHParameterization.hpp
    )
      	  
    RW_ADD_LIBRARY(${SUBSYS_NAME} assembly ${SRC_FILES} ${SRC_FILES_HPP} ${LOADER_SRC_FILES_HPP})
    RW_ADD_INCLUDES(assembly "rwlibs/assembly" ${SRC_FILES_HPP})
    TARGET_LINK_LIBRARIES(${SUBSYS_NAME} ${SUBSYS_DEPS})
    ADD_DEPENDENCIES(${SUBSYS_NAME} ${SUBSYS_DEPS})
ENDIF()