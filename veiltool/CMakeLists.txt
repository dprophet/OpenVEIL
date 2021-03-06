#	Copyright (c) 2017, TecSec, Inc.
#
#	Redistribution and use in source and binary forms, with or without
#	modification, are permitted provided that the following conditions are met:
#	
#		* Redistributions of source code must retain the above copyright
#		  notice, this list of conditions and the following disclaimer.
#		* Redistributions in binary form must reproduce the above copyright
#		  notice, this list of conditions and the following disclaimer in the
#		  documentation and/or other materials provided with the distribution.
#		* Neither the name of TecSec nor the names of the contributors may be
#		  used to endorse or promote products derived from this software 
#		  without specific prior written permission.
#		 
#	ALTERNATIVELY, provided that this notice is retained in full, this product
#	may be distributed under the terms of the GNU General Public License (GPL),
#	in which case the provisions of the GPL apply INSTEAD OF those given above.
#		 
#	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
#	ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
#	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
#	DISCLAIMED.  IN NO EVENT SHALL TECSEC BE LIABLE FOR ANY 
#	DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
#	(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
#	LOSS OF USE, DATA OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
#	ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
#	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
#	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# Written by Roger Butler

INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    )

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/veiltool.rc.in ${CMAKE_CURRENT_BINARY_DIR}/veiltool.rc @ONLY)

set(CppSources
    veiltool.cpp
    stdafx.cpp
	VeilToolUtils.cpp
	
	genecc.cpp
	genrsa.cpp
	gendh.cpp
	pemtool.cpp
	selftest.cpp
	settingsTool.cpp
	foldersTool.cpp
	genx25519.cpp
	genNums.cpp
	)

add_executable(veiltool 
	targetver.h 
	stdafx.h 
	
	${CppSources} 
	${CMAKE_CURRENT_SOURCE_DIR}/veiltool.rc.in 
	${CMAKE_CURRENT_BINARY_DIR}/veiltool.rc 
	
	${soFilesToCopy}
	)
add_precompiled_header(veiltool stdafx.h FORCEINCLUDE SOURCE_CXX stdafx.cpp)

set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/veiltool.rc PROPERTIES COMPILE_DEFINITIONS RC_COMPILED)
Source_Group(Commands FILES
	genecc.cpp
	genrsa.cpp
	gendh.cpp
	pemtool.cpp
	selftest.cpp
	settingsTool.cpp
	foldersTool.cpp
	genx25519.cpp
	genNums.cpp
	)
	
set_target_properties(veiltool PROPERTIES FOLDER "Tools")
TARGET_LINK_LIBRARIES(veiltool VEILCore)
set_target_properties(veiltool PROPERTIES OUTPUT_NAME veil)
if(UNIX)
    set_target_properties(veiltool PROPERTIES OUTPUT_NAME_DEBUG veil.debug)
endif(UNIX)

add_subdirectory(extensions)

install(TARGETS veiltool RUNTIME DESTINATION ${BIN_DIR})
