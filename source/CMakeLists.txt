cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_COMPILER "clang++")
add_definitions("-std=c++14 -fno-exceptions -fno-rtti -Wextra -Ofast -MMD -MP")
add_definitions("-Wno-unused-parameter -D_LINUX -DGODWHALE_CLUSTER_SLAVE -DUSE_CMAKE")
set(CMAKE_EXE_LINKER_FLAGS "-v")

if(NOT DEFINED target)
   set(target "tournament")
endif()
if(target STREQUAL tournament)
  add_definitions("-DNDEBUG -DUSE_AVX2 -mbmi2 -mavx2 -DFOR_TOURNAMENT -march=corei7-avx")
elseif(target STREQUAL tournament-sse42)
  add_definitions("-DNDEBUG -DUSE_SSE42 -msse4.2 -DFOR_TOURNAMENT -march=corei7")
elseif(DEFINED target)
  message(FATAL_ERROR "invalid target")
endif()

if (NOT "$ENV{MSYSTEM}" STREQUAL "")
  set(CMAKE_EXE_LINKER_FLAGS "-Wl,--stack,100000000")
  add_definitions("-fno-threadsafe-statics -DUSE_MSYS2 -DBOOST_USE_WINDOWS_H -DBOOST_ERROR_CODE_HEADER_ONLY")
endif()

set(sources
	shogi.cpp
	bitboard.cpp
	misc.cpp
	movegen.cpp
	position.cpp
	usi.cpp
	thread.cpp
	tt.cpp
	move_picker.cpp
	extra/book/apery_book.cpp
	extra/book/book.cpp
	extra/bitop.cpp
	extra/entering_king_win.cpp
	extra/long_effect.cpp
	extra/mate/mate1ply_with_effect.cpp
	extra/mate/mate1ply_without_effect.cpp
	extra/mate/mate_n_ply.cpp
	extra/benchmark.cpp
	extra/test_cmd.cpp
	extra/timeman.cpp
	extra/see.cpp
	extra/sfen_packer.cpp
	extra/kif_converter/kif_convert_tools.cpp
	eval/evaluate_bona_piece.cpp
	eval/kppt/evaluate_kppt.cpp
	eval/kppt/evaluate_kppt_learner.cpp
	eval/kpp_kkpt/evaluate_kpp_kkpt.cpp
	eval/kpp_kkpt/evaluate_kpp_kkpt_learner.cpp
	eval/kpppt/evaluate_kpppt.cpp
	eval/kpppt/evaluate_kpppt_learner.cpp
	eval/kppp_kkpt/evaluate_kppp_kkpt.cpp
	eval/kppp_kkpt/evaluate_kppp_kkpt_learner.cpp
	eval/kkpp_kkpt/evaluate_kkpp_kkpt.cpp
	eval/kkpp_kkpt/evaluate_kkpp_kkpt_learner.cpp
	eval/kkppt/evaluate_kkppt.cpp
	eval/kkppt/evaluate_kkppt_learner.cpp
	eval/kpp_kkpt_fv_var/evaluate_kpp_kkpt_fv_var.cpp
	eval/kpp_kkpt_fv_var/evaluate_kpp_kkpt_fv_var_learner.cpp
	eval/evaluate.cpp
	eval/evaluate_io.cpp
	eval/evaluate_mir_inv_tools.cpp
	engine/user-engine/user-search.cpp
	engine/help-mate-engine/help-mate-search.cpp
	engine/2018-otafuku-engine/2018-otafuku-search.cpp
	learn/learner.cpp
	learn/learning_tools.cpp
	learn/multi_think.cpp
	godwhale/godwhale_io.cpp
)
add_executable(godwhale_child ${sources})

if (NOT "$ENV{MSYSTEM}" STREQUAL "")
  target_link_libraries(godwhale_child -static pthread boost_system-mt ws2_32)
else()
  target_link_libraries(godwhale_child pthread boost_system)
endif()
