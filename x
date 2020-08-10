Build in build_imx8x_gcc
-- Preparing Xtensa toolchain
-- The C compiler identification is GNU 8.1.0
-- The ASM compiler identification is GNU
-- Found assembler: /work/repos/sof/crosstool-ng/builds/xtensa-imx-elf/bin/xtensa-imx-elf-gcc
-- Found Git: /usr/bin/git (found version "2.17.1") 
-- Generating /work/repos/sof/sof/build_imx8x_gcc/generated/include/version.h
-- Found Python3: /usr/bin/python3.6 (found version "3.6.9") found components:  Interpreter 
-- Configuring done
-- Generating done
-- Build files have been written to: /work/repos/sof/sof/build_imx8x_gcc
Scanning dependencies of target imx8x_defconfig
[100%] Applying olddefconfig with imx8x_defconfig
Using existing configuration '.config' as base
Configuration written to '.config'
[100%] Built target imx8x_defconfig
-- Preparing Xtensa toolchain
-- Up-to-date /work/repos/sof/sof/build_imx8x_gcc/generated/include/version.h
-- Configuring done
-- Generating done
-- Build files have been written to: /work/repos/sof/sof/build_imx8x_gcc
Scanning dependencies of target genconfig
Scanning dependencies of target check_version_h
Scanning dependencies of target bin_extras
[  1%] Checking /work/repos/sof/sof/build_imx8x_gcc/generated/include/version.h
[  1%] Generating /work/repos/sof/sof/build_imx8x_gcc/generated/include/autoconfig.h
[  1%] Built target bin_extras
-- Up-to-date /work/repos/sof/sof/build_imx8x_gcc/generated/include/version.h
[  1%] Built target check_version_h
Scanning dependencies of target rimage_ep
Scanning dependencies of target smex_ep
[  2%] Creating directories for 'rimage_ep'
[  3%] Creating directories for 'smex_ep'
[  4%] Built target genconfig
Scanning dependencies of target ld_script_imx8.x
[  4%] Generating linker script: /work/repos/sof/sof/build_imx8x_gcc/imx8.x
Scanning dependencies of target reset
Scanning dependencies of target xlevel2
Scanning dependencies of target ext_manifest
Scanning dependencies of target xlevel3
[  4%] Built target ld_script_imx8.x
[  5%] Building ASM object src/arch/xtensa/CMakeFiles/reset.dir/xtos/memctl_default.S.o
[  5%] No download step for 'rimage_ep'
[  6%] Building ASM object src/arch/xtensa/xtos/CMakeFiles/xlevel2.dir/int-handler.S.o
[  6%] Building C object src/init/CMakeFiles/ext_manifest.dir/ext_manifest.c.o
[  6%] Building ASM object src/arch/xtensa/xtos/CMakeFiles/xlevel3.dir/int-handler.S.o
[  6%] No download step for 'smex_ep'
[  6%] No patch step for 'rimage_ep'
Scanning dependencies of target hal
[  6%] No patch step for 'smex_ep'
[  6%] Building C object src/arch/xtensa/hal/CMakeFiles/hal.dir/attribute.c.o
[  6%] Building ASM object src/arch/xtensa/CMakeFiles/reset.dir/xtos/reset-vector.S.o
[  7%] No update step for 'rimage_ep'
[  9%] No update step for 'smex_ep'
[  9%] Building ASM object src/arch/xtensa/xtos/CMakeFiles/xlevel2.dir/int-vector.S.o
[ 10%] Building ASM object src/arch/xtensa/xtos/CMakeFiles/xlevel3.dir/int-vector.S.o
[ 11%] Performing configure step for 'rimage_ep'
[ 12%] Linking C static library libext_manifest.a
[ 12%] Performing configure step for 'smex_ep'
[ 13%] Linking ASM static library libreset.a
[ 13%] Built target ext_manifest
[ 13%] Building ASM object src/arch/xtensa/xtos/CMakeFiles/xlevel2.dir/int-initlevel.S.o
[ 14%] Building C object src/arch/xtensa/hal/CMakeFiles/hal.dir/cache.c.o
[ 14%] Building ASM object src/arch/xtensa/xtos/CMakeFiles/xlevel3.dir/int-initlevel.S.o
Scanning dependencies of target xtos
[ 14%] Built target reset
[ 14%] Building ASM object src/arch/xtensa/xtos/CMakeFiles/xtos.dir/core-restore.S.o
[ 14%] Building ASM object src/arch/xtensa/hal/CMakeFiles/hal.dir/cache_asm.S.o
[ 15%] Building ASM object src/arch/xtensa/xtos/CMakeFiles/xtos.dir/core-save.S.o
[ 17%] Linking ASM static library libxlevel2.a
[ 19%] Linking ASM static library libxlevel3.a
[ 19%] Building ASM object src/arch/xtensa/hal/CMakeFiles/hal.dir/clock.S.o
[ 19%] Built target xlevel2
[ 19%] Built target xlevel3
[ 19%] Building ASM object src/arch/xtensa/xtos/CMakeFiles/xtos.dir/core-shutoff.S.o
[ 20%] Building ASM object src/arch/xtensa/xtos/CMakeFiles/xtos.dir/double-vector.S.o
[ 20%] Building C object src/arch/xtensa/hal/CMakeFiles/hal.dir/coherence.c.o
[ 21%] Building C object src/arch/xtensa/hal/CMakeFiles/hal.dir/debug.c.o
Scanning dependencies of target data_structs
[ 21%] Building C object src/ipc/CMakeFiles/data_structs.dir/cc_version.c.o
[ 21%] Building ASM object src/arch/xtensa/xtos/CMakeFiles/xtos.dir/debug-vector.S.o
[ 22%] Building C object src/ipc/CMakeFiles/data_structs.dir/probe_support.c.o
-- The C compiler identification is GNU 7.5.0
-- The C compiler identification is GNU 7.5.0
[ 23%] Building ASM object src/arch/xtensa/xtos/CMakeFiles/xtos.dir/xea1/exc-alloca-handler.S.o
[ 23%] Building C object src/ipc/CMakeFiles/data_structs.dir/user_abi_version.c.o
[ 23%] Building ASM object src/arch/xtensa/xtos/CMakeFiles/xtos.dir/xea1/exc-c-wrapper-handler.S.o
[ 23%] Building ASM object src/arch/xtensa/hal/CMakeFiles/hal.dir/debug_hndlr.S.o
[ 25%] Building C object src/arch/xtensa/hal/CMakeFiles/hal.dir/disass.c.o
-- Check for working C compiler: /usr/bin/cc
-- Check for working C compiler: /usr/bin/cc
[ 25%] Building ASM object src/arch/xtensa/xtos/CMakeFiles/xtos.dir/xea2/exc-c-wrapper-handler.S.o
[ 26%] Building ASM object src/arch/xtensa/xtos/CMakeFiles/xtos.dir/xea1/exc-return.S.o
[ 26%] Linking C static library libdata_structs.a
[ 26%] Building ASM object src/arch/xtensa/hal/CMakeFiles/hal.dir/int_asm.S.o
[ 26%] Building ASM object src/arch/xtensa/xtos/CMakeFiles/xtos.dir/xea2/exc-return.S.o
[ 26%] Building C object src/arch/xtensa/hal/CMakeFiles/hal.dir/interrupts.c.o
[ 26%] Built target data_structs
[ 27%] Building C object src/arch/xtensa/xtos/CMakeFiles/xtos.dir/exc-sethandler.c.o
[ 28%] Building ASM object src/arch/xtensa/hal/CMakeFiles/hal.dir/memcopy.S.o
[ 28%] Building ASM object src/arch/xtensa/hal/CMakeFiles/hal.dir/mem_ecc_parity.S.o
[ 28%] Building ASM object src/arch/xtensa/xtos/CMakeFiles/xtos.dir/exc-syscall-handler.S.o
[ 29%] Building C object src/arch/xtensa/hal/CMakeFiles/hal.dir/misc.c.o
[ 29%] Building ASM object src/arch/xtensa/hal/CMakeFiles/hal.dir/miscellaneous.S.o
[ 30%] Building C object src/arch/xtensa/hal/CMakeFiles/hal.dir/mmu.c.o
[ 31%] Building ASM object src/arch/xtensa/xtos/CMakeFiles/xtos.dir/exc-table.S.o
-- Check for working C compiler: /usr/bin/cc -- works
-- Check for working C compiler: /usr/bin/cc -- works
[ 31%] Building ASM object src/arch/xtensa/hal/CMakeFiles/hal.dir/mp_asm.S.o
-- Detecting C compiler ABI info
[ 32%] Building ASM object src/arch/xtensa/hal/CMakeFiles/hal.dir/mpu_asm.S.o
[ 32%] Building C object src/arch/xtensa/hal/CMakeFiles/hal.dir/mpu.c.o
-- Detecting C compiler ABI info
[ 32%] Building ASM object src/arch/xtensa/xtos/CMakeFiles/xtos.dir/exc-unhandled.S.o
[ 34%] Building C object src/arch/xtensa/hal/CMakeFiles/hal.dir/set_region_translate.c.o
[ 34%] Building ASM object src/arch/xtensa/hal/CMakeFiles/hal.dir/state_asm.S.o
[ 35%] Building ASM object src/arch/xtensa/xtos/CMakeFiles/xtos.dir/interrupt-table.S.o
[ 36%] Building C object src/arch/xtensa/hal/CMakeFiles/hal.dir/state.c.o
[ 36%] Building C object src/arch/xtensa/xtos/CMakeFiles/xtos.dir/int-sethandler.c.o
[ 37%] Building ASM object src/arch/xtensa/xtos/CMakeFiles/xtos.dir/xea1/intlevel-restore.S.o
[ 37%] Building ASM object src/arch/xtensa/xtos/CMakeFiles/xtos.dir/xea2/intlevel-restore.S.o
[ 37%] Building ASM object src/arch/xtensa/hal/CMakeFiles/hal.dir/syscache_asm.S.o
[ 38%] Building ASM object src/arch/xtensa/hal/CMakeFiles/hal.dir/windowspill_asm.S.o
[ 39%] Building ASM object src/arch/xtensa/xtos/CMakeFiles/xtos.dir/xea1/intlevel-setmin.S.o
[ 39%] Building ASM object src/arch/xtensa/xtos/CMakeFiles/xtos.dir/xea2/intlevel-setmin.S.o
-- Detecting C compiler ABI info - done
-- Detecting C compiler ABI info - done
[ 39%] Building ASM object src/arch/xtensa/xtos/CMakeFiles/xtos.dir/xea2/intlevel-set.S.o
[ 40%] Building ASM object src/arch/xtensa/xtos/CMakeFiles/xtos.dir/xea1/intlevel-set.S.o
[ 40%] Building ASM object src/arch/xtensa/xtos/CMakeFiles/xtos.dir/xea1/int-lowpri-dispatcher.S.o
[ 40%] Linking C static library libhal.a
[ 42%] Building ASM object src/arch/xtensa/xtos/CMakeFiles/xtos.dir/xea2/int-lowpri-dispatcher.S.o
[ 42%] Building ASM object src/arch/xtensa/xtos/CMakeFiles/xtos.dir/ints-off.S.o
[ 43%] Building ASM object src/arch/xtensa/xtos/CMakeFiles/xtos.dir/ints-on.S.o
[ 43%] Building ASM object src/arch/xtensa/xtos/CMakeFiles/xtos.dir/kernel-vector.S.o
-- Detecting C compile features
[ 43%] Built target hal
[ 44%] Building ASM object src/arch/xtensa/xtos/CMakeFiles/xtos.dir/memep-enable.S.o
-- Detecting C compile features
[ 44%] Building ASM object src/arch/xtensa/xtos/CMakeFiles/xtos.dir/memep-initrams.S.o
[ 45%] Building ASM object src/arch/xtensa/xtos/CMakeFiles/xtos.dir/memerror-vector.S.o
[ 45%] Building ASM object src/arch/xtensa/xtos/CMakeFiles/xtos.dir/nmi-vector.S.o
[ 46%] Building ASM object src/arch/xtensa/xtos/CMakeFiles/xtos.dir/xea2/reloc-vectors.S.o
[ 46%] Building ASM object src/arch/xtensa/xtos/CMakeFiles/xtos.dir/user-vector.S.o
[ 47%] Building ASM object src/arch/xtensa/xtos/CMakeFiles/xtos.dir/xea1/window-vectors.S.o
[ 47%] Building ASM object src/arch/xtensa/xtos/CMakeFiles/xtos.dir/xea2/window-vectors.S.o
[ 48%] Linking C static library libxtos.a
[ 48%] Built target xtos
-- Detecting C compile features - done
-- Configuring done
-- Detecting C compile features - done
-- No CMAKE_BUILD_TYPE, defaulting to Debug
-- Generating done
-- Build files have been written to: /work/repos/sof/sof/build_imx8x_gcc/smex_ep/build
-- Configuring done
[ 50%] Performing build step for 'smex_ep'
-- Generating done
-- Build files have been written to: /work/repos/sof/sof/build_imx8x_gcc/rimage_ep/build
[ 50%] Performing build step for 'rimage_ep'
Scanning dependencies of target smex
[ 25%] Building C object CMakeFiles/smex.dir/smex.c.o
[ 50%] Building C object CMakeFiles/smex.dir/ldc.c.o
[ 75%] Building C object CMakeFiles/smex.dir/elf.c.o
Scanning dependencies of target rimage
[ 12%] Building C object CMakeFiles/rimage.dir/src/man_cnl.c.o
[ 12%] Building C object CMakeFiles/rimage.dir/src/man_apl.c.o
[ 18%] Building C object CMakeFiles/rimage.dir/src/file_simple.c.o
[ 25%] Building C object CMakeFiles/rimage.dir/src/man_kbl.c.o
[ 37%] Building C object CMakeFiles/rimage.dir/src/man_sue.c.o
[ 37%] Building C object CMakeFiles/rimage.dir/src/man_tgl.c.o
[ 50%] Building C object CMakeFiles/rimage.dir/src/cse.c.o
[ 50%] Building C object CMakeFiles/rimage.dir/src/css.c.o
Scanning dependencies of target sof
[ 56%] Building C object CMakeFiles/rimage.dir/src/plat_auth.c.o
[ 62%] Building C object CMakeFiles/rimage.dir/src/hash.c.o
[ 68%] Building C object CMakeFiles/rimage.dir/src/pkcs1_5.c.o
[ 50%] Building C object CMakeFiles/sof.dir/src/platform/imx8/lib/clk.c.o
[ 51%] Building C object CMakeFiles/sof.dir/src/platform/imx8/lib/dai.c.o
[ 75%] Building C object CMakeFiles/rimage.dir/src/manifest.c.o
[ 81%] Building C object CMakeFiles/rimage.dir/src/ext_manifest.c.o
[ 51%] Building C object CMakeFiles/sof.dir/src/platform/imx8/lib/dma.c.o
[ 52%] Building C object CMakeFiles/sof.dir/src/platform/imx8/lib/memory.c.o
[ 87%] Building C object CMakeFiles/rimage.dir/src/elf.c.o
[100%] Linking C executable smex
[ 93%] Building C object CMakeFiles/rimage.dir/src/rimage.c.o
[ 52%] Building C object CMakeFiles/sof.dir/src/platform/imx8/platform.c.o
[100%] Built target smex
[ 52%] No install step for 'smex_ep'
[ 52%] Building C object CMakeFiles/sof.dir/src/arch/xtensa/drivers/timer.c.o
[ 53%] Building C object CMakeFiles/sof.dir/src/arch/xtensa/drivers/interrupt.c.o
[ 55%] Completed 'smex_ep'
[ 55%] Building C object CMakeFiles/sof.dir/src/arch/xtensa/lib/notifier.c.o
[ 55%] Built target smex_ep
[ 55%] Building C object CMakeFiles/sof.dir/src/arch/xtensa/schedule/schedule.c.o
[ 56%] Building C object CMakeFiles/sof.dir/src/arch/xtensa/schedule/task.c.o
[ 56%] Building ASM object CMakeFiles/sof.dir/src/arch/xtensa/xtos/crt1-boards.S.o
[ 57%] Building ASM object CMakeFiles/sof.dir/src/arch/xtensa/xtos/_vectors.S.o
[ 57%] Building C object CMakeFiles/sof.dir/src/arch/xtensa/init.c.o
[100%] Linking C executable rimage
[ 57%] Building ASM object CMakeFiles/sof.dir/src/arch/xtensa/exc-dump.S.o
[ 59%] Building C object CMakeFiles/sof.dir/src/ipc/ipc.c.o
[ 59%] Building C object CMakeFiles/sof.dir/src/ipc/handler.c.o
[ 60%] Building C object CMakeFiles/sof.dir/src/ipc/dma-copy.c.o
[ 60%] Building C object CMakeFiles/sof.dir/src/ipc/ipc-host-ptable.c.o
[ 61%] Building C object CMakeFiles/sof.dir/src/audio/host.c.o
[ 61%] Building C object CMakeFiles/sof.dir/src/audio/pipeline.c.o
[100%] Built target rimage
[ 62%] Building C object CMakeFiles/sof.dir/src/audio/pipeline_static.c.o
[ 62%] No install step for 'rimage_ep'
[ 62%] Completed 'rimage_ep'
[ 62%] Built target rimage_ep
[ 62%] Building C object CMakeFiles/sof.dir/src/audio/component.c.o
[ 63%] Building C object CMakeFiles/sof.dir/src/audio/buffer.c.o
[ 63%] Building C object CMakeFiles/sof.dir/src/audio/channel_map.c.o
[ 64%] Building C object CMakeFiles/sof.dir/src/audio/volume/volume_generic.c.o
[ 64%] Building C object CMakeFiles/sof.dir/src/audio/volume/volume_hifi3.c.o
[ 64%] Building C object CMakeFiles/sof.dir/src/audio/volume/volume.c.o
[ 65%] Building C object CMakeFiles/sof.dir/src/audio/src/src_generic.c.o
[ 65%] Building C object CMakeFiles/sof.dir/src/audio/src/src_hifi2ep.c.o
[ 67%] Building C object CMakeFiles/sof.dir/src/audio/src/src_hifi3.c.o
[ 67%] Building C object CMakeFiles/sof.dir/src/audio/src/src.c.o
[ 68%] Building C object CMakeFiles/sof.dir/src/audio/eq_fir/eq_fir.c.o
[ 68%] Building C object CMakeFiles/sof.dir/src/audio/eq_fir/fir_hifi2ep.c.o
[ 69%] Building C object CMakeFiles/sof.dir/src/audio/eq_fir/fir_hifi3.c.o
[ 69%] Building C object CMakeFiles/sof.dir/src/audio/eq_fir/fir.c.o
[ 70%] Building C object CMakeFiles/sof.dir/src/audio/eq_iir/eq_iir.c.o
[ 70%] Building C object CMakeFiles/sof.dir/src/audio/eq_iir/iir.c.o
[ 71%] Building C object CMakeFiles/sof.dir/src/audio/eq_iir/iir_generic.c.o
[ 71%] Building C object CMakeFiles/sof.dir/src/audio/eq_iir/iir_hifi3.c.o
[ 72%] Building C object CMakeFiles/sof.dir/src/audio/dcblock/dcblock.c.o
[ 72%] Building C object CMakeFiles/sof.dir/src/audio/dcblock/dcblock_generic.c.o
[ 72%] Building C object CMakeFiles/sof.dir/src/audio/tone.c.o
[ 73%] Building C object CMakeFiles/sof.dir/src/audio/mixer.c.o
[ 73%] Building C object CMakeFiles/sof.dir/src/audio/mux/mux.c.o
[ 75%] Building C object CMakeFiles/sof.dir/src/audio/mux/mux_generic.c.o
[ 75%] Building C object CMakeFiles/sof.dir/src/audio/switch.c.o
[ 76%] Building C object CMakeFiles/sof.dir/src/audio/dai.c.o
[ 76%] Building C object CMakeFiles/sof.dir/src/audio/kpb.c.o
[ 77%] Building C object CMakeFiles/sof.dir/src/audio/selector/selector_generic.c.o
[ 77%] Building C object CMakeFiles/sof.dir/src/audio/selector/selector.c.o
[ 78%] Building C object CMakeFiles/sof.dir/src/audio/detect_test.c.o
[ 78%] Building C object CMakeFiles/sof.dir/src/audio/pcm_converter/pcm_converter.c.o
[ 79%] Building C object CMakeFiles/sof.dir/src/audio/pcm_converter/pcm_converter_generic.c.o
[ 79%] Building C object CMakeFiles/sof.dir/src/audio/pcm_converter/pcm_converter_hifi3.c.o
[ 80%] Building C object CMakeFiles/sof.dir/src/audio/asrc/asrc.c.o
[ 80%] Building C object CMakeFiles/sof.dir/src/audio/asrc/asrc_farrow.c.o
[ 80%] Building C object CMakeFiles/sof.dir/src/audio/asrc/asrc_farrow_generic.c.o
[ 81%] Building C object CMakeFiles/sof.dir/src/audio/asrc/asrc_farrow_hifi3.c.o
[ 81%] Building C object CMakeFiles/sof.dir/src/audio/post_process/post_process.c.o
[ 82%] Building C object CMakeFiles/sof.dir/src/audio/post_process/lib_api.c.o
[ 82%] Building C object CMakeFiles/sof.dir/src/lib/lib.c.o
[ 84%] Building C object CMakeFiles/sof.dir/src/lib/alloc.c.o
[ 84%] Building C object CMakeFiles/sof.dir/src/lib/notifier.c.o
[ 85%] Building C object CMakeFiles/sof.dir/src/lib/pm_runtime.c.o
[ 85%] Building C object CMakeFiles/sof.dir/src/lib/clk.c.o
[ 86%] Building C object CMakeFiles/sof.dir/src/lib/dma.c.o
[ 86%] Building C object CMakeFiles/sof.dir/src/lib/dai.c.o
[ 87%] Building C object CMakeFiles/sof.dir/src/lib/wait.c.o
[ 87%] Building C object CMakeFiles/sof.dir/src/math/numbers.c.o
[ 87%] Building C object CMakeFiles/sof.dir/src/math/trig.c.o
[ 88%] Building C object CMakeFiles/sof.dir/src/math/decibels.c.o
[ 88%] Building C object CMakeFiles/sof.dir/src/spinlock.c.o
[ 89%] Building C object CMakeFiles/sof.dir/src/debug/panic.c.o
[ 89%] Building C object CMakeFiles/sof.dir/src/drivers/imx/esai.c.o
[ 90%] Building C object CMakeFiles/sof.dir/src/drivers/imx/sai.c.o
[ 90%] Building C object CMakeFiles/sof.dir/src/drivers/imx/interrupt.c.o
[ 92%] Building C object CMakeFiles/sof.dir/src/drivers/imx/ipc.c.o
[ 92%] Building C object CMakeFiles/sof.dir/src/drivers/imx/timer.c.o
[ 93%] Building C object CMakeFiles/sof.dir/src/drivers/imx/sdma.c.o
[ 93%] Building C object CMakeFiles/sof.dir/src/drivers/imx/edma.c.o
[ 94%] Building C object CMakeFiles/sof.dir/src/drivers/generic/dummy-dma.c.o
[ 94%] Building C object CMakeFiles/sof.dir/src/drivers/interrupt.c.o
[ 95%] Building C object CMakeFiles/sof.dir/src/init/init.c.o
[ 95%] Building C object CMakeFiles/sof.dir/src/schedule/dma_multi_chan_domain.c.o
[ 95%] Building C object CMakeFiles/sof.dir/src/schedule/dma_single_chan_domain.c.o
[ 96%] Building C object CMakeFiles/sof.dir/src/schedule/edf_schedule.c.o
[ 96%] Building C object CMakeFiles/sof.dir/src/schedule/ll_schedule.c.o
[ 97%] Building C object CMakeFiles/sof.dir/src/schedule/schedule.c.o
[ 97%] Building C object CMakeFiles/sof.dir/src/schedule/task.c.o
[ 98%] Building C object CMakeFiles/sof.dir/src/schedule/timer_domain.c.o
[ 98%] Building C object CMakeFiles/sof.dir/src/trace/dma-trace.c.o
[100%] Building C object CMakeFiles/sof.dir/src/trace/trace.c.o
[100%] Linking C executable sof
/work/repos/sof/crosstool-ng/builds/xtensa-imx-elf/lib/gcc/xtensa-imx-elf/8.1.0/../../../../xtensa-imx-elf/bin/ld: cannot find -lxa_mp3_dec
collect2: error: ld returned 1 exit status
CMakeFiles/sof.dir/build.make:1432: recipe for target 'sof' failed
make[3]: *** [sof] Error 1
CMakeFiles/Makefile2:401: recipe for target 'CMakeFiles/sof.dir/all' failed
make[2]: *** [CMakeFiles/sof.dir/all] Error 2
CMakeFiles/Makefile2:1663: recipe for target 'src/arch/xtensa/CMakeFiles/bin.dir/rule' failed
make[1]: *** [src/arch/xtensa/CMakeFiles/bin.dir/rule] Error 2
Makefile:697: recipe for target 'bin' failed
make: *** [bin] Error 2
