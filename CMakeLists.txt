cmake_minimum_required(VERSION 2.8)
project(Engoo)

if( COMMAND cmake_policy )
	if( POLICY CMP0054 )
		cmake_policy( SET CMP0054 NEW )
	endif( POLICY CMP0054 )
endif( COMMAND cmake_policy )

if( MSVC )
	# Statically link the run time and disable run time checks in debug mode.
	string(REPLACE "/MD " " " CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE} )
	string(REPLACE "/MD " " " CMAKE_CXX_FLAGS_MINSIZEREL ${CMAKE_CXX_FLAGS_MINSIZEREL} )
	string(REPLACE "/MD " " " CMAKE_CXX_FLAGS_RELWITHDEBINFO ${CMAKE_CXX_FLAGS_RELWITHDEBINFO} )
	string(REPLACE "/MDd " " " CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG} )
	string(REPLACE "/MD " " " CMAKE_C_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE} )
	string(REPLACE "/MD " " " CMAKE_C_FLAGS_MINSIZEREL ${CMAKE_C_FLAGS_MINSIZEREL} )
	string(REPLACE "/MD " " " CMAKE_C_FLAGS_RELWITHDEBINFO ${CMAKE_C_FLAGS_RELWITHDEBINFO} )
	string(REPLACE "/MDd " " " CMAKE_C_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG} )

	string(REPLACE "/RTC1" "" CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG} )
	string(REPLACE "/RTC1" "" CMAKE_C_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG} )

	set( CMAKE_C_FLAGS_RELEASE "/MT ${CMAKE_C_FLAGS_RELEASE}" )
	set( CMAKE_C_FLAGS_MINSIZEREL "/MT ${CMAKE_C_FLAGS_MINSIZEREL}" )
	set( CMAKE_C_FLAGS_RELWITHDEBINFO "/MT ${CMAKE_C_FLAGS_RELWITHDEBINFO}" )
	set( CMAKE_C_FLAGS_DEBUG "/MTd ${CMAKE_C_FLAGS_DEBUG}" )

	set( CMAKE_CXX_FLAGS_RELEASE "/MT ${CMAKE_CXX_FLAGS_RELEASE}" )
	set( CMAKE_CXX_FLAGS_MINSIZEREL "/MT ${CMAKE_CXX_FLAGS_MINSIZEREL}" )
	set( CMAKE_CXX_FLAGS_RELWITHDEBINFO "/MT ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}" )
	set( CMAKE_CXX_FLAGS_DEBUG "/MTd ${CMAKE_CXX_FLAGS_DEBUG}" )

	add_definitions("/D_CRT_SECURE_NO_WARNINGS")
endif( MSVC )

# On for now until the ASM is made to work on newer compilers
option( NO_ASM "Disable assembly code" OFF )

if( NOT NO_ASM )
	if( MSVC )
		add_subdirectory( gas2masm )
	endif( MSVC )

	set( ASM_SOURCE_EXTENSION .s )
	if( UNIX )
		set( ASM_OUTPUT_EXTENSION .o )
	else( UNIX )
		set( ASM_OUTPUT_EXTENSION .obj )
	endif( UNIX )
	MACRO( ADD_ASM_FILE indir infile )
		set( ASM_OUTPUT_${infile} "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/Engoo.dir/${indir}/${infile}${ASM_OUTPUT_EXTENSION}" )
		set( ASM_TEMP_${infile} "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/Engoo.dir/${indir}/${infile}.spp" )
		set( ASM_TEMP2_${infile} "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/Engoo.dir/${indir}/${infile}.asm" )
		if( MSVC )
			add_custom_command( OUTPUT ${ASM_OUTPUT_${infile}}
				COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/Engoo.dir/${indir}
				COMMAND cl /EP "${CMAKE_CURRENT_SOURCE_DIR}/${indir}/${infile}${ASM_SOURCE_EXTENSION}" > "${ASM_TEMP_${infile}}"
				COMMAND gas2masm <  "${ASM_TEMP_${infile}}" >  "${ASM_TEMP2_${infile}}"
				COMMAND ml /c /Cp /coff "/Fo${ASM_OUTPUT_${infile}}" /Zm /Zi  "${ASM_TEMP2_${infile}}"
				MAIN_DEPENDENCY gas2masm )
		else( MSVC )
			add_custom_command( OUTPUT ${ASM_OUTPUT_${infile}}
				COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/Engoo.dir/${indir}
				COMMAND mingw32-gcc -x assembler-with-cpp -o "${ASM_OUTPUT_${infile}}" -c "${CMAKE_CURRENT_SOURCE_DIR}/${indir}/${infile}${ASM_SOURCE_EXTENSION}" )
		endif( MSVC )
		set( ASM_SOURCES ${ASM_SOURCES} "${ASM_OUTPUT_${infile}}" )
	ENDMACRO( ADD_ASM_FILE )
endif( NOT NO_ASM )

if( WIN32 )
	if( "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" )
		# CMake is not set up to compile and link rc files with GCC. :(
		add_custom_command( OUTPUT WinQuake_vc.o
			COMMAND windres -o WinQuake_vc.o -i ${CMAKE_CURRENT_SOURCE_DIR}/WinQuake/WinQuake_vc.rc
			DEPENDS WinQuake/WinQuake_vc.rc )
		set( SYSTEM_SOURCES ${SYSTEM_SOURCES} WinQuake_vc.o )
	else( "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" )
		set( SYSTEM_SOURCES ${SYSTEM_SOURCES} WinQuake/WinQuake_vc.rc )
	endif( "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" )

	find_path( DINPUT_INCLUDE_DIR dinput.h
		PATHS ENV DXSDK_DIR
		PATH_SUFFIXES Include )
	if( NOT DINPUT_INCLUDE_DIR )
		message( SEND_ERROR "Could not find DirectX header files" )
	else( NOT DINPUT_INCLUDE_DIR )
		include_directories( ${DINPUT_INCLUDE_DIR} )
	endif( NOT DINPUT_INCLUDE_DIR )
endif( WIN32 )

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/SciTech/include")

if( NOT NO_ASM )
	ADD_ASM_FILE( asm d_draw )
	ADD_ASM_FILE( asm d_draw16 )
	ADD_ASM_FILE( asm d_parta )
	ADD_ASM_FILE( asm d_polysa )
	ADD_ASM_FILE( asm d_scana )
	ADD_ASM_FILE( asm d_spr8 )
	ADD_ASM_FILE( asm d_varsa )
	ADD_ASM_FILE( asm math )
	ADD_ASM_FILE( asm r_aclipa )
	ADD_ASM_FILE( asm r_aliasa )
	ADD_ASM_FILE( asm r_drawa )
	ADD_ASM_FILE( asm r_edgea )
	ADD_ASM_FILE( asm r_varsa )
	ADD_ASM_FILE( asm snd_mixa )
	ADD_ASM_FILE( asm surf16 )
	ADD_ASM_FILE( asm surf8 )
	ADD_ASM_FILE( asm surf8b )
	ADD_ASM_FILE( asm surf8fst )
	ADD_ASM_FILE( asm surf8g )
	ADD_ASM_FILE( asm surf8r )
	ADD_ASM_FILE( asm surf8rgb )
	ADD_ASM_FILE( asm sys_wina )
	ADD_ASM_FILE( asm worlda )
else( NOT NO_ASM )
	add_definitions( -DNOASM )
endif( NOT NO_ASM )

set( NOT_COMPILED_SOURCE_FILES

	# ASM files are included here so they show up in the project
	asm/d_draw.s
	asm/d_draw16.s
	asm/d_parta.s
	asm/d_polysa.s
	asm/d_scana.s
	asm/d_spr8.s
	asm/d_varsa.s
	asm/math.s
	asm/r_aclipa.s
	asm/r_aliasa.s
	asm/r_drawa.s
	asm/r_edgea.s
	asm/r_varsa.s
	asm/snd_mixa.s
	asm/surf16.s
	asm/surf8.s
	asm/surf8b.s
	asm/surf8fst.s
	asm/surf8g.s
	asm/surf8r.s
	asm/surf8rgb.s
	asm/sys_wina.s
	asm/worlda.s
)

# Headers usually correspond to c files so we might as well just glob them
file( GLOB HEADER_SOURCES asm/*.h WinQuake/*.h midilib/*.h )

add_executable(engoo WIN32
	${ASM_SOURCES}
	${SYSTEM_SOURCES}
	${HEADER_SOURCES}
	${NOT_COMPILED_SOURCE_FILES}

	midilib/midi.c
	midilib/mpu401.c
	midilib/music.c

	WinQuake/bot.c
	WinQuake/cd_win.c
	WinQuake/chase.c
	WinQuake/cl_demo.c
	WinQuake/cl_input.c
	WinQuake/cl_main.c
	WinQuake/cl_parse.c
	WinQuake/cl_tent.c
	WinQuake/cmd.c
	WinQuake/common.c
	WinQuake/conproc.c
	WinQuake/console.c
	WinQuake/crc.c
	WinQuake/cvar.c
	WinQuake/d_edge.c
	WinQuake/d_fill.c
	WinQuake/d_init.c
	WinQuake/d_modech.c
	WinQuake/d_part.c
	WinQuake/d_polyse.c
	WinQuake/d_scan.c
	WinQuake/d_sky.c
	WinQuake/d_sprite.c
	WinQuake/d_surf.c
	WinQuake/d_vars.c
	WinQuake/d_zpoint.c
	WinQuake/draw.c
	WinQuake/host.c
	WinQuake/host_cmd.c
	WinQuake/in_win.c
	WinQuake/keys.c
	WinQuake/mathlib.c
	WinQuake/menu.c
	WinQuake/model.c
	WinQuake/model_common.c
	WinQuake/net_dgrm.c
	WinQuake/net_loop.c
	WinQuake/net_main.c
	WinQuake/net_vcr.c
	WinQuake/net_win.c
	WinQuake/net_wins.c
	WinQuake/net_wipx.c
	WinQuake/nvs_client.c
	WinQuake/nvs_common.c
	WinQuake/nvs_server.c
	WinQuake/nvs_server_data.c
	WinQuake/pr_cmds.c
	WinQuake/pr_edict.c
	WinQuake/pr_exec.c
	WinQuake/r_aclip.c
	WinQuake/r_alias.c
	WinQuake/r_bsp.c
	WinQuake/r_draw.c
	WinQuake/r_edge.c
	WinQuake/r_efrag.c
	WinQuake/r_light.c
	WinQuake/r_main.c
	WinQuake/r_misc.c
	WinQuake/r_part.c
	WinQuake/r_sky.c
	WinQuake/r_sprite.c
	WinQuake/r_surf.c
	WinQuake/r_vars.c
	WinQuake/sbar.c
	WinQuake/screen.c
	WinQuake/snd_dma.c
	WinQuake/snd_mem.c
	WinQuake/snd_mix.c
	WinQuake/snd_win.c
	WinQuake/sv_main.c
	WinQuake/sv_move.c
	WinQuake/sv_phys.c
	WinQuake/sv_user.c
	WinQuake/sys_win.c
	WinQuake/vid_win.c
	WinQuake/view.c
	WinQuake/wad.c
	WinQuake/world.c
	WinQuake/zone.c
)

set_source_files_properties( ${NOT_COMPILED_SOURCE_FILES} PROPERTIES HEADER_FILE_ONLY TRUE )

target_link_libraries( engoo "${CMAKE_CURRENT_SOURCE_DIR}/SciTech/lib/win32/vc/mgllt.lib" dxguid winmm wsock32 )

source_group("Assembler Files" REGULAR_EXPRESSION "^${CMAKE_CURRENT_SOURCE_DIR}/asm/.+$" )
source_group("Midi" REGULAR_EXPRESSION "^${CMAKE_CURRENT_SOURCE_DIR}/midilib/.+$" )
source_group("Resources Files" REGULAR_EXPRESSION "^${CMAKE_CURRENT_SOURCE_DIR}/WinQuake/.+.rc$" )
