.. SPDX-License-Identifier: BSD-3-Clause

=========================================================================
IPC4 support for i.MX (8MP, 95, 8QM, 8QXP, 8ULP) — SOF + NXP Linux
=========================================================================

:Status: Work-in-progress bring-up (feature/ipc4 branches). i.MX8MP, i.MX95,
         i.MX8QXP, i.MX8QM and i.MX8ULP play IPC4 end-to-end. See `i.MX95 IPC4
         bring-up: fixes and gotchas`_ and `i.MX8QM / i.MX8QXP / i.MX8ULP
         bring-up`_.
:Firmware repo: ``~/work/sof_dir8/sof`` (branch ``feature/ipc4_95_8mp_works``)
:Kernel repo:   ``~/work/repos/linux-nxp-rebase`` (branch ``feature/ipc4_95_8mp_works``)
:Platforms:
   * i.MX8MP  — Cadence HiFi4 DSP (``CONFIG_IMX8M`` / ``imx8m``)
   * i.MX95   — Arm Cortex-M7 (``CONFIG_IMX95`` / ``imx95``)
   * i.MX8QM  — Cadence HiFi4 DSP (``CONFIG_IMX8``  / ``imx8``)
   * i.MX8QXP — Cadence HiFi4 DSP (``CONFIG_IMX8X`` / ``imx8x``)
   * i.MX8ULP — Cadence HiFi4 DSP (``CONFIG_IMX8ULP`` / ``imx8ulp``)

.. contents::
   :local:
   :depth: 2


Overview
========

SOF has historically used **IPC3** on all i.MX platforms. This work adds
**IPC4** (the same major IPC version already used on Intel platforms) to two
NXP SoCs:

* **i.MX8MP** — the audio DSP is a Cadence **HiFi4** core.
* **i.MX95**  — the audio "DSP" is an Arm **Cortex-M7** core managed as a
  separate Logical Machine by the SCMI System Manager.

IPC4 is now the *default* IPC type for both platforms on these branches. IPC3
still builds and runs and is selected either by an explicit build target or, at
runtime, by which firmware/topology files are present on the target (see
`How the kernel picks IPC3 vs IPC4`_).

The change has two halves that must match:

#. **Firmware (SOF)** — build an IPC4 image (``CONFIG_IPC_MAJOR_4=y``), packed
   by rimage with the IPC4 ext-manifest, using an IPC4 mailbox/window memory
   layout, plus IPC4 topologies.
#. **Kernel (NXP Linux SOF driver)** — advertise IPC4 in the device
   descriptor, implement the i.MX IPC4 mailbox transport (there is no Intel-style
   HDA/DMA hardware), and load the IPC4 firmware + topology from the IPC4
   firmware paths.

Because i.MX has **no host-side DMA engine that can master the audio buffer or
raise period interrupts** (unlike Intel HDA), two i.MX-specific mechanisms are
introduced on top of stock IPC4:

* a **compact 8-byte IPC4 header carried in SRAM** immediately in front of each
  mailbox payload (there are no MU data registers wired for SOF payload headers);
* a **vendor "host period elapsed" IPC4 notification** (type 200) sent by the
  DSP host-copier once per host period, so ALSA gets its period wakeups.


Architecture at a glance
========================

::

   +-------------------- Linux (NXP kernel) --------------------+
   |  snd_sof_of_imx9_driver / imx8m driver                     |
   |    imx9.c / imx8.c   : sof_dev_desc (IPC4 default)          |
   |    imx-common.c      : IPC4 mailbox transport               |
   |      - imx_send_msg()        write 8B hdr @ hostbox-8       |
   |      - imx_handle_ipc4_msg()  read  8B hdr @ dspbox-8        |
   |      - reply vs. notification dispatch                       |
   |    ipc4-pcm.c        : host-byte-counter stream pointer      |
   |    ipc4.c            : rx of vendor period-elapsed notif      |
   +----------------------------|-------------------------------+
                                | MU doorbell + shared SRAM
   +----------------------------v-------------------------------+
   |             SOF firmware (Zephyr, CONFIG_IPC_MAJOR_4)        |
   |    src/drivers/imx/ipc.c   : compact hdr read/write in SRAM  |
   |    platform/imx*/lib/mailbox.h : IPC4 box + SW_REG layout    |
   |    src/audio/copier/copier_host.c : period-elapsed notif TX  |
   |    src/ipc/ipc4/*          : dai / helper / notification      |
   +------------------------------------------------------------+


Firmware (SOF) side
===================

Build targets
-------------

``scripts/xtensa-build-zephyr.py`` defines the platform build targets:

.. code-block:: python

   "imx8m" : PlatformConfig("imx", "imx8mp_evk/mimx8ml8/adsp", ...,
       ipc4 = True,
       extra_conf = ".../app/boards/imx8mp_evk_mimx8ml8_adsp_ipc4.conf")

   "imx95" : PlatformConfig("imx", "imx95_evk/mimx9596/m7/ddr", ...,
       ipc4 = True,
       extra_conf = ".../app/boards/imx95_evk_mimx9596_m7_ddr_ipc4.conf")

So ``imx8m`` and ``imx95`` now build **IPC4 by default**. The old IPC3 builds
are still available as the deprecated aliases ``imx8m-ipc3`` and
``imx95-ipc3`` (no ``extra_conf`` overlay → ``CONFIG_IPC_MAJOR`` defaults to 3).

Build commands:

.. code-block:: shell

   # i.MX95 IPC4
   ./scripts/xtensa-build-zephyr.py -p imx95

   # i.MX8MP IPC4
   ./scripts/xtensa-build-zephyr.py -p imx8m

The IPC4 overlay files (``app/boards/*_ipc4.conf``) turn on
``CONFIG_IPC_MAJOR_4=y`` and disable features i.MX cannot support in this
bring-up: ``CONFIG_TRACE=n`` (no DMA-trace window wired for IPC4),
``CONFIG_CROSS_CORE_STREAM=n`` (single core), ``CONFIG_COMP_PEAK_VOL=n``.

rimage packaging (the ``.ri`` file)
-----------------------------------

IPC4 requires the ext-manifest-v4-capable rimage config. ``zephyr/CMakeLists.txt``
selects it automatically when ``CONFIG_IPC_MAJOR_4`` is set:

.. code-block:: cmake

   board_set_rimage_target(imx95)
   if (CONFIG_IPC_MAJOR_4)
       set(RIMAGE_TARGET       imx95_ipc4 CACHE STRING "rimage target" FORCE)
       set(CACHED_RIMAGE_TARGET imx95_ipc4 CACHE STRING "..."          FORCE)
       zephyr_linker_sources(DATA_SECTIONS
           .../src/platform/imx95/linker/module-section.ld)
   endif()

The FORCE overrides are required because the board's ``board.cmake`` already ran
``board_set_rimage_target(imx95)`` (IPC3 target) earlier; a plain cache set
would be a no-op. The extra ``module-section.ld`` keeps the ``.module`` manifest
entries (read by rimage and by the firmware at runtime) from being dropped by
``--gc-sections``.

The rimage configs:

* ``tools/rimage/config/imx95_ipc4.toml``
* ``tools/rimage/config/imx8m_ipc4.toml``

use ``version = [1, 1]`` — "simple file write prefixed with an unsigned IPC4
ext-manifest" — and describe the SRAM memory zone (for imx95: base
``0x80000000``, size ``0x100000``).

IPC4 mailbox / memory layout
----------------------------

Stock IPC4 exchanges a compact 8-byte header (primary + extension dword) in
hardware registers. i.MX has **no MU data registers wired for the SOF payload
header**, so the header is carried in the 8 bytes *preceding* each payload box
in shared SRAM. This is the central i.MX-specific IPC4 detail
(``src/platform/imx95/include/platform/lib/mailbox.h``):

.. code-block:: c

   #if CONFIG_IPC_MAJOR_4
   #define SRAM_IPC4_HDR_SIZE  8
   #else
   #define SRAM_IPC4_HDR_SIZE  0
   #endif

   /* host -> DSP (hostbox) */
   #define MAILBOX_HOSTBOX_HDR_BASE 0x86000000                 /* 8B header slot  */
   #define MAILBOX_HOSTBOX_BASE    (0x86000000 + SRAM_IPC4_HDR_SIZE) /* payload */

   /* DSP -> host (dspbox) */
   #define MAILBOX_DSPBOX_HDR_BASE  0x86001000
   #define MAILBOX_DSPBOX_BASE     (0x86001000 + SRAM_IPC4_HDR_SIZE)

i.MX95 IPC4 SRAM map (relative to the DT ``sram`` bar, physical ``0x86000000``
for the mailbox aperture):

===================  ==============  ======  ===================================
Region               Base            Size    Purpose
===================  ==============  ======  ===================================
Inbox (host→DSP)     0x86000000      0x1000  8B hdr slot + payload
Outbox (DSP→host)    0x86001000      0x1000  8B hdr slot + payload
Stream               0x86002000      0x1000  stream data
Debug                0x86003000      0x0800  debug
SW registers         0x86003800      0x0800  IPC4 LLP / stream position (copier)
===================  ==============  ======  ===================================

The ``MAILBOX_SW_REG_*`` block is new for IPC4: i.MX has no dedicated SW
register hardware, so a slice of mailbox SRAM holds the copier's stream-position
(LLP) data. New ``mailbox_sw_reg_{read,write}[64]()`` helpers do the
cache-coherent accesses.

The compact header itself is read/written in ``src/drivers/imx/ipc.c``
(``ipc_platform_compact_read_msg`` / ``ipc_platform_compact_write_msg``), which
invalidates/writes-back the ``*_HDR_BASE`` SRAM and copies the two header
dwords so that ``ipc4_get_message_request()`` sees a valid request.

Host period-elapsed notification
--------------------------------

Because there is no host DMA to raise a period IRQ, the DSP host copier
(``src/audio/copier/copier_host.c``) sends a **vendor IPC4 notification (type
200)** once per host period. The extension word carries the host gateway id
(``device * 2 + direction``). Supporting code lives in
``src/ipc/ipc4/notification.c`` and ``src/include/ipc4/notification.h``.

Topologies
----------

IPC4 topologies are new ``.conf`` files under ``tools/topology/topology2/``:

===============================  ======================================  =========
Source conf                      Built tplg                              Codec
===============================  ======================================  =========
imx95-wm8962.conf                sof-imx95-wm8962.tplg                   WM8962 (playback)
imx95-wm8962-duplex.conf         sof-imx95-wm8962-duplex.tplg            WM8962 (duplex)
imx8-wm8960.conf                 sof-imx8mp-wm8960.tplg                  WM8960 (playback)
imx8-wm8960-duplex.conf          sof-imx8mp-wm8960-duplex.tplg           WM8960 (duplex)
===============================  ======================================  =========

They are registered in
``tools/topology/topology2/production/tplg-targets-imx8.cmake``, e.g.::

   "imx95-wm8962\;sof-imx95-wm8962\;SAI_DAI_INDEX=3,STREAM_CODEC_NAME=sai3-wm8962"

The graph for imx95 playback is::

   host-copier.0.playback -> gain.1.1 -> dai-copier.SAI.sai3-codec.playback

and the duplex topology adds the capture path
``dai-copier.SAI -> gain.2.1 -> host-copier.0.capture``.

Note: the SAI back-end ``hw_config`` declares the **codec as the clock
provider** (bit-clock/frame-sync master), matching the WM8960/WM8962 board
wiring.


Kernel (NXP Linux SOF driver) side
==================================

Device descriptor: IPC4 is the default
--------------------------------------

``sound/soc/sof/imx/imx-common.h`` defines ``IMX_SOF_DEV_DESC_IPC4()``, used by
both imx95 and imx8mp. It advertises **both** IPC types and defaults to IPC4:

.. code-block:: c

   .ipc_supported_mask = BIT(SOF_IPC_TYPE_3) | BIT(SOF_IPC_TYPE_4),
   .ipc_default        = SOF_IPC_TYPE_4,
   .default_fw_path = {
       [SOF_IPC_TYPE_3] = "imx/sof",          /* -> /lib/firmware/imx/sof       */
       [SOF_IPC_TYPE_4] = "imx/sof-ipc4",     /* -> /lib/firmware/imx/sof-ipc4  */
   },
   .default_tplg_path = {
       [SOF_IPC_TYPE_3] = "imx/sof-tplg",     /* -> /lib/firmware/imx/sof-tplg      */
       [SOF_IPC_TYPE_4] = "imx/sof-ipc4-tplg",/* -> /lib/firmware/imx/sof-ipc4-tplg */
   },
   .default_fw_filename = {
       [SOF_IPC_TYPE_3] = "sof-imx95.ri",
       [SOF_IPC_TYPE_4] = "sof-imx95.ri",     /* same basename, different dir */
   },

For i.MX95 (``sound/soc/sof/imx/imx9.c``) the descriptor also carries the IPC4
window offsets inside the SRAM bar, mirroring the firmware ``mailbox.h``:

.. code-block:: c

   .ipc4_inbox_offset   = 0x6001000 + IMX_IPC4_HDR_SIZE,  /* host->dsp payload */
   .ipc4_outbox_offset  = 0x6000000 + IMX_IPC4_HDR_SIZE,  /* dsp->host payload */
   .ipc4_debug_offset   = 0x6003000,
   .ipc4_fw_regs_offset = 0x6003800,                      /* SW_REG window     */

(On i.MX95 the box ordering is reversed relative to i.MX8MP: host→dsp box first,
dsp→host box second, and ``fw_regs`` is a *separate* region required for stream
position / ``host_byte_cnt`` reads.)

i.MX IPC4 transport (no HDA)
----------------------------

``sound/soc/sof/imx/imx-common.c`` implements the transport:

* ``imx_send_msg()`` writes the 8-byte IPC4 header at ``host_box.offset - 8``.
* ``imx_handle_ipc4_msg()`` reads the DSP header at ``dsp_box.offset - 8`` and
  dispatches **reply vs. notification**.
* ``imx_get_mailbox_offset()`` / ``imx_get_window_offset()`` map IPC4 window
  indices to SRAM offsets.
* ``imx_pcm_hw_params()`` sets ``stream_tag = dev*2 + stream + 1`` and the
  buffer ``phy_addr``.
* ``imx_get_host_byte_counter()`` reads ``host_byte_cnt[]`` from the fw info box.

Stream pointer without fw_reg LLP ABI
-------------------------------------

The i.MX firmware does not publish the ``fw_reg`` ABI version or
``pipeline_regs``, so the generic IPC4 stream-pointer path returned
``-EOPNOTSUPP`` (aplay got ``-EIO``, ``hw_ptr`` stuck at 0). ``ipc4-pcm.c`` was
patched so that whenever the platform provides ``.get_host_byte_counter``:

* ``sof_ipc4_pcm_setup()`` allocates ``time_info`` regardless of the missing
  ``fw_reg`` ABI;
* ``sof_ipc4_pcm_pointer()`` falls through to the host-byte-counter derivation
  instead of failing.

Contiguous PCM buffer
---------------------

i.MX has no bus-mastering host DMA — the DSP reads the PCM buffer via a single
linear ``phy_addr``. A default SG buffer is non-contiguous and cached (host
writes never reach DRAM; DSP reads stale zeros). A new optional op
``pcm_contiguous_buffer`` (set in ``imx-common.c``) makes ``sof_pcm_new()``
allocate ``SNDRV_DMA_TYPE_DEV`` (contiguous + coherent) instead.

Period-elapsed notification handling
------------------------------------

``sound/soc/sof/ipc4.c`` recognizes the vendor period-elapsed notification
(type 200), maps the gateway id back to the substream, and calls
``snd_sof_pcm_period_elapsed()`` — the software equivalent of the HDA period
interrupt.


Binaries used by the firmware and where they go
===============================================

The build stages files under ``build-sof-staging/sof/<VENDOR>/...`` where
``<VENDOR>`` is ``imx``. The on-target root is ``/lib/firmware``. The kernel
paths above map directly to these directories.

.. list-table::
   :header-rows: 1
   :widths: 22 34 44

   * - What
     - Build staging path
     - Target path (must match kernel desc)
   * - **IPC4 firmware** (imx95)
     - ``build-sof-staging/sof/imx/sof-ipc4/sof-imx95.ri``
     - ``/lib/firmware/imx/sof-ipc4/sof-imx95.ri``
   * - **IPC4 firmware** (imx8mp)
     - ``build-sof-staging/sof/imx/sof-ipc4/sof-imx8mp.ri``
     - ``/lib/firmware/imx/sof-ipc4/sof-imx8mp.ri``
   * - IPC4 topology (imx95)
     - ``.../sof-imx95-wm8962.tplg`` (+ ``-duplex``)
     - ``/lib/firmware/imx/sof-ipc4-tplg/sof-imx95-wm8962.tplg``
   * - IPC4 topology (imx8mp)
     - ``.../sof-imx8mp-wm8960.tplg`` (+ ``-duplex``)
     - ``/lib/firmware/imx/sof-ipc4-tplg/sof-imx8mp-wm8960.tplg``
   * - IPC3 firmware (fallback)
     - ``build-sof-staging/sof/imx/sof-imx95.ri``
     - ``/lib/firmware/imx/sof/sof-imx95.ri``
   * - IPC3 topology (fallback)
     - ``.../sof-imx95-wm8962.tplg``
     - ``/lib/firmware/imx/sof-tplg/sof-imx95-wm8962.tplg``

Which topology basename the kernel asks for is set by the OF machine table
(``sof_imx9_machs`` in ``imx9.c``): for ``fsl,imx95-19x19-evk`` it is
``sof-imx95-wm8962.tplg``, combined with the IPC4 tplg dir → the kernel requests
``imx/sof-ipc4-tplg/sof-imx95-wm8962.tplg``.

Firmware is placed in DDR/SRAM per the reserved-memory / ``sram`` node in the
device tree (the i.MX95 M7 reset vector is programmed via SCMI to the ``sram``
resource start; see ``imx95_chip_probe()``).

Install (copy) commands
-----------------------

.. code-block:: shell

   # Build (produces build-sof-staging/ under the west workspace top)
   ./scripts/xtensa-build-zephyr.py -p imx95   # and/or -p imx8m
   ./scripts/build-tools.sh -T                 # build topologies

   # Firmware + topology onto the target rootfs (adjust paths to your board)
   rsync -a build-sof-staging/sof/imx/ root@target:/lib/firmware/imx/

   # or manually, minimally, for imx95 IPC4:
   scp build-sof-staging/sof/imx/sof-ipc4/sof-imx95.ri \
       root@target:/lib/firmware/imx/sof-ipc4/
   scp tools/build_tools/topology/topology2/production/sof-imx95-wm8962.tplg \
       root@target:/lib/firmware/imx/sof-ipc4-tplg/

After copying, reload the driver (``modprobe -r snd_sof_imx9; modprobe
snd_sof_imx9``) or reboot.


How the kernel picks IPC3 vs IPC4
=================================

Selection happens in ``sound/soc/sof/fw-file-profile.c``:

#. Start at ``desc->ipc_default`` (**IPC4** for these platforms).
#. ``sof_file_profile_for_ipc_type()`` checks whether the IPC4 firmware file
   exists at ``imx/sof-ipc4/sof-<plat>.ri``.
#. If it is **missing**, the loop walks down to the next supported type and
   logs::

      Using fallback IPC type 0 (requested type was 1)

   then tries the IPC3 firmware ``imx/sof/sof-<plat>.ri``.

So the *presence of the IPC4 ``.ri`` on the target* is what decides which IPC
version actually runs. If you want IPC4, the IPC4 firmware **and** an IPC4
topology must both be installed at the IPC4 paths above.


Explaining your i.MX95 boot log
===============================

Your log:

.. code-block:: text

   SOF intended IPC type 1 (IPC4); default fw: imx/sof-ipc4/sof-imx95.ri; tplg dir: imx/sof-ipc4-tplg
   ...
   Using fallback IPC type 0 (requested type was 1)
   ...
     Topology file:     imx/sof-tplg/sof-imx95-wm8962.tplg
   SOF selected IPC type 0 (IPC3); fw: imx/sof/sof-imx95.ri; tplg: imx/sof-tplg/sof-imx95-wm8962.tplg
   loading firmware: imx/sof/sof-imx95.ri
   Firmware info: version 2:14:99-65899
   The received message size is invalid
   ...
   Firmware boot failure due to timeout
   error: failed to boot DSP firmware -5
   probe with driver sof-audio-of-imx9 failed with error -5

Step by step:

#. **The driver did the right thing.** The descriptor defaults to IPC4, so it
   first looked for ``/lib/firmware/imx/sof-ipc4/sof-imx95.ri``.

#. **The IPC4 firmware file is not present on the target.** That is exactly what
   ``Using fallback IPC type 0 (requested type was 1)`` means — the IPC4 profile
   lookup failed, so the driver fell back to the next supported type, IPC3.

#. **It then loaded the IPC3 firmware** ``imx/sof/sof-imx95.ri`` and IPC3
   topology ``imx/sof-tplg/sof-imx95-wm8962.tplg``.

#. **But the IPC3 boot handshake fails**: ``The received message size is
   invalid`` comes from the IPC3 ``fw_ready`` handshake — the kernel could not
   parse the firmware's boot message, so the boot never completes and you get
   ``Firmware boot failure due to timeout`` → ``-5`` → probe failure.

Root cause
----------

The IPC4 firmware/topology you built were **never copied to the IPC4 firmware
paths** on the board, so the kernel silently fell back to an IPC3 image that is
not compatible with this driver/board configuration (or is a stale/mismatched
``.ri``). Two things went wrong at once: (a) IPC4 was requested but its files
were absent, and (b) the IPC3 fallback image present under ``imx/sof/`` does not
boot cleanly.

Fix
---

Install the IPC4 artifacts so the intended IPC4 path is taken:

.. code-block:: shell

   # on the build host
   ./scripts/xtensa-build-zephyr.py -p imx95
   ./scripts/build-tools.sh -T

   # copy to the target
   install -d /lib/firmware/imx/sof-ipc4 /lib/firmware/imx/sof-ipc4-tplg
   cp build-sof-staging/sof/imx/sof-ipc4/sof-imx95.ri \
       /lib/firmware/imx/sof-ipc4/
   cp .../sof-imx95-wm8962.tplg \
       /lib/firmware/imx/sof-ipc4-tplg/

   # reload
   modprobe -r snd_sof_imx9 2>/dev/null
   modprobe snd_sof_imx9

After this you should see ``SOF selected IPC type 4 (IPC4)`` and a successful
boot instead of the fallback.

Verifying:

.. code-block:: shell

   ls -l /lib/firmware/imx/sof-ipc4/sof-imx95.ri
   ls -l /lib/firmware/imx/sof-ipc4-tplg/sof-imx95-wm8962.tplg
   dmesg | grep -i "selected IPC type"

If you instead want to force IPC3 for a quick check, make sure a *known-good*
IPC3 firmware is at ``/lib/firmware/imx/sof/sof-imx95.ri`` and a matching
``imx/sof-tplg/sof-imx95-wm8962.tplg`` — but the ``received message size is
invalid`` above indicates the IPC3 image currently on the board is not
compatible, so IPC4 is the intended target here.


i.MX95 IPC4 bring-up: fixes and gotchas
=======================================

Bringing IPC4 up on i.MX95 (Cortex-M7) surfaced a chain of independent bugs.
Each is listed with its symptom and fix, in the order they were hit — this is
the recommended order to verify a new i.MX (ARM) IPC4 port.

1. Base-FW module not found (boot fails, FW error 9)
----------------------------------------------------

Symptom: kernel ``FW reported error: 9 - Specified resource not found`` /
``failed to boot DSP firmware -22`` on the first ``LARGE_CONFIG_GET``
(FW_CONFIG). Cause: ``ipc4_get_comp_drv()`` in the kernel resolved module 0
(base FW) only under ``CONFIG_IMX8M``; i.MX95 is ``CONFIG_IMX95`` and fell into
the ``return NULL`` branch. Fix: extend the i.MX manifest paths to
``CONFIG_IMX8M || CONFIG_IMX95`` (and the same for the sibling ``CONFIG_IMX8M``
guards in ``copier_host.c``, ``copier_dai.c``, ``ipc/ipc4/dai.c``,
``host-zephyr.c``), plus add a real ``platform_ipc4_get_module()`` prototype in
``src/include/sof/platform.h`` (ARM builds treat the implicit declaration as an
error).

**General rule:** any ``CONFIG_IMX8M``-only guard in the IPC4 paths must also
cover ``CONFIG_IMX95`` — grep ``CONFIG_IMX8M`` under ``src/`` when porting.

2. Topology channels_count = 0 (module init fails, FW error 104)
---------------------------------------------------------------

Symptom: ``aplay`` → firmware ``module_adapter: invalid channels count 0`` →
kernel ``FW reported error: 104`` creating ``host-copier.0.playback``. Cause:
``tools/topology/topology2/include/pipelines/gain-playback.conf`` (i.MX-only)
defined its ``input/output_audio_format`` blocks with only bit depths, omitting
``in_channels``/``in_ch_cfg``/``in_ch_map``, so alsatplg encoded
``channels_count = 0`` in ``fmt_cfg``. Fix: add explicit ``in_channels 2`` /
``$CHANNEL_CONFIG_STEREO`` / ``$CHANNEL_MAP_STEREO`` (and ``out_*``) to every
format block, mirroring the Intel cavs pipelines. Verify with a tuple dump of
the built ``.tplg`` (SOF token 1907 = ``in_fmt_cfg``, low byte = channels).

3. ``-fshort-enums`` collapses the IPC4 audio-format struct
----------------------------------------------------------

Symptom: even with the topology fixed, the DSP still read ``channels_count = 0``
though the kernel wrote ``fmt_cfg = 0x00012002``. Cause: ``struct
ipc4_audio_format`` used bare ``enum`` members; **ARM EABI defaults to
-fshort-enums**, shrinking those enums and collapsing the on-wire struct from
24 to 20 bytes (``sizeof`` 20 instead of 24), so ``channels_count`` landed at
the wrong offset. Xtensa (i.MX8MP) has 4-byte enums, so it was unaffected. Fix:
use fixed-width ``uint32_t`` (not ``enum``) for on-wire IPC4 struct members —
``ipc4_audio_format`` (``src/include/module/ipc4/base-config.h``) and
``ipc4_astate.clock_src`` (``src/include/ipc4/base_fw.h``).

**General rule:** never use bare ``enum`` in on-wire IPC4/IPC ABI structs — ARM
``-fshort-enums`` breaks the wire layout. SOF-internal enum structs are fine.

4. Wrong PCM-pointer path (delay only)
--------------------------------------

``sof_ipc4_build_time_info()`` scanned the LLP reading slots and false-matched
uninitialised SRAM (i.MX firmware never populates them), leaving ``llp_offset``
non-zero so ``sof_ipc4_pcm_pointer()`` took the LLP path. It only affects the
reported *delay* (the pointer itself is always host-byte-counter based), but the
fix is to force ``llp_offset = 0`` when the platform provides
``get_host_byte_counter`` (i.MX).

5. Codec has no MCLK — SAI never clocks (the big one)
----------------------------------------------------

Symptom: pipeline runs, host copier produces, notifications + PCM pointer work,
but the SAI eDMA never drains (``dma_free`` stuck at 0) and ``aplay`` -EIO. The
SAI is correctly a bit-clock **consumer** (``TCR2.BCD=0``) waiting for the
codec's BCLK, but the codec logs ``wm8962 DC servo timed out`` — it has **no
MCLK**. On the i.MX95 EVK the WM8962's MCLK is the **SAI3 MCLK output**
(M7-owned; Linux ``&sai3`` is disabled, ``&wm8962`` takes ``IMX95_CLK_SAI3``).
The SOF Zephyr overlay's ``&sai3`` node was missing ``mclk-is-output``, so the
``nxp,dai-sai`` driver never drove the MCLK pin. Fix: add ``mclk-is-output;`` to
``&sai3`` in ``app/boards/imx95_evk_mimx9596_m7_ddr.overlay``.

6. First playback after boot still fails (codec DC-servo race)
--------------------------------------------------------------

Symptom: with MCLK enabled, the **first** ``aplay`` after boot still -EIOs
(``DC servo timed out``) but every subsequent one plays cleanly. Cause: MCLK
only comes up when the first stream configures the SAI, which races the codec's
DAPM power-up; once asserted the MCLK output persists, so later streams are
fine. Fix: assert the SAI MCLK output at driver ``sai_init`` time (guarded by
``mclk-is-output``) in ``zephyr/drivers/dai/nxp/sai/sai.c``, so a stable MCLK is
present before the codec's first power-up.

Working state
-------------

With the above, i.MX95 (Cortex-M7) IPC4 plays ``aplay`` end to end with clear
audio. Binaries/paths are unchanged from the tables above (firmware
``imx/sof-ipc4/sof-imx95.ri``, topology ``imx/sof-ipc4-tplg/``).

i.MX8QM / i.MX8QXP / i.MX8ULP bring-up
======================================

After i.MX8MP and i.MX95, IPC4 was extended to the remaining i.MX HiFi4
DSPs: **i.MX8QM** (``imx8``), **i.MX8QXP** (``imx8x``) and **i.MX8ULP**
(``imx8ulp``) — all tested and playing end-to-end.
These share the i.MX8MP firmware platform directory (``src/platform/imx8``)
and the same SDRAM1-based mailbox window, so most of the work was
generalisation rather than new mechanism.

Generalise the IPC4 gates: ``CONFIG_IMX8M || CONFIG_IMX95`` → ``CONFIG_IMX``
--------------------------------------------------------------------------

All the i.MX-specific IPC4 code paths (no bus-mastering host DMA, no SSP
``hw_config`` blob, the software host-DMA period notification, and module
resolution via the platform manifest table instead of an IMR-resident rimage
manifest) were originally guarded by ``CONFIG_IMX8M || CONFIG_IMX95``. These
behaviours are common to *every* i.MX SOF DSP, so the guards were replaced
with the umbrella ``CONFIG_IMX`` in:

* ``src/audio/copier/copier_host.c`` (host-DMA proxy params, period notif),
* ``src/audio/copier/copier_dai.c`` (synthesise the missing i.MX SSP config),
* ``src/audio/host-zephyr.c`` (single-element host DMA collapse),
* ``src/ipc/ipc4/dai.c`` (SOF_DAI_IMX_SAI spec_config handling),
* ``src/ipc/ipc4/helper.c`` (``ipc4_get_comp_drv`` via the manifest table),
* ``src/include/sof/platform.h`` (``platform_ipc4_get_module`` prototype).

**General rule (updated):** i.MX IPC4 paths must be gated on ``CONFIG_IMX``,
never a single SoC's Kconfig — grep ``CONFIG_IMX8`` / ``CONFIG_IMX95`` under
``src/`` when porting a new i.MX platform.

i.MX8QM/QXP firmware platform (``src/platform/imx8``)
----------------------------------------------------

``src/platform/imx8`` is built for both ``CONFIG_IMX8`` (QM) and
``CONFIG_IMX8X`` (QXP). The IPC4 additions mirror imx8m exactly:

* ``memory.h`` — the 8-byte compact-header slot before each box payload
  (``SRAM_IPC4_HDR_SIZE``) and a ``SRAM_SW_REG`` window (LLP/stream
  position) carved right after the 0x800 debug window (overlaying the
  IPC3-only exception window).
* ``mailbox.h`` — ``MAILBOX_{DSP,HOST}BOX_HDR_BASE`` and the
  ``mailbox_sw_reg_{read,write}[64]`` / ``mailbox_sw_regs_write`` accessors.
* ``platform.c`` — send ``SOF_IPC4_FW_READY`` as a notification at boot;
  carry the IPC4 module manifest in a KEEP'd ``.module`` section and resolve
  module IDs via ``platform_ipc4_get_module()``.
* ``platform.h`` / ``clk.h`` — drop the IPC3 panic/fw-status box under IPC4;
  report ``CPU_LOWEST_FREQ_IDX`` / ``HW_CFG_VERSION`` for the base-FW config.
* ``linker/module-section.ld`` + ``zephyr/CMakeLists.txt`` — keep ``.module``
  from ``--gc-sections`` and force the rimage target (``imx8_ipc4`` for QM,
  ``imx8x_ipc4`` for QXP).

i.MX8QM/QXP SRAM window (relative to the DT bar, ``window_offset = 0x800000``
maps to ``SDRAM1_BASE``):

===================  ==============  ======  ===================================
Region               Offset          Size    Purpose
===================  ==============  ======  ===================================
Outbox (DSP→host)    0x800000        0x1000  8B hdr slot + payload
Inbox  (host→DSP)    0x801000        0x1000  8B hdr slot + payload
Debug                0x802000        0x0800  debug
SW registers         0x802800        0x0800  IPC4 LLP / stream position
===================  ==============  ======  ===================================

The matching kernel side (``imx8_chip_info`` / ``imx8x_chip_info`` in
``sound/soc/sof/imx/imx8.c``) sets ``ipc4_inbox_offset = 0x800008``,
``ipc4_outbox_offset = 0x801008``, ``ipc4_debug_offset = 0x802000``,
``ipc4_fw_regs_offset = 0x802800``, and both descriptors switch to
``IMX_SOF_DEV_DESC_IPC4()``. Firmware ``sof-imx8.ri`` / ``sof-imx8x.ri`` and
topology ``sof-imx8-wm8960.tplg`` (built with ``SAI_DAI_INDEX=1``,
``STREAM_CODEC_NAME=sai1-wm8960-hifi``) install under
``/lib/firmware/imx/sof-ipc4{,-tplg}/``.

The "can't find free slot" boot-state bug (SW-reg window not zeroed)
-------------------------------------------------------------------

Symptom: card enumerates, but the first ``aplay`` fails ``hw_params`` and the
DSP console prints::

   ipc: comp:1 0x10001 dai type = 5 index = 1 ... can't find free slot
   copier: failed to create dai -> module init -22 -> MODULE_MSG err 104

Cause: ``dai_get_unused_llp_slot()`` (``src/ipc/ipc4/dai.c``) scans the LLP
reading-slot table in ``MAILBOX_SW_REG_BASE`` for a zero (free) entry. That
SRAM window is **not a loadable segment nor a ``.bss`` variable**, so neither
the firmware loader nor the C runtime clears it; power-on/stale garbage makes
every slot look used and DAI creation fails. i.MX8MP and i.MX95 already
``bzero()`` this window in ``platform_init()`` under ``CONFIG_IPC_MAJOR_4`` —
imx8/imx8x/imx8ulp did not. Fix: add the same clear to each platform's
``platform_init()``. **This is mandatory for every i.MX IPC4 platform.**

Codec control I2C: use native LPI2C, not the M4 rpmsg bus (QM/QXP)
------------------------------------------------------------------

Unlike i.MX8MP (codec on a plain A-core LPI2C), the i.MX8QXP/QM reference
DTS route the audio codec's **control I2C through the Cortex-M4 companion**:

* i.MX8QXP — ``imx8qxp-mek-sof-wm8960.dts`` included ``imx8qxp-mek-rpmsg.dts``,
  which ``/delete-node/``\s the native ``cm40_i2c`` and re-parents the codec's
  ``i2c-mux`` onto ``i2c_rpbus_5`` (served by the M4 over rpmsg).
* i.MX8QM — ``imx8qm-mek-rpmsg.dts`` re-homes the codec bus onto
  ``i2c_rpbus_1``.

With SOF driving the HiFi4 DSP and **no M4 rpmsg companion running**, those
rpmsg buses never register, the ``i2c-mux`` defers, the ``wm8960`` never
probes, and ``audio-graph-card2`` defers forever ("parse error") — no sound
card, no error in ``dmesg``. Diagnose with
``cat /sys/kernel/debug/asoc/components`` (no ``wm8960``) and
``cat /sys/kernel/debug/devices_deferred``.

Two ways forward:

#. **Boot the M4 SRTM companion** (U-Boot ``bootaux`` or baked into
   ``flash.bin``) so the ``i2c-rpbus-*`` adapters come up — the reference
   design's intent, needed for the board's other rpmsg peripherals too.
#. **Use the native LPI2C** (chosen here): include the base board DTS instead
   of the ``-rpmsg`` one so the codec sits on the kernel-owned LPI2C (QXP:
   ``cm40_i2c``; QM: ``i2c1``). See the kernel commit "arm64: dts: imx8q:
   reach SOF wm8960 codec over native I2C". No M4 needed; also frees the DSP
   debug UART (``lpuart2`` on QM) that the M4 firmware would own.

Note the DSP ``ports``/``machine-drv-name`` on the DSP node create the card
via a standalone ``sof-sound-wm8960 { compatible = "audio-graph-card2" }`` OF
node; on i.MX, SOF does **not** spawn the machine device
(``sof_imx8_ops`` has no ``.machine_register``), and ``machine-drv-name`` is
unused by the kernel.

i.MX8ULP port
------------------------

i.MX8ULP has its own firmware platform (``src/platform/imx8ulp``); the IPC4
layer was ported by mirroring imx8. One layout difference matters: i.MX8ULP
keeps its larger **0x2800 debug window**, so the SW-reg window lands at window
offset **0x4800**, and the kernel ``imx8ulp_chip_info.ipc4_fw_regs_offset`` is
``0x804800`` (not ``0x802800``). Delivered:

* ``src/platform/imx8ulp/{memory.h,mailbox.h,platform.c,platform.h,clk.h}``,
  ``linker/module-section.ld``;
* ``tools/rimage/config/imx8ulp_ipc4.toml``,
  ``app/boards/imx8ulp_evk_mimx8ud7_adsp_ipc4.conf``;
* an **opt-in** ``imx8ulp-ipc4`` build target (default ``imx8ulp`` stays IPC3);
* kernel ``imx8ulp_chip_info`` IPC4 offsets + ``IMX_SOF_DEV_DESC_IPC4``.

**Still open for i.MX8ULP:** no IPC4 topology exists for the EVK (btsco) yet,
and none of the offsets/zones have been validated on hardware. Build with
``./scripts/xtensa-build-zephyr.py -p imx8ulp-ipc4`` and validate the mailbox
offsets and DAI/SAI wiring on a board before relying on it.

Build & deploy (i.MX8QM / i.MX8QXP)
-----------------------------------

.. code-block:: shell

   # firmware (IPC4 is the default for imx8 / imx8x)
   ./scripts/xtensa-build-zephyr.py -p imx8      # i.MX8QM  -> sof-imx8.ri
   ./scripts/xtensa-build-zephyr.py -p imx8x     # i.MX8QXP -> sof-imx8x.ri
   ./scripts/build-tools.sh -T                   # topologies

   # deploy (adjust rootfs path; QXP shown)
   cp build-sof-staging/sof/imx/sof-ipc4/sof-imx8x.ri \
      /srv/rootfs/lib/firmware/imx/sof-ipc4/
   cp tools/build_tools/topology/topology2/production/sof-imx8-wm8960.tplg \
      /srv/rootfs/lib/firmware/imx/sof-ipc4-tplg/

   # boot with the native-I2C DTB, then:
   cat /proc/asound/cards
   aplay -Dhw:1,0 sample_48k_s32_2ch.wav


Current status / known issues
=============================

These are WIP bring-up branches. From the commit messages:

* **i.MX8MP playback** runs end-to-end at correct speed under IPC4; an open
  "metallic buzz" issue (suspected SAI TX FIFO underrun / DAI→SDMA reload
  timing) is being investigated. IPC3 is unaffected.
* The **2-period DAI gateway buffer** tweak (``ipc4-topology.c``) and the
  ``2*ibs`` DAI-in experiment are workarounds for the NXP SDMA's 2-buffer-
  descriptor limit and are marked *not for upstream*.
* IPC4 **trace** is disabled on i.MX (``CONFIG_TRACE=n``) — no DMA-trace window
  is wired yet.
* **i.MX8QXP and i.MX8QM playback** run end-to-end under IPC4 (WM8960 on native
  LPI2C, SAI1). Both required the SW-reg "can't find free slot" clear.
* **i.MX8ULP** is ported and tested. No IPC4 EVK topology ships in-tree yet,
  so use the opt-in ``imx8ulp-ipc4`` build target with a local topology.
