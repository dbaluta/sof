Found west: /home/nxf25322/.local/bin/west
Building platforms: imx8m
In dir: /home/nxf25322/work/sof_dir; running command:
    west topdir
In dir: /home/nxf25322/work/sof_dir; running command:
    west config manifest.path
In dir: /home/nxf25322/work/sof_dir; running command:
    west config manifest.file
West workspace: /home/nxf25322/work/sof_dir
West manifest path: /home/nxf25322/work/sof_dir/sof/west.yml
In dir: /home/nxf25322/work/sof_dir; running command:
    cmake --build build-rimage
ninja: no work to do.
SOF_TOP=/home/nxf25322/work/sof_dir/sof
west_top=/home/nxf25322/work/sof_dir
In dir: /home/nxf25322/work/sof_dir; running command:
    west config -l
... with extra/modified environment:
WEST_CONFIG_LOCAL=/home/nxf25322/work/sof_dir/build-sof-staging/sof-info/imx8m/westconfig.ini

manifest.path=sof
manifest.file=west.yml
zephyr.base=zephyr
rimage.path=/home/nxf25322/work/sof_dir/build-rimage/rimage
rimage.extra-args=-k 'key param ignored by imx8m' -f 2.8.99 -b 1

In dir: /home/nxf25322/work/sof_dir; running command:
    west build --build-dir build-imx8m --board nxp_adsp_imx8m /home/nxf25322/work/sof_dir/sof/app -p always --
... with extra/modified environment:
WEST_CONFIG_LOCAL=/home/nxf25322/work/sof_dir/build-sof-staging/sof-info/imx8m/westconfig.ini

-- west build: making build dir /home/nxf25322/work/sof_dir/build-imx8m pristine
-- west build: generating a build system
Loading Zephyr default modules (Zephyr base).
-- Application: /home/nxf25322/work/sof_dir/sof/app
-- CMake version: 3.28.1
-- Found Python3: /usr/bin/python3 (found suitable version "3.8.10", minimum required is "3.8") found components: Interpreter 
-- Cache files will be written to: /home/nxf25322/.cache/zephyr
-- Zephyr version: 3.6.0-rc2 (/home/nxf25322/work/sof_dir/zephyr)
-- Found west (found suitable version "1.2.0", minimum required is "0.14.0")
-- Board: nxp_adsp_imx8m
-- ZEPHYR_TOOLCHAIN_VARIANT not set, trying to locate Zephyr SDK
-- Found host-tools: zephyr 0.16.4 (/home/nxf25322/zephyr-sdk-0.16.4)
-- Found toolchain: zephyr 0.16.4 (/home/nxf25322/zephyr-sdk-0.16.4)
-- Found Dtc: /home/nxf25322/zephyr-sdk-0.16.4/sysroots/x86_64-pokysdk-linux/usr/bin/dtc (found suitable version "1.6.0", minimum required is "1.4.6") 
-- Found BOARD.dts: /home/nxf25322/work/sof_dir/zephyr/boards/xtensa/nxp_adsp_imx8m/nxp_adsp_imx8m.dts
-- Found devicetree overlay: /home/nxf25322/work/sof_dir/sof/app/boards/nxp_adsp_imx8m.overlay
-- Generated zephyr.dts: /home/nxf25322/work/sof_dir/build-imx8m/zephyr/zephyr.dts
-- Generated devicetree_generated.h: /home/nxf25322/work/sof_dir/build-imx8m/zephyr/include/generated/devicetree_generated.h
-- Including generated dts.cmake file: /home/nxf25322/work/sof_dir/build-imx8m/zephyr/dts.cmake

warning: SCHED_CPU_MASK_PIN_ONLY (defined at kernel/Kconfig:139) was assigned the value 'y' but got
the value 'n'. Check these unsatisfied dependencies: SMP (=n). See
http://docs.zephyrproject.org/latest/kconfig.html#CONFIG_SCHED_CPU_MASK_PIN_ONLY and/or look up
SCHED_CPU_MASK_PIN_ONLY in the menuconfig/guiconfig interface. The Application Development Primer,
Setting Configuration Values, and Kconfig - Tips and Best Practices sections of the manual might be
helpful too.


warning: SMP_BOOT_DELAY (defined at kernel/Kconfig:1175) was assigned the value 'y' but got the
value 'n'. Check these unsatisfied dependencies: SMP (=n). See
http://docs.zephyrproject.org/latest/kconfig.html#CONFIG_SMP_BOOT_DELAY and/or look up
SMP_BOOT_DELAY in the menuconfig/guiconfig interface. The Application Development Primer, Setting
Configuration Values, and Kconfig - Tips and Best Practices sections of the manual might be helpful
too.

Parsing /home/nxf25322/work/sof_dir/zephyr/Kconfig
Loaded configuration '/home/nxf25322/work/sof_dir/zephyr/boards/xtensa/nxp_adsp_imx8m/nxp_adsp_imx8m_defconfig'
Merged configuration '/home/nxf25322/work/sof_dir/sof/app/prj.conf'
Merged configuration '/home/nxf25322/work/sof_dir/sof/app/boards/nxp_adsp_imx8m.conf'
Configuration saved to '/home/nxf25322/work/sof_dir/build-imx8m/zephyr/.config'
Kconfig header saved to '/home/nxf25322/work/sof_dir/build-imx8m/zephyr/include/generated/autoconf.h'
-- Found GnuLd: /home/nxf25322/zephyr-sdk-0.16.4/xtensa-nxp_imx8m_adsp_zephyr-elf/bin/../lib/gcc/xtensa-nxp_imx8m_adsp_zephyr-elf/12.2.0/../../../../xtensa-nxp_imx8m_adsp_zephyr-elf/bin/ld.bfd (found version "2.38") 
-- The C compiler identification is GNU 12.2.0
-- The CXX compiler identification is GNU 12.2.0
-- The ASM compiler identification is GNU
-- Found assembler: /home/nxf25322/zephyr-sdk-0.16.4/xtensa-nxp_imx8m_adsp_zephyr-elf/bin/xtensa-nxp_imx8m_adsp_zephyr-elf-gcc
-- Found Python3: /usr/bin/python3 (found version "3.8.10") found components: Interpreter 
-- SOF version.cmake starting at 2024-03-13T13:29:10Z UTC
-- /home/nxf25322/work/sof_dir/sof/ is at git commit with parent(s):
commit 12f39114421ca6cd0b6db2b21a4b8bb01185c31c 35cd430034dd98c00d731b6e11fb2fb393973d8f (HEAD -> native_drivers_test_branch)
Author: Daniel Baluta <daniel.baluta@nxp.com>
Date:   Tue Feb 27 18:48:51 2024 +0200

    nxp: imx8m: Add nodoes for native HOST_DMA
    
    This is required for switching to Zephyr native drivers
    
    Signed-off-by: Daniel Baluta <daniel.baluta@nxp.com>
-- GIT_TAG / GIT_LOG_HASH : v2.5-stable-branch-2096-g12f39114421c-dirty / 12f391144
-- Source content hash: f4597c4f. Notes:
  - by design, source hash is broken by Kconfig changes. See #3890.
  - Source hash is also broken by _asymmetric_ autocrlf=input, see
    #5917 and reverted #5920.
-- Generated new /home/nxf25322/work/sof_dir/build-imx8m/zephyr/include/generated/sof_versions.h
Load components for MIMX8ML8:
driver_common component is included.
driver_reset component is included.
driver_iuart component is included.
driver_sdma component is included.
driver_memory component is included.
-- Configuring done (2.3s)
-- Generating done (0.2s)
-- Build files have been written to: /home/nxf25322/work/sof_dir/build-imx8m
-- west build: building application
[1/295] Preparing syscall dependency handling

[2/295] Creating directories for 'sof_logger_ep'
[3/295] Creating directories for 'smex_ep'
[4/295] Generating ../../../../include/generated/core-isa-dM.h
[5/295] No download step for 'sof_logger_ep'
[6/295] Generating include/generated/version.h
-- Zephyr version: 3.6.0-rc2 (/home/nxf25322/work/sof_dir/zephyr), build: v3.6.0-rc2-116-g64dba72c7aee
[7/295] No update step for 'sof_logger_ep'
[8/295] No patch step for 'sof_logger_ep'
[9/295] No download step for 'smex_ep'
[10/295] No update step for 'smex_ep'
[11/295] No patch step for 'smex_ep'
[12/295] Generating ../../../../include/generated/zsr.h
[13/295] Generating misc/generated/syscalls_subdirs.trigger
[14/295] Performing configure step for 'smex_ep'
-- The C compiler identification is GNU 9.4.0
-- Detecting C compiler ABI info
-- Detecting C compiler ABI info - done
-- Check for working C compiler: /usr/bin/cc - skipped
-- Detecting C compile features
-- Detecting C compile features - done
-- No CMAKE_BUILD_TYPE, defaulting to Debug
-- Configuring done (0.2s)
-- Generating done (0.0s)
-- Build files have been written to: /home/nxf25322/work/sof_dir/build-imx8m/zephyr/smex_ep/build
[15/295] Generating misc/generated/syscalls.json, misc/generated/struct_tags.json
[16/295] Generating include/generated/syscall_dispatch.c, include/generated/syscall_list.h
[17/295] Performing build step for 'smex_ep'
[1/4] Building C object CMakeFiles/smex.dir/smex.c.o
[2/4] Building C object CMakeFiles/smex.dir/ldc.c.o
[3/4] Building C object CMakeFiles/smex.dir/elf.c.o
[4/4] Linking C executable smex
[18/295] Generating include/generated/driver-validation.h
[19/295] Generating include/generated/kobj-types-enum.h, include/generated/otype-to-str.h, include/generated/otype-to-size.h
[20/295] No install step for 'smex_ep'
[21/295] Completed 'smex_ep'
[22/295] Performing configure step for 'sof_logger_ep'
-- The C compiler identification is GNU 9.4.0
-- Detecting C compiler ABI info
-- Detecting C compiler ABI info - done
-- Check for working C compiler: /usr/bin/cc - skipped
-- Detecting C compile features
-- Detecting C compile features - done
-- No CMAKE_BUILD_TYPE, defaulting to Debug
CMake Warning at topology/CMakeLists.txt:39 (message):
  All topologies skipped: minimum alsatplg version 1.2.5, found 1.2.2.


-- Looking for include file sys/inotify.h
-- Looking for include file sys/inotify.h - found
-- Configuring done (0.3s)
-- Generating done (0.1s)
-- Build files have been written to: /home/nxf25322/work/sof_dir/build-imx8m/zephyr/sof-logger_ep/build
[23/295] Building C object zephyr/CMakeFiles/offsets.dir/arch/xtensa/core/offsets/offsets.c.obj
[24/295] Generating include/generated/offsets.h
[25/295] Performing build step for 'sof_logger_ep'
[1/5] Building C object logger/CMakeFiles/sof-logger.dir/misc.c.o
[2/5] Building C object logger/CMakeFiles/sof-logger.dir/filter.c.o
[3/5] Building C object logger/CMakeFiles/sof-logger.dir/logger.c.o
[4/5] Building C object logger/CMakeFiles/sof-logger.dir/convert.c.o
[5/5] Linking C executable logger/sof-logger
[26/295] Building C object CMakeFiles/app.dir/src/main.c.obj
[27/295] Building C object zephyr/CMakeFiles/zephyr.dir/lib/os/printk.c.obj
[28/295] Building C object zephyr/CMakeFiles/zephyr.dir/lib/os/sem.c.obj
[29/295] Building C object zephyr/CMakeFiles/zephyr.dir/lib/os/assert.c.obj
[30/295] Building C object zephyr/CMakeFiles/zephyr.dir/lib/os/thread_entry.c.obj
[31/295] Building C object zephyr/CMakeFiles/zephyr.dir/lib/os/cbprintf.c.obj
[32/295] Linking C static library app/libapp.a
[33/295] Building C object zephyr/CMakeFiles/zephyr.dir/lib/utils/hex.c.obj
[34/295] Building C object zephyr/CMakeFiles/zephyr.dir/lib/utils/timeutil.c.obj
[35/295] Building C object zephyr/CMakeFiles/zephyr.dir/lib/utils/dec.c.obj
[36/295] Building C object zephyr/CMakeFiles/zephyr.dir/misc/generated/configs.c.obj
[37/295] Building C object zephyr/CMakeFiles/zephyr.dir/lib/os/p4wq.c.obj
[38/295] Building C object zephyr/CMakeFiles/zephyr.dir/subsys/logging/log_cache.c.obj
[39/295] Building C object zephyr/CMakeFiles/zephyr.dir/lib/heap/heap.c.obj
[40/295] Building C object zephyr/CMakeFiles/zephyr.dir/lib/os/mpsc_pbuf.c.obj
[41/295] Building C object zephyr/CMakeFiles/zephyr.dir/lib/utils/rb.c.obj
[42/295] Building C object zephyr/CMakeFiles/zephyr.dir/subsys/tracing/tracing_none.c.obj
[43/295] Building C object zephyr/CMakeFiles/zephyr.dir/lib/os/cbprintf_packaged.c.obj
[44/295] Building C object zephyr/CMakeFiles/zephyr.dir/lib/os/cbprintf_complete.c.obj
[45/295] Building C object zephyr/CMakeFiles/zephyr.dir/lib/utils/bitarray.c.obj
[46/295] Building C object zephyr/arch/common/CMakeFiles/arch__common.dir/sw_isr_common.c.obj
[47/295] Building C object zephyr/arch/common/CMakeFiles/isr_tables.dir/isr_tables.c.obj
[48/295] Building C object zephyr/CMakeFiles/zephyr.dir/subsys/logging/log_msg.c.obj
[49/295] Linking C static library zephyr/arch/common/libisr_tables.a
[50/295] Building C object zephyr/CMakeFiles/zephyr.dir/subsys/logging/log_mgmt.c.obj
[51/295] Building C object zephyr/arch/common/CMakeFiles/arch__common.dir/dynamic_isr.c.obj
[52/295] Building C object zephyr/CMakeFiles/zephyr.dir/subsys/logging/backends/log_backend_uart.c.obj
[53/295] Building ASM object zephyr/arch/arch/xtensa/core/CMakeFiles/arch__xtensa__core.dir/window_vectors.S.obj
[54/295] Building ASM object zephyr/arch/arch/xtensa/core/CMakeFiles/arch__xtensa__core.dir/xtensa_asm2_util.S.obj
[55/295] Building C object zephyr/arch/arch/xtensa/core/CMakeFiles/arch__xtensa__core.dir/fatal.c.obj
[56/295] Building C object zephyr/arch/arch/xtensa/core/CMakeFiles/arch__xtensa__core.dir/cpu_idle.c.obj
[57/295] Building C object zephyr/CMakeFiles/zephyr.dir/subsys/logging/log_core.c.obj
[58/295] Generating linker_zephyr_pre0.cmd
[59/295] Linking C static library zephyr/arch/common/libarch__common.a
[60/295] Building ASM object zephyr/arch/arch/xtensa/core/startup/CMakeFiles/arch__xtensa__core__startup.dir/memctl_default.S.obj
[61/295] Building C object zephyr/CMakeFiles/zephyr.dir/subsys/logging/log_output.c.obj
[62/295] Building ASM object zephyr/arch/arch/xtensa/core/CMakeFiles/arch__xtensa__core.dir/crt1.S.obj
[63/295] Building ASM object zephyr/arch/arch/xtensa/core/startup/CMakeFiles/arch__xtensa__core__startup.dir/reset_vector.S.obj
[64/295] Building ASM object zephyr/arch/arch/xtensa/core/startup/CMakeFiles/arch__xtensa__core__startup.dir/memerror_vector.S.obj
[65/295] Building C object zephyr/arch/arch/xtensa/core/CMakeFiles/arch__xtensa__core.dir/thread.c.obj
[66/295] Generating ../../../include/generated/libc/minimal/strerror_table.h
[67/295] Linking ASM static library zephyr/arch/arch/xtensa/core/startup/libarch__xtensa__core__startup.a
[68/295] Building C object zephyr/lib/libc/common/CMakeFiles/lib__libc__common.dir/source/string/strnlen.c.obj
[69/295] Building C object zephyr/CMakeFiles/zephyr_pre0.dir/misc/empty_file.c.obj
[70/295] Linking C static library zephyr/libzephyr.a
[71/295] Building C object zephyr/lib/libc/common/CMakeFiles/lib__libc__common.dir/source/stdlib/abort.c.obj
[72/295] Building C object zephyr/lib/libc/minimal/CMakeFiles/lib__libc__minimal.dir/source/stdlib/strtol.c.obj
[73/295] Building C object zephyr/arch/arch/xtensa/core/CMakeFiles/arch__xtensa__core.dir/irq_manage.c.obj
[74/295] Building C object zephyr/lib/libc/minimal/CMakeFiles/lib__libc__minimal.dir/source/stdlib/atoi.c.obj
[75/295] Building C object zephyr/lib/libc/minimal/CMakeFiles/lib__libc__minimal.dir/source/stdlib/strtoll.c.obj
[76/295] Building C object zephyr/lib/libc/minimal/CMakeFiles/lib__libc__minimal.dir/source/stdlib/strtoul.c.obj
[77/295] Building C object zephyr/lib/libc/minimal/CMakeFiles/lib__libc__minimal.dir/source/stdlib/bsearch.c.obj
[78/295] Building C object zephyr/lib/libc/minimal/CMakeFiles/lib__libc__minimal.dir/source/stdlib/strtoull.c.obj
[79/295] Building C object zephyr/lib/libc/common/CMakeFiles/lib__libc__common.dir/source/stdlib/malloc.c.obj
[80/295] Building C object zephyr/lib/libc/minimal/CMakeFiles/lib__libc__minimal.dir/source/string/strstr.c.obj
[81/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/trace/trace.c.obj
[82/295] Building C object zephyr/lib/libc/minimal/CMakeFiles/lib__libc__minimal.dir/source/stdlib/exit.c.obj
[83/295] Building C object zephyr/lib/libc/minimal/CMakeFiles/lib__libc__minimal.dir/source/string/strncasecmp.c.obj
[84/295] Building C object zephyr/lib/libc/minimal/CMakeFiles/lib__libc__minimal.dir/source/string/strspn.c.obj
[85/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/trace/dma-trace.c.obj
[86/295] Building C object zephyr/lib/libc/minimal/CMakeFiles/lib__libc__minimal.dir/source/stdlib/qsort.c.obj
[87/295] Building C object zephyr/lib/libc/minimal/CMakeFiles/lib__libc__minimal.dir/source/string/strerror.c.obj
[88/295] Building C object zephyr/lib/libc/minimal/CMakeFiles/lib__libc__minimal.dir/source/stdout/sprintf.c.obj
[89/295] Building C object zephyr/lib/libc/minimal/CMakeFiles/lib__libc__minimal.dir/source/math/sqrt.c.obj
[90/295] Building C object zephyr/lib/libc/minimal/CMakeFiles/lib__libc__minimal.dir/source/stdout/fprintf.c.obj
[91/295] Building C object zephyr/lib/libc/minimal/CMakeFiles/lib__libc__minimal.dir/source/math/sqrtf.c.obj
[92/295] Linking C static library zephyr/lib/libc/common/liblib__libc__common.a
[93/295] Building C object zephyr/lib/libc/minimal/CMakeFiles/lib__libc__minimal.dir/source/string/string.c.obj
[94/295] Building C object zephyr/lib/libc/minimal/CMakeFiles/lib__libc__minimal.dir/source/stdout/stdout_console.c.obj
[95/295] Building C object zephyr/lib/libc/minimal/CMakeFiles/lib__libc__minimal.dir/source/time/gmtime.c.obj
[96/295] Building C object zephyr/drivers/console/CMakeFiles/drivers__console.dir/uart_console.c.obj
[97/295] Linking C static library zephyr/lib/libc/minimal/liblib__libc__minimal.a
[98/295] Linking C static library zephyr/drivers/console/libdrivers__console.a
[99/295] Building C object zephyr/drivers/serial/CMakeFiles/drivers__serial.dir/uart_mcux_iuart.c.obj
[100/295] Building C object zephyr/drivers/pinctrl/CMakeFiles/drivers__pinctrl.dir/pinctrl_imx.c.obj
[101/295] Linking C static library zephyr/drivers/serial/libdrivers__serial.a
[102/295] Building C object zephyr/drivers/timer/CMakeFiles/drivers__timer.dir/sys_clock_init.c.obj
[103/295] Building C object zephyr/drivers/clock_control/CMakeFiles/drivers__clock_control.dir/clock_control_mcux_ccm.c.obj
[104/295] Building C object zephyr/drivers/pinctrl/CMakeFiles/drivers__pinctrl.dir/common.c.obj
[105/295] Building C object zephyr/arch/arch/xtensa/core/CMakeFiles/arch__xtensa__core.dir/vector_handlers.c.obj
[106/295] Building C object zephyr/drivers/dma/CMakeFiles/drivers__dma.dir/dma_nxp_sdma.c.obj
/home/nxf25322/work/sof_dir/zephyr/drivers/dma/dma_nxp_sdma.c: In function 'sdma_stop':
/home/nxf25322/work/sof_dir/zephyr/drivers/dma/dma_nxp_sdma.c:126:27: warning: unused variable 'dev_data' [-Wunused-variable]
  126 |         struct sdma_data *dev_data = dev->data;
      |                           ^~~~~~~~
/home/nxf25322/work/sof_dir/zephyr/drivers/dma/dma_nxp_sdma.c: In function 'sdma_reload':
/home/nxf25322/work/sof_dir/zephyr/drivers/dma/dma_nxp_sdma.c:149:30: warning: passing argument 2 of 'SDMA_PrepareTransfer' makes integer from pointer without a cast [-Wint-conversion]
  149 |                              (void *)src,
      |                              ^~~~~~~~~~~
      |                              |
      |                              void *
In file included from /home/nxf25322/work/sof_dir/zephyr/drivers/dma/dma_nxp_sdma.h:15,
                 from /home/nxf25322/work/sof_dir/zephyr/drivers/dma/dma_nxp_sdma.c:7:
/home/nxf25322/work/sof_dir/modules/hal/nxp/mcux/mcux-sdk/devices/MIMX8ML8/drivers/fsl_sdma.h:772:36: note: expected 'uint32_t' {aka 'unsigned int'} but argument is of type 'void *'
  772 |                           uint32_t srcAddr,
      |                           ~~~~~~~~~^~~~~~~
/home/nxf25322/work/sof_dir/zephyr/drivers/dma/dma_nxp_sdma.c:150:30: warning: passing argument 3 of 'SDMA_PrepareTransfer' makes integer from pointer without a cast [-Wint-conversion]
  150 |                              (void *)dst,
      |                              ^~~~~~~~~~~
      |                              |
      |                              void *
/home/nxf25322/work/sof_dir/modules/hal/nxp/mcux/mcux-sdk/devices/MIMX8ML8/drivers/fsl_sdma.h:773:36: note: expected 'uint32_t' {aka 'unsigned int'} but argument is of type 'void *'
  773 |                           uint32_t destAddr,
      |                           ~~~~~~~~~^~~~~~~~
/home/nxf25322/work/sof_dir/zephyr/drivers/dma/dma_nxp_sdma.c: In function 'sdma_init':
/home/nxf25322/work/sof_dir/zephyr/drivers/dma/dma_nxp_sdma.c:190:35: warning: unused variable 'cfg' [-Wunused-variable]
  190 |         const struct sdma_config *cfg;
      |                                   ^~~
/home/nxf25322/work/sof_dir/zephyr/drivers/dma/dma_nxp_sdma.c:188:27: warning: unused variable 'data' [-Wunused-variable]
  188 |         struct sdma_data *data;
      |                           ^~~~
/home/nxf25322/work/sof_dir/zephyr/drivers/dma/dma_nxp_sdma.c: At top level:
/home/nxf25322/work/sof_dir/zephyr/drivers/dma/dma_nxp_sdma.c:33:13: warning: 'sdma_isr' defined but not used [-Wunused-function]
   33 | static void sdma_isr(const void *parameter)
      |             ^~~~~~~~
[107/295] Linking C static library zephyr/drivers/pinctrl/libdrivers__pinctrl.a
[108/295] Building C object zephyr/drivers/timer/CMakeFiles/drivers__timer.dir/xtensa_sys_timer.c.obj
[109/295] Linking C static library zephyr/drivers/clock_control/libdrivers__clock_control.a
[110/295] Linking C static library zephyr/arch/arch/xtensa/core/libarch__xtensa__core.a
[111/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/init/ext_manifest.c.obj
[112/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/init/init.c.obj
[113/295] Building C object zephyr/drivers/dma/CMakeFiles/drivers__dma.dir/dma_nxp_sof_host_dma.c.obj
[114/295] Linking C static library zephyr/drivers/timer/libdrivers__timer.a
[115/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/audio/asrc/asrc.c.obj
[116/295] Linking C static library zephyr/drivers/dma/libdrivers__dma.a
[117/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/ipc/dma-copy.c.obj
[118/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/ipc/ipc3/dai.c.obj
[119/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/ipc/ipc3/host-page-table.c.obj
[120/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/ipc/ipc-helper.c.obj
[121/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/ipc/ipc3/helper.c.obj
[122/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/platform/imx8m/platform.c.obj
[123/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/platform/imx8m/lib/clk.c.obj
[124/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/drivers/imx/ipc.c.obj
[125/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/ipc/ipc-common.c.obj
[126/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/platform/imx8m/lib/memory.c.obj
[127/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/math/decibels.c.obj
[128/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/lib/dma.c.obj
[129/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/math/numbers.c.obj
[130/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/math/trig.c.obj
[131/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/math/exp_fcn_hifi.c.obj
[132/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/lib/clk.c.obj
[133/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/math/exp_fcn.c.obj
[134/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/lib/cpu-clk-manager.c.obj
[135/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/lib/pm_runtime.c.obj
[136/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/lib/notifier.c.obj
[137/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/audio/pcm_converter/pcm_converter_hifi3.c.obj
[138/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/schedule/zephyr_ll.c.obj
[139/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/lib/dai.c.obj
[140/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/lib/dma.c.obj
[141/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/audio/channel_map.c.obj
[142/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/audio/sink_source_utils.c.obj
[143/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/audio/sink_api_helper.c.obj
[144/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/audio/pcm_converter/pcm_converter.c.obj
[145/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/audio/buffer.c.obj
[146/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/audio/source_api_helper.c.obj
[147/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/ipc/ipc3/handler.c.obj
[148/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/arch/xtensa/drivers/cache_attr.c.obj
[149/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/audio/audio_stream.c.obj
[150/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/audio/pcm_converter/pcm_converter_generic.c.obj
[151/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/audio/pipeline/pipeline-schedule.c.obj
[152/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/schedule/zephyr_domain.c.obj
[153/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/audio/pipeline/pipeline-params.c.obj
[154/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/schedule/schedule.c.obj
[155/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/audio/pipeline/pipeline-xrun.c.obj
[156/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/idc/zephyr_idc.c.obj
[157/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/lib.c.obj
[158/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/audio/component.c.obj
[159/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/schedule.c.obj
[160/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/audio/pipeline/pipeline-stream.c.obj
[161/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/lib/cpu.c.obj
[162/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/module/audio/source_api.c.obj
[163/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/audio/pipeline/pipeline-graph.c.obj
[164/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/edf_schedule.c.obj
[165/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/audio/eq_fir/eq_fir_hifi3.c.obj
[166/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/module/audio/sink_api.c.obj
[167/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/wrapper.c.obj
[168/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/lib/alloc.c.obj
[169/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/audio/eq_fir/eq_fir_hifi2ep.c.obj
[170/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/math/fir_hifi2ep.c.obj
[171/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/audio/eq_fir/eq_fir_ipc3.c.obj
[172/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/math/fir_hifi3.c.obj
[173/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/math/iir_df1_hifi3.c.obj
[174/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/audio/eq_iir/eq_iir.c.obj
[175/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/math/fir_generic.c.obj
[176/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/audio/eq_fir/eq_fir_generic.c.obj
[177/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/math/iir_df1_generic.c.obj
[178/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/audio/eq_fir/eq_fir.c.obj
[179/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/audio/eq_iir/eq_iir_ipc3.c.obj
[180/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/audio/data_blob.c.obj
[181/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/math/iir_df1.c.obj
[182/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/audio/eq_iir/eq_iir_generic.c.obj
[183/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/audio/asrc/asrc_farrow_hifi3.c.obj
[184/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/audio/asrc/asrc_ipc3.c.obj
[185/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/audio/dcblock/dcblock_hifi3.c.obj
[186/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/audio/dcblock/dcblock_hifi4.c.obj
[187/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/audio/dcblock/dcblock_generic.c.obj
[188/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/audio/dcblock/dcblock_ipc3.c.obj
[189/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/audio/asrc/asrc_farrow_generic.c.obj
[190/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/audio/dcblock/dcblock.c.obj
[191/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/audio/host-zephyr.c.obj
[192/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/audio/mixer/mixer_hifi3.c.obj
[193/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/audio/selector/selector_generic.c.obj
[194/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/audio/asrc/asrc_farrow.c.obj
[195/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/audio/mixer/mixer_generic.c.obj
[196/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/audio/volume/volume_hifi4.c.obj
[197/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/audio/selector/selector.c.obj
[198/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/audio/mixer/mixer.c.obj
[199/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/audio/volume/volume_hifi3.c.obj
[200/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/audio/volume/volume_hifi4_with_peakvol.c.obj
[201/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/audio/volume/volume_generic_with_peakvol.c.obj
[202/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/audio/volume/volume_hifi3_with_peakvol.c.obj
[203/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/audio/src/src_hifi2ep.c.obj
[204/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/audio/volume/volume_generic.c.obj
[205/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/audio/volume/volume_ipc3.c.obj
[206/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/samples/audio/detect_test.c.obj
[207/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/audio/src/src_hifi3.c.obj
[208/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/audio/volume/volume.c.obj
[209/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/audio/src/src_hifi4.c.obj
[210/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/audio/module_adapter/module_adapter_ipc3.c.obj
[211/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/audio/module_adapter/module/generic.c.obj
[212/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/audio/src/src_lite.c.obj
[213/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/audio/src/src_ipc3.c.obj
[214/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/audio/kpb.c.obj
[215/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/audio/src/src_generic.c.obj
[216/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/audio/tdfb/tdfb_ipc3.c.obj
[217/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/audio/src/src.c.obj
[218/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/audio/dai-zephyr.c.obj
[219/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/audio/tdfb/tdfb_hifiep.c.obj
[220/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/math/sqrt_int16.c.obj
[221/295] No install step for 'sof_logger_ep'
[222/295] Completed 'sof_logger_ep'
[223/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/samples/audio/smart_amp_test_ipc3.c.obj
[224/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/audio/tdfb/tdfb_hifi3.c.obj
[225/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/audio/module_adapter/module_adapter.c.obj
[226/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/audio/tdfb/tdfb_direction.c.obj
[227/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/audio/tdfb/tdfb.c.obj
[228/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/audio/mux/mux_ipc3.c.obj
[229/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/audio/tdfb/tdfb_generic.c.obj
[230/295] Building C object modules/xtensa/CMakeFiles/modules_xtensa_hal.dir/src/hal/attribute.c.obj
[231/295] Building ASM object modules/xtensa/CMakeFiles/modules_xtensa_hal.dir/src/hal/cache_asm.S.obj
[232/295] Building C object modules/xtensa/CMakeFiles/modules_xtensa_hal.dir/src/hal/cache.c.obj
[233/295] Building ASM object modules/xtensa/CMakeFiles/modules_xtensa_hal.dir/src/hal/clock.S.obj
[234/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/audio/mux/mux_generic.c.obj
[235/295] Building C object modules/xtensa/CMakeFiles/modules_xtensa_hal.dir/src/hal/debug.c.obj
[236/295] Building C object modules/xtensa/CMakeFiles/modules_xtensa_hal.dir/src/hal/coherence.c.obj
[237/295] Building C object modules/hal_nxp/hal_nxp/CMakeFiles/..__modules__hal__nxp.dir/mcux/mcux-sdk/drivers/common/fsl_common.c.obj
[238/295] Building ASM object modules/xtensa/CMakeFiles/modules_xtensa_hal.dir/src/hal/debug_hndlr.S.obj
[239/295] Building ASM object modules/xtensa/CMakeFiles/modules_xtensa_hal.dir/src/hal/int_asm.S.obj
[240/295] Building C object modules/hal_nxp/hal_nxp/CMakeFiles/..__modules__hal__nxp.dir/mcux/mcux-sdk/drivers/common/fsl_common_dsp.c.obj
[241/295] Building ASM object modules/xtensa/CMakeFiles/modules_xtensa_hal.dir/src/hal/memcopy.S.obj
[242/295] Building ASM object modules/xtensa/CMakeFiles/modules_xtensa_hal.dir/src/hal/mem_ecc_parity.S.obj
[243/295] Building C object modules/xtensa/CMakeFiles/modules_xtensa_hal.dir/src/hal/misc.c.obj
[244/295] Building C object modules/xtensa/CMakeFiles/modules_xtensa_hal.dir/src/hal/disass.c.obj
[245/295] Building C object modules/xtensa/CMakeFiles/modules_xtensa_hal.dir/src/hal/mmu.c.obj
[246/295] Building ASM object modules/xtensa/CMakeFiles/modules_xtensa_hal.dir/src/hal/miscellaneous.S.obj
[247/295] Building C object modules/sof/CMakeFiles/modules_sof.dir/home/nxf25322/work/sof_dir/sof/src/audio/mux/mux.c.obj
[248/295] Building ASM object modules/xtensa/CMakeFiles/modules_xtensa_hal.dir/src/hal/mp_asm.S.obj
[249/295] Building C object modules/xtensa/CMakeFiles/modules_xtensa_hal.dir/src/hal/set_region_translate.c.obj
[250/295] Building C object modules/xtensa/CMakeFiles/modules_xtensa_hal.dir/src/hal/interrupts.c.obj
[251/295] Building ASM object modules/xtensa/CMakeFiles/modules_xtensa_hal.dir/src/hal/state_asm.S.obj
[252/295] Building C object modules/hal_nxp/hal_nxp/CMakeFiles/..__modules__hal__nxp.dir/mcux/mcux-sdk/devices/MIMX8ML8/drivers/fsl_clock.c.obj
[253/295] Building ASM object modules/xtensa/CMakeFiles/modules_xtensa_hal.dir/src/hal/windowspill_asm.S.obj
[254/295] Building ASM object modules/xtensa/CMakeFiles/modules_xtensa_hal.dir/src/hal/syscache_asm.S.obj
[255/295] Building C object modules/xtensa/CMakeFiles/modules_xtensa_hal.dir/src/hal/state.c.obj
[256/295] Building C object zephyr/kernel/CMakeFiles/kernel.dir/banner.c.obj
[257/295] Building C object zephyr/kernel/CMakeFiles/kernel.dir/busy_wait.c.obj
[258/295] Building C object modules/hal_nxp/hal_nxp/CMakeFiles/..__modules__hal__nxp.dir/mcux/mcux-sdk/drivers/sdma/fsl_sdma.c.obj
[259/295] Building C object zephyr/kernel/CMakeFiles/kernel.dir/device.c.obj
[260/295] Building C object zephyr/kernel/CMakeFiles/kernel.dir/main_weak.c.obj
[261/295] Building C object zephyr/kernel/CMakeFiles/kernel.dir/errno.c.obj
[262/295] Linking C static library modules/xtensa/libmodules_xtensa_hal.a
[263/295] Building C object zephyr/kernel/CMakeFiles/kernel.dir/version.c.obj
[264/295] Building C object zephyr/kernel/CMakeFiles/kernel.dir/fatal.c.obj
[265/295] Building C object zephyr/kernel/CMakeFiles/kernel.dir/kheap.c.obj
[266/295] Building C object zephyr/kernel/CMakeFiles/kernel.dir/mem_slab.c.obj
[267/295] Building C object modules/hal_nxp/hal_nxp/CMakeFiles/..__modules__hal__nxp.dir/mcux/mcux-sdk/drivers/iuart/fsl_uart.c.obj
[268/295] Linking C static library modules/hal_nxp/hal_nxp/lib..__modules__hal__nxp.a
[269/295] Building C object zephyr/kernel/CMakeFiles/kernel.dir/msg_q.c.obj
[270/295] Building C object zephyr/kernel/CMakeFiles/kernel.dir/idle.c.obj
[271/295] Building C object zephyr/kernel/CMakeFiles/kernel.dir/init.c.obj
[272/295] Building C object zephyr/kernel/CMakeFiles/kernel.dir/mailbox.c.obj
[273/295] Building C object zephyr/kernel/CMakeFiles/kernel.dir/stack.c.obj
[274/295] Building C object zephyr/kernel/CMakeFiles/kernel.dir/system_work_q.c.obj
[275/295] Building C object zephyr/kernel/CMakeFiles/kernel.dir/queue.c.obj
[276/295] Building C object zephyr/kernel/CMakeFiles/kernel.dir/thread.c.obj
[277/295] Building C object zephyr/kernel/CMakeFiles/kernel.dir/sem.c.obj
[278/295] Building C object zephyr/kernel/CMakeFiles/kernel.dir/condvar.c.obj
[279/295] Linking C static library modules/sof/libmodules_sof.a
[280/295] Building C object zephyr/kernel/CMakeFiles/kernel.dir/mutex.c.obj
[281/295] Building C object zephyr/kernel/CMakeFiles/kernel.dir/dynamic_disabled.c.obj
[282/295] Building C object zephyr/kernel/CMakeFiles/kernel.dir/mempool.c.obj
[283/295] Building C object zephyr/kernel/CMakeFiles/kernel.dir/timer.c.obj
[284/295] Building C object zephyr/kernel/CMakeFiles/kernel.dir/work.c.obj
[285/295] Building C object zephyr/kernel/CMakeFiles/kernel.dir/timeout.c.obj
[286/295] Building C object zephyr/kernel/CMakeFiles/kernel.dir/sched.c.obj
[287/295] Linking C static library zephyr/kernel/libkernel.a
[288/295] Linking C executable zephyr/zephyr_pre0.elf
/home/nxf25322/zephyr-sdk-0.16.4/xtensa-nxp_imx8m_adsp_zephyr-elf/bin/../lib/gcc/xtensa-nxp_imx8m_adsp_zephyr-elf/12.2.0/../../../../xtensa-nxp_imx8m_adsp_zephyr-elf/bin/ld.bfd: warning: orphan section `NonCacheable' from `modules/hal_nxp/hal_nxp/lib..__modules__hal__nxp.a(fsl_sdma.c.obj)' being placed in section `NonCacheable'
[289/295] Generating linker.cmd
[290/295] Generating isr_tables.c, isr_tables_vt.ld, isr_tables_swi.ld
[291/295] Building C object zephyr/CMakeFiles/zephyr_final.dir/misc/empty_file.c.obj
[292/295] Building C object zephyr/CMakeFiles/zephyr_final.dir/isr_tables.c.obj
[293/295] Linking C executable zephyr/zephyr.elf
/home/nxf25322/zephyr-sdk-0.16.4/xtensa-nxp_imx8m_adsp_zephyr-elf/bin/../lib/gcc/xtensa-nxp_imx8m_adsp_zephyr-elf/12.2.0/../../../../xtensa-nxp_imx8m_adsp_zephyr-elf/bin/ld.bfd: warning: orphan section `NonCacheable' from `modules/hal_nxp/hal_nxp/lib..__modules__hal__nxp.a(fsl_sdma.c.obj)' being placed in section `NonCacheable'
Memory region         Used Size  Region Size  %age Used
vector_reset_text:         289 B        736 B     39.27%
vector_reset_lit:          0 GB        288 B      0.00%
vector_base_text:         362 B        376 B     96.28%
 vector_int2_lit:          0 GB          4 B      0.00%
vector_int2_text:           3 B         28 B     10.71%
 vector_int3_lit:          0 GB          4 B      0.00%
vector_int3_text:           3 B         28 B     10.71%
 vector_int4_lit:          0 GB          4 B      0.00%
vector_int4_text:           3 B         28 B     10.71%
 vector_int5_lit:          0 GB          4 B      0.00%
vector_int5_text:           3 B         28 B     10.71%
vector_kernel_lit:          0 GB          4 B      0.00%
vector_kernel_text:           3 B         28 B     10.71%
 vector_user_lit:          0 GB          4 B      0.00%
vector_user_text:          22 B         28 B     78.57%
vector_double_lit:          0 GB          4 B      0.00%
vector_double_text:           6 B         28 B     21.43%
 iram_text_start:         192 B        424 B     45.28%
          sdram0:      270744 B         8 MB      3.23%
          sdram1:       8144 KB      8172 KB     99.66%
        IDT_LIST:          0 GB         8 KB      0.00%
static_uuid_entries_seg:        1632 B        24 KB      6.64%
static_log_entries_seg:       74064 B        32 MB      0.22%
 fw_metadata_seg:         720 B        32 MB      0.00%
Generating files from /home/nxf25322/work/sof_dir/build-imx8m/zephyr/zephyr.elf for board: nxp_adsp_imx8m
[294/295] cd /home/nxf25322/work/sof_dir/build-imx8m/zephyr && /usr/bin/python3 /home/nxf25322/work/sof_dir/zephyr/scripts/build/check_init_priorities.py --elf-file=/home/nxf25322/work/sof_dir/build-imx8m/zephyr/zephyr.elf
[295/295] west sign --if-tool-available --tool rimage ...
Signing with tool /home/nxf25322/work/sof_dir/build-rimage/rimage
Signing for SOC target imx8m
/home/nxf25322/work/sof_dir/build-rimage/rimage -o /home/nxf25322/work/sof_dir/build-imx8m/zephyr/zephyr.ri -k 'key param ignored by imx8m' -f 2.8.99 -b 1 -e -c /home/nxf25322/work/sof_dir/sof/tools/rimage/config/imx8m.toml /home/nxf25322/work/sof_dir/build-imx8m/zephyr/zephyr.elf
error: 'alias_mask' not found

Module Reading /home/nxf25322/work/sof_dir/build-imx8m/zephyr/zephyr.elf
  Found 47 sections, listing valid sections...
	No	LMA		VMA		End		Size	Type	Name
	1	0x3b6f8000	0x3b6f8000	0x3b6f8121	0x121	TEXT	.ResetVector.text
	2	0x3b6f8400	0x3b6f8400	0x3b6f856a	0x16a	TEXT	.WindowVectors.text
	3	0x3b6f857c	0x3b6f857c	0x3b6f857f	0x3	TEXT	.Level2InterruptVector.text
	4	0x3b6f859c	0x3b6f859c	0x3b6f859f	0x3	TEXT	.Level3InterruptVector.text
	5	0x3b6f85bc	0x3b6f85bc	0x3b6f85bf	0x3	TEXT	.DebugExceptionVector.text
	6	0x3b6f85dc	0x3b6f85dc	0x3b6f85df	0x3	TEXT	.NMIExceptionVector.text
	7	0x3b6f85fc	0x3b6f85fc	0x3b6f85ff	0x3	TEXT	.KernelExceptionVector.text
	8	0x3b6f861c	0x3b6f861c	0x3b6f8632	0x16	TEXT	.UserExceptionVector.text
	9	0x3b6f863c	0x3b6f863c	0x3b6f8642	0x6	TEXT	.DoubleExceptionVector.text
	10	0x3b6f8658	0x3b6f8658	0x3b6f8718	0xc0	TEXT	.iram.text
	11	0x92400000	0x92400000	0x9240f998	0xf998	DATA	.rodata
	12	0x9240f998	0x9240f998	0x92435fd6	0x2663e	TEXT	.text
	13	0x92435fd8	0x92435fd8	0x924360c8	0xf0	DATA	initlevel
	14	0x924360c8	0x924360c8	0x92436118	0x50	DATA	device_area
	15	0x92436118	0x92436118	0x92436290	0x178	DATA	log_const_area
	16	0x92436290	0x92436290	0x924362a0	0x10	DATA	log_backend_area
	17	0x924362a0	0x924362a0	0x9243630c	0x6c	DATA	.fw_ready
	18	0x92436310	0x92436310	0x9243d710	0x7400	BSS	.noinit
	19	0x9243d710	0x9243d710	0x9243f000	0x18f0	DATA	.data
	20	0x9243f000	0x9243f000	0x9243fa80	0xa80	DATA	NonCacheable
	21	0x9243fa80	0x9243fa80	0x9243fb80	0x100	DATA	sw_isr_table
	22	0x9243fb80	0x9243fb80	0x9243fb88	0x8	DATA	device_states
	23	0x9243fb88	0x9243fb88	0x9243fbc0	0x38	DATA	log_mpsc_pbuf_area
	24	0x9243fbc0	0x9243fbc0	0x9243fbc4	0x4	DATA	log_msg_ptr_area
	25	0x9243fbc4	0x9243fbc4	0x9243fbd8	0x14	DATA	k_heap_area
	26	0x9243fbd8	0x9243fbd8	0x9243fbe8	0x10	DATA	k_sem_area
	27	0x9243fc00	0x9243fc00	0x92442198	0x2598	BSS	.bss
	28	0x92c05000	0x92c05000	0x933f9000	0x7f4000	BSS	.heap_mem
 module: input size 232280 (0x38b58) bytes 28 sections
 module: text 158132 (0x269b4) bytes
	data 74148 (0x121a4) bytes
	bss  8378776 (0x7fd998) bytes

writing module 0 /home/nxf25322/work/sof_dir/build-imx8m/zephyr/zephyr.elf

	Totals	Start		End		Size
	TEXT	0x3b6f8000	0x92435fd6	0x56d3e000
	DATA	0x92400000	0x9243fbe8	0x40000
	BSS	0x92436310	0x933f9000	0xfc3000

	No	Address		Size		File		Type	Name
	0	0x3b6f8000	0x00000121	0x0000014c	TEXT	.ResetVector.text
	1	0x3b6f8400	0x0000016a	0x000002c4	TEXT	.WindowVectors.text
	2	0x3b6f857c	0x00000003	0x000002d4	TEXT	.Level2InterruptVector.text
	3	0x3b6f859c	0x00000003	0x000002e4	TEXT	.Level3InterruptVector.text
	4	0x3b6f85bc	0x00000003	0x000002f4	TEXT	.DebugExceptionVector.text
	5	0x3b6f85dc	0x00000003	0x00000304	TEXT	.NMIExceptionVector.text
	6	0x3b6f85fc	0x00000003	0x00000314	TEXT	.KernelExceptionVector.text
	7	0x3b6f861c	0x00000016	0x00000338	TEXT	.UserExceptionVector.text
	8	0x3b6f863c	0x00000006	0x0000034c	TEXT	.DoubleExceptionVector.text
	9	0x3b6f8658	0x000000c0	0x00000418	TEXT	.iram.text
	10	0x9240f998	0x0002663e	0x00026a64	DATA	.text
	11	0x92400000	0x0000f998	0x00036408	DATA	.rodata
	12	0x92435fd8	0x000000f0	0x00036504	DATA	initlevel
	13	0x924360c8	0x00000050	0x00036560	DATA	device_area
	14	0x92436118	0x00000178	0x000366e4	DATA	log_const_area
	15	0x92436290	0x00000010	0x00036700	DATA	log_backend_area
	16	0x924362a0	0x0000006c	0x00036778	DATA	.fw_ready
	17	0x9243d710	0x000018f0	0x00038074	DATA	.data
	18	0x9243f000	0x00000a80	0x00038b00	DATA	NonCacheable
	19	0x9243fa80	0x00000100	0x00038c0c	DATA	sw_isr_table
	20	0x9243fb80	0x00000008	0x00038c20	DATA	device_states
	21	0x9243fb88	0x00000038	0x00038c64	DATA	log_mpsc_pbuf_area
	22	0x9243fbc0	0x00000004	0x00038c74	DATA	log_msg_ptr_area
	23	0x9243fbc4	0x00000014	0x00038c94	DATA	k_heap_area
	24	0x9243fbd8	0x00000010	0x00038cb0	DATA	k_sem_area

firmware: image size 232624 (0x38cb0) bytes 1 modules

Extended manifest found module, type: 0x0005 size: 0x0020 (  32) offset: 0x0000
Extended manifest found module, type: 0x0004 size: 0x0020 (  32) offset: 0x0020
Extended manifest found module, type: 0x0003 size: 0x0030 (  48) offset: 0x0040
Extended manifest found module, type: 0x0002 size: 0x0070 ( 112) offset: 0x0070
Extended manifest found module, type: 0x0000 size: 0x0050 (  80) offset: 0x00E0
Extended manifest found module, type: 0x0001 size: 0x01A0 ( 416) offset: 0x0130
Extended manifest saved to file /home/nxf25322/work/sof_dir/build-imx8m/zephyr/zephyr.ri.xman size 0x02E0 (736) bytes

Prefixing /home/nxf25322/work/sof_dir/build-imx8m/zephyr/zephyr.ri with manifest /home/nxf25322/work/sof_dir/build-imx8m/zephyr/zephyr.ri.xman
in current dir: /home/nxf25322/work/sof_dir/sof; running command:
    /home/nxf25322/work/sof_dir/build-imx8m/zephyr/smex_ep/build/smex -l /home/nxf25322/work/sof_dir/build-sof-staging/sof/sof-imx8m.ldc /home/nxf25322/work/sof_dir/build-imx8m/zephyr/zephyr.elf
  Found 47 sections, listing valid sections......
	No	Start		End		Size	Type	Name
	1	0x3b6f8000	0x3b6f8121	0x121	TEXT	.ResetVector.text
	2	0x3b6f8400	0x3b6f856a	0x16a	TEXT	.WindowVectors.text
	3	0x3b6f857c	0x3b6f857f	0x3	TEXT	.Level2InterruptVector.text
	4	0x3b6f859c	0x3b6f859f	0x3	TEXT	.Level3InterruptVector.text
	5	0x3b6f85bc	0x3b6f85bf	0x3	TEXT	.DebugExceptionVector.text
	6	0x3b6f85dc	0x3b6f85df	0x3	TEXT	.NMIExceptionVector.text
	7	0x3b6f85fc	0x3b6f85ff	0x3	TEXT	.KernelExceptionVector.text
	8	0x3b6f861c	0x3b6f8632	0x16	TEXT	.UserExceptionVector.text
	9	0x3b6f863c	0x3b6f8642	0x6	TEXT	.DoubleExceptionVector.text
	10	0x3b6f8658	0x3b6f8718	0xc0	TEXT	.iram.text
	11	0x92400000	0x9240f998	0xf998	DATA	.rodata
	12	0x9240f998	0x92435fd6	0x2663e	TEXT	.text
	13	0x92435fd8	0x924360c8	0xf0	DATA	initlevel
	14	0x924360c8	0x92436118	0x50	DATA	device_area
	15	0x92436118	0x92436290	0x178	DATA	log_const_area
	16	0x92436290	0x924362a0	0x10	DATA	log_backend_area
	17	0x924362a0	0x9243630c	0x6c	DATA	.fw_ready
	18	0x92436310	0x9243d710	0x7400	HEAP	.noinit
	19	0x9243d710	0x9243f000	0x18f0	DATA	.data
	20	0x9243f000	0x9243fa80	0xa80	DATA	NonCacheable
	21	0x9243fa80	0x9243fb80	0x100	DATA	sw_isr_table
	22	0x9243fb80	0x9243fb88	0x8	DATA	device_states
	23	0x9243fb88	0x9243fbc0	0x38	DATA	log_mpsc_pbuf_area
	24	0x9243fbc0	0x9243fbc4	0x4	DATA	log_msg_ptr_area
	25	0x9243fbc4	0x9243fbd8	0x14	DATA	k_heap_area
	26	0x9243fbd8	0x9243fbe8	0x10	DATA	k_sem_area
	27	0x9243fc00	0x92442198	0x2598	BSS	.bss
	28	0x92c05000	0x933f9000	0x7f4000	HEAP	.heap_mem
	41	0x1fffa000	0x1fffa660	0x660	NOTE	.static_uuid_entries
	42	0x20000000	0x20012150	0x12150	NOTE	.static_log_entries
	43	0x22000000	0x220002d0	0x2d0	NOTE	.fw_metadata

 module: input size 232280 (0x38b58) bytes 28 sections
 module: text 158132 (0x269b4) bytes
    data 74148 (0x121a4) bytes
    bss  8378776 (0x7fd998) bytes

fw abi main version:	3.29.0
fw abi dbg version:	5.3.0
logs dictionary size:	74140
including fw version of size:	60
uids dictionary size:	1648
in current dir: /home/nxf25322/work/sof_dir/sof; running command:
    /home/nxf25322/zephyr-sdk-0.16.4/xtensa-nxp_imx8m_adsp_zephyr-elf/bin/xtensa-nxp_imx8m_adsp_zephyr-elf-objcopy --remove-section .comment /home/nxf25322/work/sof_dir/build-imx8m/zephyr/zephyr.strip /home/nxf25322/work/sof_dir/build-sof-staging/sof-info/imx8m/stripped-zephyr.elf
build-sof-staging  
|-- sof  
|   |-- community  
|   |   +-- sof-imx8m.ri  	sha256=8fd384d1dd48df2221756d5a1144b113a618653a455489c8ec9c08f23ced6109
|   +-- sof-imx8m.ldc  	sha256=e9802bc567e8f7d8ea1e2f5a6279ce77f78093aed54fb2bbd893f574a063e7cd
|-- sof-info  
|   +-- imx8m  
|       |-- config.gz  
|       |-- generated_configs.c.gz  	sha256=199215537e1bfe77248b16c011135731258b34b11fafdc41300c6261ebfc79f4
|       |-- sof_versions.h  	sha256=1713def60c6cf5138e958ddad01df272f72eeeb2c296274aab3f0ed2e67999e3
|       |-- stripped-zephyr.elf.gz  	sha256=476481c25cd80a5432406b6eca2cb11fdf951559fee87a75cc0689d4e9940700
|       |-- westconfig.ini  
|       |-- zephyr.elf.gz  
|       |-- zephyr.map.gz  
|       +-- zephyr_version.h  	sha256=bb78b39cb8e7947d8d29307c5ecc6b2a5e335e3f14beaf0d8d440f50f1e8cf07
+-- tools  
    |-- acetool.py  
    |-- cavstool.py  
    |-- cavstool_client.py  
    |-- cavstwist.sh  
    |-- mtrace-reader.py  
    |-- remote-fw-service.py  
    +-- sof-logger  
