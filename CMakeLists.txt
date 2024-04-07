# SPDX-License-Identifier: BSD-2-Clause

# Copyright (c) 2023 Maarten L. Hekkelman

# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:

# 1. Redistributions of source code must retain the above copyright notice, this
# list of conditions and the following disclaimer
# 2. Redistributions in binary form must reproduce the above copyright notice,
# this list of conditions and the following disclaimer in the documentation
# and/or other materials provided with the distribution.

# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

cmake_minimum_required(VERSION 3.28)

# set the project name
project(libmgui VERSION 1.0.0 LANGUAGES CXX)

list(PREPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(GNUInstallDirs)
include(CheckFunctionExists)
include(CheckIncludeFiles)
include(CheckLibraryExists)
include(CMakePackageConfigHelpers)
include(GenerateExportHeader)
include(FindPkgConfig)
include(VersionString)

if(WIN32)
	if(${CMAKE_SYSTEM_VERSION} GREATER_EQUAL 10) # Windows 10
		add_definitions(-D _WIN32_WINNT=0x0A00)
	elseif(${CMAKE_SYSTEM_VERSION} EQUAL 6.3) # Windows 8.1
		add_definitions(-D _WIN32_WINNT=0x0603)
	elseif(${CMAKE_SYSTEM_VERSION} EQUAL 6.2) # Windows 8
		add_definitions(-D _WIN32_WINNT=0x0602)
	elseif(${CMAKE_SYSTEM_VERSION} EQUAL 6.1) # Windows 7
		add_definitions(-D _WIN32_WINNT=0x0601)
	elseif(${CMAKE_SYSTEM_VERSION} EQUAL 6.0) # Windows Vista
		add_definitions(-D _WIN32_WINNT=0x0600)
	else() # Windows XP (5.1)
		add_definitions(-D _WIN32_WINNT=0x0501)
	endif()

	# Man, this is 2024 we're living in...
	add_definitions(-DNOMINMAX)

	# We do not want to write an export file for all our symbols...
	set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

if(MSVC)
	# make msvc standards compliant...
	add_compile_options(/permissive- /bigobj)
	add_link_options(/NODEFAULTLIB:library)

	# This is dubious...
	if(BUILD_SHARED_LIBS)
		set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
	else()
		set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
	endif()
endif()

find_package(Mrc REQUIRED)
message("Using resources compiled with ${MRC_EXECUTABLE}")
mrc_write_header(${CMAKE_CURRENT_SOURCE_DIR}/include/mrsrc.hpp)

set(CMAKE_THREAD_PREFER_PTHREAD)
set(THREADS_PREFER_PTHREAD_FLAG)
find_package(Threads REQUIRED)

if(NOT TARGET mxml AND NOT mxml_FOUND)
	find_package(mxml 1.0.2 REQUIRED)
endif()

# Version info
write_version_header("${CMAKE_CURRENT_SOURCE_DIR}/src" LIB_NAME "libmgui")

# The salt executable
list(APPEND headers
	include/MAlerts.hpp
	include/MAnimation.hpp
	include/MApplication.hpp
	include/MCanvas.hpp
	include/MClipboard.hpp
	include/MColor.hpp
	include/MColorPicker.hpp
	include/MCommand.hpp
	include/MControls.hpp
	include/MControlsImpl.hpp
	include/MControls.inl
	include/MDevice.hpp
	include/MDeviceImpl.hpp
	include/MDialog.hpp
	include/MError.hpp
	include/MFile.hpp
	include/MLib.hpp
	include/MMenu.hpp
	include/MP2PEvents.hpp
	include/MPreferences.hpp
	include/mrsrc.hpp
	include/MSound.hpp
	include/MStrings.hpp
	include/MTypes.hpp
	include/MUnicode.hpp
	include/MUnicode.inl
	include/MUnicodeTables.hpp
	include/MUtils.hpp
	include/MView.hpp
	include/MWindow.hpp)

list(APPEND private_headers
	${CMAKE_CURRENT_SOURCE_DIR}/src/Gtk/MGtkCanvasImpl.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Gtk/MGtkControlsImpl.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Gtk/MGtkDeviceImpl.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Gtk/MGtkLib.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Gtk/MGtkWidgetMixin.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Gtk/MGtkWindowImpl.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Gtk/MGtkApplicationImpl.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/revision.hpp
)

list(APPEND sources
	${CMAKE_CURRENT_SOURCE_DIR}/src/Gtk/MGtkAlerts.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Gtk/MGtkClipboardImpl.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Gtk/MGtkError.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Gtk/MGtkFile.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Gtk/MGtkMenu.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Gtk/MGtkSound.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Gtk/MGtkUtils.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Gtk/MGtkCanvasImpl.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Gtk/MGtkControlsImpl.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Gtk/MGtkDeviceImpl.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Gtk/MGtkDialogImpl.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Gtk/MGtkWidgetMixin.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Gtk/MGtkApplicationImpl.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Gtk/MGtkWindowImpl.cpp

	${CMAKE_CURRENT_SOURCE_DIR}/src/MAlerts.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/MAnimation.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/MApplication.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/MCanvas.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/MClipboard.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/MColor.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/MColorPicker.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/MControls.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/MDevice.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/MDialog.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/MLib.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/MMenu.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/MPreferences.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/MStrings.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/MTypes.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/MUnicode.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/MUtils.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/MView.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/MWindow.cpp
)

add_library(mgui STATIC)
add_library(mgui::mgui ALIAS mgui)
target_compile_features(mgui PUBLIC cxx_std_20)

target_sources(mgui
	PRIVATE ${sources}
	PRIVATE FILE_SET private_headers TYPE HEADERS
	PUBLIC FILE_SET public_headers TYPE HEADERS
	BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
	FILES ${headers}
)

target_link_libraries(mgui Threads::Threads mxml::mxml)

install(TARGETS mgui
	EXPORT mgui
	FILE_SET public_headers DESTINATION include)

install(EXPORT mgui
	NAMESPACE mgui::
	DESTINATION lib/cmake/mgui
	FILE mgui-targets.cmake)

configure_package_config_file(
	${PROJECT_SOURCE_DIR}/cmake/mgui-config.cmake.in
	${CMAKE_CURRENT_BINARY_DIR}/mgui-config.cmake
	INSTALL_DESTINATION lib/cmake/mgui)

write_basic_package_version_file(
	"${CMAKE_CURRENT_BINARY_DIR}/mgui-config-version.cmake"
	COMPATIBILITY AnyNewerVersion
)

install(FILES
	${CMAKE_CURRENT_BINARY_DIR}/mgui-config.cmake
	${CMAKE_CURRENT_BINARY_DIR}/mgui-config-version.cmake
	DESTINATION lib/cmake/mgui)

if(UNIX)
	pkg_check_modules(GTK4 gtk4 REQUIRED IMPORTED_TARGET)
	target_link_libraries(mgui PkgConfig::GTK4)
endif()

if(PROJECT_IS_TOP_LEVEL)
	add_subdirectory(examples)
endif()
