# SPDX-License-Identifier: BSD-3-Clause

#add_library(xa_mp3_dec.a STATIC IMPORTED)                                                                                                     

add_local_sources(sof post_process.c lib_api.c)
target_link_libraries(sof_static_libraries INTERFACE  /work/repos/sof/sof/src/audio/post_process/xa_mp3_dec.a)
