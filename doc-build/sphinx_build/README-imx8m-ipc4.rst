.. SPDX-License-Identifier: BSD-3-Clause
.. Copyright 2024 NXP

SOF IPC4 Support for i.MX8M Platforms
=======================================

This document describes the IPC4 support added for i.MX8M platforms
(specifically i.MX8MP with HiFi4 DSP), explains the differences between
IPC3 and IPC4 in detail including the precise binary message formats,
and provides an overview of the SOF architecture as it relates to these
changes.

.. contents:: Table of Contents
   :depth: 3
   :local:

----

SOF Architecture Overview
--------------------------

Sound Open Firmware (SOF) is an open-source audio DSP firmware and SDK.
It runs on the DSP core of a SoC and communicates with a Linux host driver
over a shared-memory mailbox + interrupt mechanism.

The main layers in the SOF stack are:

::

   +---------------------------------------------------------------+
   |                     Linux Host (ARM)                          |
   |  sound/soc/sof/        SOF ASoC driver                        |
   |    imx/imx8.c          NXP chip-specific ops                  |
   |    imx/imx-common.c    Shared probe / IPC send / IPC recv     |
   +---------------------------+-----------------------------------+
                               |  Messaging Unit (MU)
                               |  Shared SRAM mailbox
   +---------------------------+-----------------------------------+
   |                     DSP Firmware (Xtensa)                     |
   |  src/drivers/imx/ipc.c     IPC platform driver               |
   |  src/platform/imx8m/       Platform init, memory map         |
   |  src/audio/                Pipeline components (gain, SRC)    |
   |  Zephyr RTOS               Scheduler, DMA, DAI drivers        |
   +---------------------------------------------------------------+

**Key hardware blocks on i.MX8MP:**

- **HiFi4 DSP** -- the Xtensa core running SOF firmware.
- **Messaging Unit (MU2)** -- a hardware block providing interrupt-based
  signalling between the ARM host and the DSP.  It has General Purpose
  Interrupt (GPI/GIP) channels, but no data registers wired for SOF use.
- **SDMA3** -- the System DMA engine used to move audio data between the
  SAI peripherals and DSP-accessible SDRAM.
- **SAI (Synchronous Audio Interface)** -- the I2S/TDM peripheral
  connected to external codecs (e.g. WM8960 on the EVK board).
- **MICFIL** -- the digital microphone (PDM) interface.

**Memory regions relevant to IPC:**

::

   SDRAM0  0x92400000  8 MB   DSP code + data (firmware image loaded here)
   SDRAM1  0x92C00000  8 MB   Mailbox, heaps, stacks

   Within SDRAM1:
     0x92C00000  outbox (DSP->host) payload   4 KB
     0x92C01000  inbox  (host->DSP) payload   4 KB
     0x92C02000  debug window                 2 KB
     0x92C02800  SW registers (IPC4 LLP)      2 KB
     0x92C03000  stream window                4 KB
     0x92C04000  trace buffer                 4 KB
     remainder   heaps + stack

----

IPC Fundamentals
-----------------

Both IPC3 and IPC4 are built on the same physical transport: a pair of
shared SRAM mailbox buffers (inbox and outbox) plus a Messaging Unit (MU)
interrupt to signal when a new message is ready.  What differs between
the two generations is the *encoding* of messages -- how commands are
described, how parameters are conveyed, and how the DSP routes incoming
messages to the right handler.

At the lowest level both protocols share a two-word abstract header type
defined in the platform-agnostic SOF transport layer:

.. code-block:: c

   /* src/include/ipc3/header.h  (IPC3 transport abstraction) */
   /* src/include/ipc4/header.h  (IPC4 transport abstraction) */
   struct ipc_cmd_hdr {
       uint32_t pri;   /* primary 32-bit word  */
       uint32_t ext;   /* extension 32-bit word */
   };

In IPC3 the ``pri`` word carries the entire command encoding and ``ext``
is rarely used.  In IPC4 both words are always meaningful and together
encode the full command without any mailbox access for the header itself.

----

IPC3 -- Legacy Protocol
------------------------

IPC3 is the original SOF inter-processor communication protocol.  Every
interaction between the host and the DSP is a struct-serialised message
written into the shared SRAM mailbox.

IPC3 Message Format
~~~~~~~~~~~~~~~~~~~~

Every IPC3 message starts with a standard two-field header followed by a
command-specific payload struct, all serialised into the shared mailbox:

.. code-block:: c

   /* src/include/ipc/header.h */

   struct sof_ipc_cmd_hdr {
       uint32_t size;   /* total size of message in bytes (header + payload) */
       uint32_t cmd;    /* encoded command -- see bit-field layout below      */
   } __attribute__((packed, aligned(4)));

The ``cmd`` word uses a three-level hierarchy packed into 32 bits:

.. code-block:: text

   Bit  31       28 27           16 15                0
        +----------+---------------+-------------------+
        |    G     |       C       |        N          |
        |   4 b    |    12 b       |       16 b        |
        +----------+---------------+-------------------+
        G  = Global type   (bits [31:28])
        C  = Command type  (bits [27:16])
        N  = Sequence / ID (bits [15:0])

**Global type values** (``G``, 4 bits):

.. code-block:: c

   /* src/include/ipc/header.h */
   SOF_IPC_GLB_REPLY        = 0x1   /* DSP->host reply                  */
   SOF_IPC_GLB_COMPOUND     = 0x2   /* Batched compound message          */
   SOF_IPC_GLB_TPLG_MSG     = 0x3   /* Topology (component/pipe/buffer) */
   SOF_IPC_GLB_PM_MSG       = 0x4   /* Power Management                 */
   SOF_IPC_GLB_COMP_MSG     = 0x5   /* Component runtime config         */
   SOF_IPC_GLB_STREAM_MSG   = 0x6   /* Stream / PCM                     */
   SOF_IPC_FW_READY         = 0x7   /* Firmware ready notification      */
   SOF_IPC_GLB_DAI_MSG      = 0x8   /* DAI (SSP / I2S / HDA / SAI)     */
   SOF_IPC_GLB_TRACE_MSG    = 0x9   /* Trace / DMA                      */
   SOF_IPC_GLB_GDB_DEBUG    = 0xA   /* GDB stub                         */
   SOF_IPC_GLB_TEST         = 0xB   /* Test (debug builds only)         */
   SOF_IPC_GLB_PROBE        = 0xC   /* Data probing                     */
   SOF_IPC_GLB_DEBUG        = 0xD   /* Debug / memory usage             */

**Command subtype values** (``C``, 12 bits):

.. code-block:: c

   /* Topology (G = SOF_IPC_GLB_TPLG_MSG) */
   SOF_IPC_TPLG_COMP_NEW       = 0x001   /* create a component           */
   SOF_IPC_TPLG_COMP_FREE      = 0x002
   SOF_IPC_TPLG_COMP_CONNECT   = 0x003   /* connect source to sink       */
   SOF_IPC_TPLG_PIPE_NEW       = 0x010   /* create a pipeline            */
   SOF_IPC_TPLG_PIPE_FREE      = 0x011
   SOF_IPC_TPLG_PIPE_COMPLETE  = 0x013   /* pipeline fully configured    */
   SOF_IPC_TPLG_BUFFER_NEW     = 0x020   /* create a buffer              */
   SOF_IPC_TPLG_BUFFER_FREE    = 0x021

   /* Power Management (G = SOF_IPC_GLB_PM_MSG) */
   SOF_IPC_PM_CTX_SAVE         = 0x001
   SOF_IPC_PM_CTX_RESTORE      = 0x002
   SOF_IPC_PM_CORE_ENABLE      = 0x007
   SOF_IPC_PM_GATE             = 0x008

   /* Component (G = SOF_IPC_GLB_COMP_MSG) */
   SOF_IPC_COMP_SET_VALUE      = 0x001
   SOF_IPC_COMP_GET_VALUE      = 0x002
   SOF_IPC_COMP_SET_DATA       = 0x003
   SOF_IPC_COMP_GET_DATA       = 0x004

   /* Stream (G = SOF_IPC_GLB_STREAM_MSG) */
   SOF_IPC_STREAM_PCM_PARAMS   = 0x001   /* configure PCM format         */
   SOF_IPC_STREAM_TRIG_START   = 0x004   /* start stream                 */
   SOF_IPC_STREAM_TRIG_STOP    = 0x005
   SOF_IPC_STREAM_TRIG_PAUSE   = 0x006
   SOF_IPC_STREAM_TRIG_DRAIN   = 0x008
   SOF_IPC_STREAM_POSITION     = 0x00a   /* position notification        */

   /* DAI (G = SOF_IPC_GLB_DAI_MSG) */
   SOF_IPC_DAI_CONFIG          = 0x001   /* set DAI (SSP/SAI/DMIC) params */

Each command type has a dedicated C struct that is placed into the mailbox
after the common header.  For example, creating a pipeline uses:

.. code-block:: c

   struct sof_ipc_pipe_new {
       struct sof_ipc_cmd_hdr hdr;  /* size + cmd */
       uint32_t comp_id;            /* pipeline component ID             */
       uint32_t pipeline_id;        /* pipeline instance number          */
       uint32_t sched_id;           /* scheduling source component ID    */
       uint32_t core;               /* DSP core index                    */
       uint32_t period;             /* scheduling period in microseconds */
       uint32_t priority;           /* pipeline priority                 */
       uint32_t period_mips;        /* worst-case MIPS budget            */
       uint32_t frames_per_sched;   /* frames per scheduling slot        */
       uint32_t xrun_limit_usecs;   /* underrun/overrun alarm threshold  */
       uint32_t timer_domain;       /* timer vs. DMA domain              */
   } __attribute__((packed, aligned(4)));

**IPC3 reply format:**

.. code-block:: c

   struct sof_ipc_reply {
       struct sof_ipc_cmd_hdr hdr;   /* hdr.cmd echoes the original command */
       int32_t error;                /* 0 on success, negative errno on failure */
   } __attribute__((packed, aligned(4)));

The entire chain (command write, doorbell, handler, reply write, reply
doorbell) must complete before the host sends the next command.  Maximum
message size is 384 bytes (bare metal) or 0x2000 bytes (static library
builds).

IPC3 Message Dispatch
~~~~~~~~~~~~~~~~~~~~~~

The DSP firmware reads the entire message from the mailbox, validates the
``size`` field, then dispatches on the ``G`` bits:

.. code-block:: text

   Host mailbox write + MU doorbell IRQ
          |
          v
   mailbox_validate()           reads full struct from SRAM, checks size
          |
          v
   ipc_cmd(hdr)                 src/ipc/ipc3/handler.c
          |
          +-- G=TPLG_MSG   --> ipc_glb_tplg_message()  --> switch on C bits
          |       +-- COMP_NEW         --> ipc_comp_new()
          |       +-- COMP_CONNECT     --> ipc_comp_connect()
          |       +-- PIPE_NEW         --> ipc_new_task_pipeline()
          |       +-- BUFFER_NEW       --> ipc_buffer_new()
          |
          +-- G=STREAM_MSG --> ipc_glb_stream_message() --> switch on C bits
          |       +-- PCM_PARAMS       --> ipc_stream_pcm_params()
          |       +-- TRIG_START       --> ipc_stream_trigger()
          |       +-- TRIG_STOP        --> ipc_stream_trigger()
          |
          +-- G=COMP_MSG   --> ipc_glb_comp_message()   --> switch on C bits
          |       +-- SET_VALUE        --> comp_set_value()
          |       +-- GET_DATA         --> comp_get_data()
          |
          +-- G=PM_MSG     --> ipc_glb_pm_message()
          +-- G=DAI_MSG    --> ipc_glb_dai_message()
          +-- G=TRACE_MSG  --> ipc_glb_trace_message()
          +-- G=FW_READY   --> ipc_fw_ready()

Boot Sequence (IPC3)
~~~~~~~~~~~~~~~~~~~~~

1. Host loads ``sof-imx8m.ri`` into SDRAM0 and releases the DSP reset.
2. DSP initialises and writes the ``sof_ipc_fw_ready`` struct (containing
   version info and window layout) into the outbox at offset 0.
3. DSP asserts MU GIR1 (General Interrupt Request #1) to tell the host
   that firmware is ready.
4. Host reads the ``fw_ready`` struct from the mailbox to discover the
   window layout.

Topology (IPC3 -- topology1)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Topology is described using M4 preprocessor macros in ``tools/topology/
topology1/*.m4`` files, compiled with ``m4`` and ``alsatplg``.  Each
topology file hard-codes the full pipeline graph, component parameters,
and DAI bindings.

Limitations of IPC3
~~~~~~~~~~~~~~~~~~~~

- Flat message structs are hard to extend without breaking ABI.
- No concept of loadable modules -- all components are compiled into the
  firmware image.
- Topology1 (m4) syntax is difficult to read, share, and parameterise.
- No standardised pipeline model; each vendor made their own design
  decisions.
- The full mailbox must be read on every IPC before dispatch is possible.
- Component addressing uses flat 32-bit IDs with no type information.

----

IPC4 -- Modern Protocol
------------------------

IPC4 is a redesigned protocol that introduces a module-centric pipeline
model, a compact two-register binary header, explicit pin-based module
connectivity, dynamic library loading, and an extended manifest format for
firmware images.

IPC4 Conceptual Model
~~~~~~~~~~~~~~~~~~~~~~

Where IPC3 operates on static, opaque *components* with implicit wiring,
IPC4 treats the DSP as a **dynamic module graph**:

- A **pipeline** is an empty container (scheduling domain + memory pool).
  It has a numeric ``instance_id`` (0--255).
- A **module** is a processing node.  Each module type has a globally
  unique 16-bit ``module_id``.  Multiple *instances* of the same module
  type can exist simultaneously, each with its own 8-bit ``instance_id``.
  Instances are addressed as the tuple ``(module_id, instance_id)``.
- Instances are connected explicitly via named **pins** (``src_queue`` and
  ``dst_queue``, each 3 bits wide, supporting up to 8 input and 8 output
  pins per module).  This allows mixers, splitters, and other multi-pin
  modules without any special-casing.
- Pipeline **state** transitions drive the scheduler:
  ``RESET -> PAUSED -> RUNNING -> EOS -> RESET``.

The full lifecycle for a single playback pipeline:

.. code-block:: text

   Host                             DSP
   ---------------------------------------------------------------
   CREATE_PIPELINE(id=1)          -> allocate pipeline object
   MOD_INIT_INSTANCE(host-copier, instance=0, pipeline=1)
   MOD_INIT_INSTANCE(gain,        instance=0, pipeline=1)
   MOD_INIT_INSTANCE(dai-copier,  instance=1, pipeline=1)
   MOD_BIND(host-copier.0 -> gain.0,      src_queue=0, dst_queue=0)
   MOD_BIND(gain.0 -> dai-copier.1,       src_queue=0, dst_queue=0)
   SET_PIPELINE_STATE(id=1, PAUSED)   -> allocate buffers, prepare DMA
   SET_PIPELINE_STATE(id=1, RUNNING)  -> LL scheduler starts
   ... audio streaming ...
   SET_PIPELINE_STATE(id=1, RESET)    -> stop + reset
   MOD_DELETE_INSTANCE(host-copier, 0)
   MOD_DELETE_INSTANCE(gain, 0)
   MOD_DELETE_INSTANCE(dai-copier, 1)
   DELETE_PIPELINE(id=1)

IPC4 Message Format -- Header Encoding
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Every IPC4 message fits in exactly **two 32-bit words** (``primary`` and
``extension``).  On Intel platforms these map directly to Messaging Unit
data registers so no SRAM read is needed to decode the command.  On i.MX,
where the MU has no data registers, SOF reserves an 8-byte *header slot*
immediately before each mailbox payload area (see `Why i.MX Needs Special
Handling`_ below).

The two-word layout is defined as ``struct ipc4_message_request`` in
``src/include/ipc4/header.h``:

.. code-block:: c

   /* src/include/ipc4/header.h */
   struct ipc4_message_request {
       union {
           uint32_t dat;
           struct {
               uint32_t rsvd0   : 24;  /* bits [23:0]  -- type-specific data    */
               uint32_t type    :  5;  /* bits [28:24] -- message type (5 bits) */
               uint32_t rsp     :  1;  /* bit  [29]    -- 0=request, 1=reply    */
               uint32_t msg_tgt :  1;  /* bit  [30]    -- 0=FW_GEN, 1=MODULE    */
               uint32_t _rsvd   :  1;  /* bit  [31]    -- HW reserved           */
           } r;
       } primary;

       union {
           uint32_t dat;
           struct {
               uint32_t ext_data   : 30;  /* bits [29:0]  -- extension data     */
               uint32_t _reserved  :  2;  /* bits [31:30] -- HW reserved        */
           } r;
       } extension;
   } __attribute__((packed, aligned(4)));

**Top-level routing -- bit 30** (``msg_tgt``):

.. code-block:: text

   msg_tgt = 0  -->  SOF_IPC4_MESSAGE_TARGET_FW_GEN_MSG
                     Global or pipeline-level messages.
                     bits [28:24] carry an ipc4_message_type value.

   msg_tgt = 1  -->  SOF_IPC4_MESSAGE_TARGET_MODULE_MSG
                     Per-module-instance messages.
                     bits [28:24] carry a sof_ipc4_module_type value.
                     bits [23:16] carry the target instance_id.
                     bits [15:0]  carry the target module_id.

**Direction -- bit 29** (``rsp``):

.. code-block:: text

   rsp = 0  -->  SOF_IPC4_MESSAGE_DIR_MSG_REQUEST (host->DSP command
                                                   or DSP->host notification)
   rsp = 1  -->  SOF_IPC4_MESSAGE_DIR_MSG_REPLY   (DSP->host reply)

In a reply the ``primary`` bits [23:0] (``rsvd0`` in the request) carry a
24-bit ``ipc4_status`` code instead of command data:

.. code-block:: c

   struct ipc4_message_reply {
       union {
           uint32_t dat;
           struct {
               uint32_t status  : 24;  /* bits [23:0]  -- ipc4_status code     */
               uint32_t type    :  5;  /* bits [28:24] -- mirrored from request */
               uint32_t rsp     :  1;  /* bit  [29]    -- always 1 for replies  */
               uint32_t msg_tgt :  1;  /* bit  [30]    -- mirrored from request */
               uint32_t _rsvd   :  1;
           } r;
       } primary;
       union { uint32_t dat; } extension;
   };

IPC4 Global Message Types (msg_tgt = FW_GEN_MSG)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

When ``msg_tgt = 0``, bits [28:24] select the global operation:

.. code-block:: c

   /* src/include/ipc4/header.h */
   enum ipc4_message_type {
       SOF_IPC4_GLB_BOOT_CONFIG              =  0,
       SOF_IPC4_GLB_ROM_CONTROL              =  1,
       SOF_IPC4_GLB_IPCGATEWAY_CMD           =  2,
       /* values 3-12 reserved */
       SOF_IPC4_GLB_PERF_MEASUREMENTS_CMD    = 13,
       SOF_IPC4_GLB_CHAIN_DMA                = 14,  /* host<->link DMA chaining  */
       SOF_IPC4_GLB_LOAD_MULTIPLE_MODULES    = 15,
       SOF_IPC4_GLB_UNLOAD_MULTIPLE_MODULES  = 16,
       SOF_IPC4_GLB_CREATE_PIPELINE          = 17,  /* allocate pipeline         */
       SOF_IPC4_GLB_DELETE_PIPELINE          = 18,
       SOF_IPC4_GLB_SET_PIPELINE_STATE       = 19,  /* RESET/PAUSE/RUNNING/EOS   */
       SOF_IPC4_GLB_GET_PIPELINE_STATE       = 20,
       SOF_IPC4_GLB_GET_PIPELINE_CONTEXT_SIZE = 21,
       SOF_IPC4_GLB_SAVE_PIPELINE            = 22,
       SOF_IPC4_GLB_RESTORE_PIPELINE         = 23,
       SOF_IPC4_GLB_LOAD_LIBRARY             = 24,
       SOF_IPC4_GLB_LOAD_LIBRARY_PREPARE     = 25,
       SOF_IPC4_GLB_INTERNAL_MESSAGE         = 26,
       SOF_IPC4_GLB_NOTIFICATION             = 27,  /* DSP->host async event     */
       /* values 28-30 reserved */
       SOF_IPC4_GLB_ENTER_GDB                = 31,
   };

IPC4 Module Message Types (msg_tgt = MODULE_MSG)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

When ``msg_tgt = 1``, bits [28:24] select the module operation:

.. code-block:: c

   /* src/include/ipc4/module.h */
   enum sof_ipc4_module_type {
       SOF_IPC4_MOD_INIT_INSTANCE         = 0,  /* create module instance    */
       SOF_IPC4_MOD_CONFIG_GET            = 1,  /* small get param (in hdr)  */
       SOF_IPC4_MOD_CONFIG_SET            = 2,  /* small set param (in hdr)  */
       SOF_IPC4_MOD_LARGE_CONFIG_GET      = 3,  /* fragmented get (mailbox)  */
       SOF_IPC4_MOD_LARGE_CONFIG_SET      = 4,  /* fragmented set (mailbox)  */
       SOF_IPC4_MOD_BIND                  = 5,  /* connect two instances     */
       SOF_IPC4_MOD_UNBIND                = 6,  /* disconnect two instances  */
       SOF_IPC4_MOD_SET_DX                = 7,  /* D0/D3 power transition    */
       SOF_IPC4_MOD_SET_D0IX              = 8,  /* D0ix sub-state            */
       SOF_IPC4_MOD_ENTER_MODULE_RESTORE  = 9,
       SOF_IPC4_MOD_EXIT_MODULE_RESTORE   = 10,
       SOF_IPC4_MOD_DELETE_INSTANCE       = 11, /* free module instance      */
   };

Key IPC4 Message Structures
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**Pipeline create** (``src/include/ipc4/pipeline.h``):

.. code-block:: c

   struct ipc4_pipeline_create {
       union {
           uint32_t dat;
           struct {
               uint32_t ppl_mem_size : 11;  /* pages for pipeline memory         */
               uint32_t ppl_priority :  5;  /* enum ipc4_pipeline_priority       */
               uint32_t instance_id  :  8;  /* pipeline ID (0..255)              */
               uint32_t type         :  5;  /* SOF_IPC4_GLB_CREATE_PIPELINE = 17 */
               uint32_t rsp          :  1;
               uint32_t msg_tgt      :  1;  /* 0 = FW_GEN_MSG                    */
               uint32_t _reserved_0  :  1;
           } r;
       } primary;
       union {
           uint32_t dat;
           struct {
               uint32_t lp          :  1;   /* 1 = low-power pipeline (WoV)      */
               uint32_t rsvd1       :  3;
               uint32_t attributes  : 16;
               uint32_t core_id     :  4;   /* which DSP core (0 = default)      */
               uint32_t rsvd2       :  5;
               uint32_t payload     :  1;   /* extended payload follows in SRAM  */
               uint32_t _reserved_2 :  2;
           } r;
       } extension;
   };

**Pipeline set state** (``src/include/ipc4/pipeline.h``):

.. code-block:: c

   struct ipc4_pipeline_set_state {
       union {
           uint32_t dat;
           struct {
               uint32_t ppl_state  : 16;  /* target state (see enum below)      */
               uint32_t ppl_id     :  8;  /* pipeline instance_id               */
               uint32_t type       :  5;  /* SOF_IPC4_GLB_SET_PIPELINE_STATE=19 */
               uint32_t rsp        :  1;
               uint32_t msg_tgt    :  1;
               uint32_t _reserved  :  1;
           } r;
       } primary;
       union {
           uint32_t dat;
           struct {
               uint32_t multi_ppl       :  1; /* 1 = IDs in mailbox payload     */
               uint32_t sync_stop_start :  1; /* sync gateways across pipelines */
               uint32_t rsvd1           : 28;
               uint32_t _reserved_2     :  2;
           } r;
       } extension;
   };

   enum ipc4_pipeline_state {
       SOF_IPC4_PIPELINE_STATE_INVALID       = 0,
       SOF_IPC4_PIPELINE_STATE_UNINITIALIZED = 1,
       SOF_IPC4_PIPELINE_STATE_RESET         = 2,  /* pipeline reset / ready     */
       SOF_IPC4_PIPELINE_STATE_PAUSED        = 3,  /* prepared, DMA set up       */
       SOF_IPC4_PIPELINE_STATE_RUNNING       = 4,  /* active, LL scheduler runs  */
       SOF_IPC4_PIPELINE_STATE_EOS           = 5,  /* end-of-stream drain        */
       SOF_IPC4_PIPELINE_STATE_ERROR_STOP,
       SOF_IPC4_PIPELINE_STATE_SAVED,
   };

**Module bind/unbind** (``src/include/ipc4/module.h``):

.. code-block:: c

   struct ipc4_module_bind_unbind {
       union {
           uint32_t dat;
           struct {
               uint32_t module_id   : 16;  /* source module type ID            */
               uint32_t instance_id :  8;  /* source module instance           */
               uint32_t type        :  5;  /* SOF_IPC4_MOD_BIND = 5 / UNBIND   */
               uint32_t rsp         :  1;
               uint32_t msg_tgt     :  1;  /* 1 = MODULE_MSG                   */
               uint32_t _reserved   :  1;
           } r;
       } primary;
       union {
           uint32_t dat;
           struct {
               uint32_t dst_module_id   : 16;  /* destination module type      */
               uint32_t dst_instance_id :  8;  /* destination instance         */
               uint32_t dst_queue       :  3;  /* destination input pin (0..7) */
               uint32_t src_queue       :  3;  /* source output pin (0..7)     */
               uint32_t _reserved_2     :  2;
           } r;
       } extension;
   };

**Module large config** (``src/include/ipc4/module.h``):

This is used for parameter blocks too large to fit in the two header words.
The payload is transferred in fragments through the mailbox, identified by
``large_param_id`` and sequenced with ``init_block`` / ``final_block``:

.. code-block:: c

   struct ipc4_module_large_config {
       union {
           uint32_t dat;
           struct {
               uint32_t module_id   : 16;
               uint32_t instance_id :  8;
               uint32_t type        :  5;  /* LARGE_CONFIG_GET=3 / SET=4        */
               uint32_t rsp         :  1;
               uint32_t msg_tgt     :  1;
               uint32_t _reserved   :  1;
           } r;
       } primary;
       union {
           uint32_t dat;
           struct {
               uint32_t data_off_size  : 20;  /* size (bytes) or fragment offset */
               uint32_t large_param_id :  8;  /* parameter ID                    */
               uint32_t final_block    :  1;  /* 1 if last fragment              */
               uint32_t init_block     :  1;  /* 1 if first fragment             */
               uint32_t _reserved_2    :  2;
           } r;
       } extension;
   };

**Notification header** (``src/include/ipc4/notification.h``):

DSP-to-host asynchronous events use ``msg_tgt=FW_GEN_MSG``,
``type=SOF_IPC4_GLB_NOTIFICATION``, and a notification type encoded in
bits [23:16] of the primary word:

.. code-block:: c

   union ipc4_notification_header {
       uint32_t dat;
       struct {
           uint32_t rsvd0      : 16;
           uint32_t notif_type :  8;  /* enum sof_ipc4_notification_type */
           uint32_t type       :  5;  /* SOF_IPC4_GLB_NOTIFICATION = 27  */
           uint32_t rsp        :  1;  /* 0 for notifications             */
           uint32_t msg_tgt    :  1;  /* 0 = FW_GEN_MSG                  */
           uint32_t _reserved  :  1;
       } r;
   };

   enum sof_ipc4_notification_type {
       SOF_IPC4_NOTIFY_PHRASE_DETECTED    =  4,  /* keyword detected     */
       SOF_IPC4_NOTIFY_RESOURCE_EVENT     =  5,  /* XRUN / budget breach */
       SOF_IPC4_NOTIFY_LOG_BUFFER_STATUS  =  6,  /* trace buffer ready   */
       SOF_IPC4_NOTIFY_TIMESTAMP_CAPTURED =  7,
       SOF_IPC4_NOTIFY_FW_READY           =  8,  /* boot complete        */
       SOF_IPC4_EXCEPTION_CAUGHT          = 10,  /* DSP panic            */
       SOF_IPC4_MODULE_NOTIFICATION       = 12,  /* user module event    */
       SOF_IPC4_PROBE_DATA_AVAILABLE      = 14,
       SOF_IPC4_WATCHDOG_TIMEOUT          = 15,
   };

IPC4 Error Status Codes
~~~~~~~~~~~~~~~~~~~~~~~~

Unlike IPC3 which reuses Linux ``errno`` values, IPC4 defines its own
24-bit status namespace (``src/include/ipc4/error_status.h``):

.. code-block:: c

   enum ipc4_status {
       IPC4_SUCCESS                      =   0,
       IPC4_ERROR_INVALID_PARAM          =   1,
       IPC4_UNKNOWN_MESSAGE_TYPE         =   2,
       IPC4_OUT_OF_MEMORY                =   3,
       IPC4_BUSY                         =   4,
       IPC4_BAD_STATE                    =   5,
       IPC4_FAILURE                      =   6,
       IPC4_INVALID_REQUEST              =   7,
       IPC4_INVALID_RESOURCE_ID          =   9,
       IPC4_RESOURCE_ID_EXISTS           =  10,
       IPC4_INVALID_RESOURCE_STATE       =  12,
       IPC4_POWER_TRANSITION_FAILED      =  13,
       IPC4_INVALID_MANIFEST             =  14,
       IPC4_UNAVAILABLE                  =  15,
       IPC4_MOD_MGMT_ERROR               = 100,
       IPC4_MOD_NOT_INITIALIZED          = 104,
       IPC4_MOD_INVALID_ID               = 110,
       IPC4_QUEUE_INVALID_ID             = 112,
       IPC4_GATEWAY_NOT_INITIALIZED      = 140,
       IPC4_PIPELINE_STATE_NOT_SET       = 164,
   };

IPC4 Message Dispatch
~~~~~~~~~~~~~~~~~~~~~~

The DSP reads only the 8-byte header (two 32-bit words) from the header
slot, then dispatches purely on bit fields -- no full mailbox read is
needed to route the message:

.. code-block:: text

   Host writes IPC4 header to header slot + MU doorbell IRQ
          |
          v
   ipc_platform_compact_read_msg()    reads 8 bytes from MAILBOX_HOSTBOX_HDR_BASE
          |
          v
   ipc_cmd(hdr)                       src/ipc/ipc4/handler-kernel.c
          |
          +-- primary.r.msg_tgt == 0 (FW_GEN_MSG)
          |       |
          |       v
          |   ipc4_process_glb_message(in)
          |       +-- LOAD_LIBRARY         --> ipc4_load_library()
          |       +-- (all others)         --> ipc4_user_process_glb_message()
          |               +-- CREATE_PIPELINE      --> ipc4_new_pipeline()
          |               +-- DELETE_PIPELINE      --> ipc4_delete_pipeline()
          |               +-- SET_PIPELINE_STATE   --> ipc4_set_pipeline_state()
          |               +-- CHAIN_DMA            --> ipc4_process_chain_dma()
          |               +-- IPCGATEWAY_CMD        --> ipc4_process_ipcgtw_cmd()
          |
          +-- primary.r.msg_tgt == 1 (MODULE_MSG)
                  |
                  v
              ipc4_process_module_message(in)
                  +-- MOD_SET_D0IX          --> ipc4_module_process_d0ix()
                  +-- MOD_SET_DX            --> ipc4_module_process_dx()
                  +-- (all others)          --> ipc4_user_process_module_message()
                          +-- INIT_INSTANCE        --> ipc4_init_module_instance()
                          +-- DELETE_INSTANCE      --> ipc4_delete_module_instance()
                          +-- BIND                 --> ipc4_bind_module_instance()
                          +-- UNBIND               --> ipc4_unbind_module_instance()
                          +-- LARGE_CONFIG_GET     --> ipc4_get_large_config_module_instance()
                          +-- LARGE_CONFIG_SET     --> ipc4_set_large_config_module_instance()
                          +-- CONFIG_GET/SET       --> ipc4_set_get_config_module_instance()

After dispatch, ``ipc_cmd()`` assembles a two-word reply and writes it
back to the outbox header slot before asserting the reply doorbell:

.. code-block:: c

   reply.primary.r.rsp     = SOF_IPC4_MESSAGE_DIR_MSG_REPLY;  /* bit 29 = 1  */
   reply.primary.r.msg_tgt = in->primary.r.msg_tgt;            /* mirrored    */
   reply.primary.r.type    = in->primary.r.type;               /* mirrored    */
   reply.primary.r.status  = err;                              /* 24-bit code */

IPC4 Pipeline State Machine
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The ``SET_PIPELINE_STATE`` command drives the following state transitions:

.. code-block:: text

   IPC4 target state    Internal SOF trigger
   -------------------------------------------------------
   RUNNING              COMP_TRIGGER_PRE_START  (from RESET)
                        COMP_TRIGGER_PRE_RELEASE (from PAUSED)
   PAUSED               COMP_TRIGGER_PAUSE
   RESET                COMP_TRIGGER_STOP -> pipeline_reset()
   EOS                  sets pipeline->expect_eos (no trigger)

Why i.MX Needs Special Handling
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Intel platforms carry the IPC4 primary and extension words in dedicated MU
data registers (``DIPCTDR`` / ``DIPCTDD``) that can be read atomically by
the DSP interrupt handler.  The i.MX MU has no such data registers wired
for SOF.  Instead, SOF on i.MX reserves an 8-byte *header slot*
immediately preceding each mailbox payload box:

::

   SDRAM1 + 0x000  [8 bytes]  IPC4 outbox header slot  <- DSP writes here
   SDRAM1 + 0x008  [payload]  MAILBOX_DSPBOX_BASE
   SDRAM1 + 0x1000 [8 bytes]  IPC4 inbox  header slot  <- host writes here
   SDRAM1 + 0x1008 [payload]  MAILBOX_HOSTBOX_BASE

The ``SRAM_IPC4_HDR_SIZE = 8`` constant controls this shift.  For IPC3
it is defined as 0 so the layout is unchanged.

Boot Sequence (IPC4)
~~~~~~~~~~~~~~~~~~~~~

1. Host loads ``sof-imx8m.ri`` (with IPC4 extended manifest prefix) and
   releases the DSP reset.
2. DSP initialises and calls ``platform_boot_complete()``.
3. ``platform_boot_complete()`` builds a compact ``SOF_IPC4_FW_READY``
   header via ``ipc_boot_complete_msg()``, writes it to the outbox header
   slot, then asserts MU GIR1 -- all through the normal
   ``ipc_platform_send_msg()`` path.
4. Host reads the 8-byte header from the header slot to confirm firmware
   is ready.

Topology (IPC4 -- topology2)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Topology is described using the ALSA topology2 ``conf`` format processed
by ``alsatplg``.  Classes (``Class.Pipeline``, ``Class.Widget``,
``Class.Dai``) are defined once in include files and instantiated with
named parameters.  A single ``.conf`` source can produce many topology
binary variants by varying preprocessor defines at build time.

Firmware Image Format (IPC4)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The IPC4 image uses rimage ``version = [1, 1]`` which prepends an
*unsigned IPC4 extended manifest* block before the ELF-derived binary.
The extended manifest carries ABI version, memory zone layout, and other
metadata that the host driver reads before loading.  The host driver uses
``manifest_fw_hdr_offset = 0`` (no Intel CAVS padding) to locate the
start of the manifest.

----

IPC3 vs IPC4 Comparison
-------------------------

.. list-table::
   :header-rows: 1
   :widths: 20 40 40

   * - Dimension
     - IPC3
     - IPC4
   * - **Header format**
     - ``{uint32_t size; uint32_t cmd;}`` always in mailbox -- full mailbox
       read required before dispatch.
     - Two HW IPC registers (primary + extension) -- header decoded from
       8-byte slot, no mailbox body read needed to route the message.
   * - **Message routing**
     - Single dimension: 4-bit global type, then 12-bit command subtype.
     - Two dimensions: 1-bit target (FW_GEN vs MODULE), then 5-bit type
       within that target.
   * - **Target addressing**
     - Flat 32-bit component ID.
     - ``(module_id[15:0], instance_id[7:0])`` composite address.
   * - **Topology model**
     - Static -- host describes the full graph upfront (COMP_NEW / PIPE_NEW
       / COMP_CONNECT) before any streaming.
     - Dynamic -- CREATE_PIPELINE allocates an empty container; modules
       are instantiated (INIT_INSTANCE) and connected (BIND) individually.
   * - **Pipeline lifecycle**
     - Separate prepare / params / trigger messages. No unified state
       machine.
     - Single ``SET_PIPELINE_STATE`` drives:
       RESET -> PAUSED -> RUNNING -> EOS -> RESET.
   * - **Config parameters**
     - Fixed-size structs per component type (e.g. ``sof_ipc_comp_volume``)
       exchanged as full mailbox payloads.
     - Small params in header extension (CONFIG_GET/SET). Large/fragmented
       params use LARGE_CONFIG with ``large_param_id``, ``init_block``,
       ``final_block``, ``data_off_size`` for multi-fragment TLV transfer.
   * - **Module loading**
     - Not supported -- all components statically compiled.
     - ``LOAD_LIBRARY`` / ``LOAD_MULTIPLE_MODULES`` -- dynamic LLEXT
       library loading at runtime.
   * - **Pin routing**
     - Implicit single input/output per component.
     - Explicit 3-bit pin IDs (``src_queue``, ``dst_queue``) in BIND.
   * - **Power management**
     - ``SOF_IPC_PM_CORE_ENABLE``, ``SOF_IPC_PM_CTX_SAVE/RESTORE`` as
       separate global messages.
     - ``MOD_SET_DX`` (D0/D3), ``MOD_SET_D0IX`` (D0ix sub-state), encoded
       as module messages targeting ``module_id=0`` (BaseFW).
   * - **Reply encoding**
     - ``sof_ipc_reply.error`` = negative Linux errno.
     - ``ipc4_message_reply.primary.r.status`` = 24-bit ``ipc4_status``
       enum; ``type`` and ``msg_tgt`` mirrored from request.
   * - **Notifications**
     - ``SOF_IPC_FW_READY``, trace, XRUN via ``GLB_STREAM_MSG`` or
       ``GLB_TRACE_MSG``.
     - ``SOF_IPC4_GLB_NOTIFICATION`` with 8-bit ``notif_type`` field.
       Host controls which notifications are enabled per type.
   * - **Max message size**
     - 384 bytes (bare metal) / 0x2000 bytes (static lib).
     - 0x1000 bytes (4 KB).

----

Changes Made
-------------

This section documents every code change made to add IPC4 support for
the i.MX8M platform.

1. Platform Memory Map (``src/platform/imx8m/include/platform/lib/memory.h``)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**What changed:**

Added the ``SRAM_IPC4_HDR_SIZE`` constant and shifted the outbox/inbox
base addresses by 8 bytes when ``CONFIG_IPC_MAJOR_4`` is selected:

.. code-block:: c

   #if CONFIG_IPC_MAJOR_4
   #define SRAM_IPC4_HDR_SIZE  8
   #else
   #define SRAM_IPC4_HDR_SIZE  0
   #endif

   /* DSP -> host (outbox) */
   #define SRAM_OUTBOX_HDR_BASE  SDRAM1_BASE                        /* header slot */
   #define SRAM_OUTBOX_BASE      (SDRAM1_BASE + SRAM_IPC4_HDR_SIZE) /* payload */

   /* host -> DSP (inbox) */
   #define SRAM_INBOX_HDR_BASE   (SDRAM1_BASE + 0x1000)
   #define SRAM_INBOX_BASE       (SDRAM1_BASE + 0x1000 + SRAM_IPC4_HDR_SIZE)

Also added ``SRAM_SW_REG_BASE`` / ``SRAM_SW_REG_SIZE`` (2 KB) for the
IPC4 software registers window used by the copier component to publish
stream position (LLP -- Link Layer Position).

**Why:** IPC4 requires a dedicated 8-byte slot for the compact header
because the i.MX MU does not expose data registers.  The SW registers
window is mandatory for IPC4 copier position reporting.

2. Mailbox Header (``src/platform/imx8m/include/platform/lib/mailbox.h``)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**What changed:**

Exposed the header slot base addresses:

.. code-block:: c

   #define MAILBOX_DSPBOX_HDR_BASE   SRAM_OUTBOX_HDR_BASE
   #define MAILBOX_HOSTBOX_HDR_BASE  SRAM_INBOX_HDR_BASE

   #define MAILBOX_SW_REG_BASE       SRAM_SW_REG_BASE
   #define MAILBOX_SW_REG_SIZE       SRAM_SW_REG_SIZE

Added four inline helpers for the IPC4 SW registers window:
``mailbox_sw_reg_write64()``, ``mailbox_sw_reg_read()``,
``mailbox_sw_reg_read64()``, ``mailbox_sw_regs_write()``.  These
perform cache maintenance (writeback/invalidate) so that the ARM host
sees consistent position data.

**Why:** The generic IPC4 copier component calls these helpers to report
DMA link-layer position to the host.

3. Platform Boot (``src/platform/imx8m/platform.c``)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**What changed:**

``platform_boot_complete()`` now has an IPC4 branch:

.. code-block:: c

   #if CONFIG_IPC_MAJOR_4
       struct ipc_cmd_hdr header;
       ipc_boot_complete_msg(&header, 0);
       header.pri |= boot_message;

       struct ipc_msg msg = {
           .header    = header.pri,
           .extension = header.ext,
           .tx_size   = 0,
       };
       return ipc_platform_send_msg(&msg);
   #else
       mailbox_dspbox_write(0, &ready, sizeof(ready));
       imx_mu_xcr_rmw(..., IMX_MU_xCR_GIRn(IMX_MU_VERSION, 1), 0);
       return 0;
   #endif

The legacy ``sof_ipc_fw_ready`` struct and ``ext_man_windows`` table
are compiled out when ``CONFIG_IPC_MAJOR_4=y``.

**Why:** IPC4 does not use the ``fw_ready`` struct.  The boot-complete
notification is sent as a regular IPC4 message through the standard send
path, which takes care of writing the compact header to the header slot
and ringing the MU doorbell.

4. IPC Platform Driver (``src/drivers/imx/ipc.c``)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**What changed:**

``ipc_platform_compact_read_msg()`` reads the compact IPC4 header from
the inbox header slot:

.. code-block:: c

   #if CONFIG_IPC_MAJOR_4
       dcache_invalidate_region(MAILBOX_HOSTBOX_HDR_BASE, 8);
       dst[0] = ((volatile uint32_t *)MAILBOX_HOSTBOX_HDR_BASE)[0];
       dst[1] = ((volatile uint32_t *)MAILBOX_HOSTBOX_HDR_BASE)[1];
       return 2;
   #endif

``ipc_platform_do_cmd()`` uses ``ipc_compact_read_msg()`` for IPC4
instead of ``mailbox_validate()``:

.. code-block:: c

   #if CONFIG_IPC_MAJOR_4
       hdr = ipc_compact_read_msg();
   #else
       hdr = mailbox_validate();
   #endif

``ipc_platform_send_msg()`` writes the compact header to the outbox
header slot for IPC4:

.. code-block:: c

   #if CONFIG_IPC_MAJOR_4
       hdr[0] = msg->header;
       hdr[1] = msg->extension;
       dcache_writeback_region(MAILBOX_DSPBOX_HDR_BASE, 8);
       if (msg->tx_size)
           mailbox_dspbox_write(0, msg->tx_data, msg->tx_size);
   #else
       mailbox_dspbox_write(0, msg->tx_data, msg->tx_size);
   #endif

**Why:** The IPC4 generic layer calls ``ipc_platform_compact_read_msg()``
to retrieve the two header words.  Without this, the DSP would attempt to
decode an empty or garbage header.  Similarly, outgoing messages must
place the header in the slot before ringing the doorbell.

5. Board Configuration (``app/boards/imx8mp_evk_mimx8ml8_adsp_ipc4.conf``)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

A new Kconfig overlay selects the IPC4 firmware variant:

.. code-block:: kconfig

   CONFIG_IPC_MAJOR_4=y
   CONFIG_CROSS_CORE_STREAM=n   # HiFi4 is single-core
   CONFIG_TRACE=n
   CONFIG_DMA_NXP_SDMA=y
   CONFIG_DAI_NXP_MICFIL=y
   CONFIG_ZEPHYR_NATIVE_DRIVERS=y

**Why:** The same Zephyr board target (``imx8mp_evk/mimx8ml8/adsp``) is
shared between IPC3 and IPC4 builds.  A separate ``.conf`` file is used
to activate the IPC4 Kconfig options without modifying the base board
configuration.

6. Rimage Configuration (``tools/rimage/config/imx8m_ipc4.toml``)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

A new rimage config for the IPC4 image format:

.. code-block:: toml

   version = [1, 1]   # simple write + unsigned IPC4 extended manifest prefix

   [adsp]
   name = "imx8m"

   [[adsp.mem_zone]]
   type = "IRAM"
   base = "0x3b6F8000"
   size = "0x800"
   host_offset = "0x10000"

   [[adsp.mem_zone]]
   type = "SRAM"
   base = "0x92400000"
   size = "0x800000"
   host_offset = "0x0"

``version = [1, 1]`` instructs rimage to prepend an unsigned IPC4
extended manifest.  The Linux host driver (``imx_ipc4_init_data()``) sets
``manifest_fw_hdr_offset = 0`` so it finds the manifest at the very start
of the file.

7. Build System (``scripts/xtensa-build-zephyr.py``)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Three modifications to the build script:

**a) ``extra_conf`` field in ``PlatformConfig``:**

.. code-block:: python

   @dataclasses.dataclass
   class PlatformConfig:
       ...
       extra_conf: pathlib.Path = None

Allows a platform entry to specify an additional Kconfig overlay without
requiring a separate Zephyr board target.

**b) ``imx8m`` is now the IPC4 default:**

.. code-block:: python

   "imx8m" : PlatformConfig(
       "imx", "imx8mp_evk/mimx8ml8/adsp",
       f"RI-2023.11{xtensa_tools_version_postfix}",
       "hifi4_mscale_v2_0_2_prod",
       RIMAGE_KEY = "key param ignored by imx8m",
       ipc4      = True,
       extra_conf = pathlib.Path(SOF_TOP, "app", "boards",
                                 "imx8mp_evk_mimx8ml8_adsp_ipc4.conf")
   ),

``ipc4 = True`` causes the output to be placed under
``build-sof-staging/sof/imx/sof-ipc4/imx8m/`` (matching the Linux
driver's default firmware path ``imx/sof-ipc4/sof-imx8m.ri``).

**c) Deprecated ``imx8m-ipc3`` entry (in ``extra_platform_configs``):**

The old IPC3 build is preserved under the name ``imx8m-ipc3`` in the
``extra_platform_configs`` dict (not included in ``--all``):

.. code-block:: python

   "imx8m-ipc3" : PlatformConfig(
       "imx", "imx8mp_evk/mimx8ml8/adsp",
       ...
       # ipc4 = False (default) -- no extra_conf overlay
   ),

**d) ``extra_conf`` wired into the build command:**

.. code-block:: python

   if platform_dict.get("extra_conf"):
       extra_conf_files.append(str(platform_dict["extra_conf"]))

**Build commands:**

.. code-block:: shell

   # IPC4 firmware (default)
   source ~/work/venv/bin/activate
   python scripts/xtensa-build-zephyr.py imx8m

   # IPC3 firmware (deprecated, still buildable)
   python scripts/xtensa-build-zephyr.py imx8m-ipc3

**Output locations:**

.. code-block:: text

   build-sof-staging/sof/imx/sof-ipc4/imx8m/community/sof-imx8m.ri   (IPC4)
   build-sof-staging/sof/imx/sof/community/sof-imx8m-ipc3.ri          (IPC3)

8. Topology2 for IPC4 (``tools/topology/topology2/``)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Two topology2 (ALSA conf format) files are provided for i.MX8MP + WM8960.

**a) Playback-only (``imx8-wm8960.conf``):**

Single pipeline: ``host-copier -> gain -> dai-copier (SAI)``.

.. code-block:: text

   host-copier.0.playback  ->  gain.1.1  ->  dai-copier.SAI.sai3-wm8960-hifi.playback

Built as ``sof-imx8mp-wm8960.tplg``.

**b) Duplex -- playback + capture (``imx8-wm8960-duplex.conf``):**

Two pipelines sharing the same SAI DAI index:

.. code-block:: text

   Playback:
   host-copier.0.playback  ->  gain.1.1  ->  dai-copier.SAI.sai3-wm8960-hifi.playback

   Capture:
   dai-copier.SAI.sai3-wm8960-hifi.capture  ->  gain.2.1  ->  host-copier.0.capture

Built as ``sof-imx8mp-wm8960-duplex.tplg``.

Both topologies are parameterised via preprocessor defines at CMake build
time:

.. code-block:: cmake

   # tplg-targets-imx8.cmake
   "imx8-wm8960\;sof-imx8mp-wm8960\;SAI_DAI_INDEX=3,STREAM_CODEC_NAME=sai3-wm8960-hifi"
   "imx8-wm8960-duplex\;sof-imx8mp-wm8960-duplex\;SAI_DAI_INDEX=3,STREAM_CODEC_NAME=sai3-wm8960-hifi"

The ``SAI_DAI_INDEX`` and ``STREAM_CODEC_NAME`` variables can be changed
to target a different SAI instance or codec without modifying the source
``.conf`` file.

----

How the Pieces Fit Together at Runtime
---------------------------------------

The following sequence shows the full IPC4 data path for a playback stream
on i.MX8MP, from Linux userspace to the codec:

.. code-block:: text

   aplay (userspace)
     |
     | ALSA PCM write
     v
   Linux ASoC / SOF driver  (sound/soc/sof/imx/imx-common.c)
     |  sof_mailbox_write()  -- writes IPC4 compact header to inbox header slot
     |  imx_dsp_ring_doorbell() -- asserts MU GIR0
     v
   MU interrupt -> DSP
     |
     | ipc_platform_compact_read_msg()
     |   reads 8 bytes from MAILBOX_HOSTBOX_HDR_BASE
     v
   IPC4 generic layer  (src/ipc/ipc4/handler-kernel.c)
     | dispatches on msg_tgt / type bits
     | MODULE_MSG / PIPELINE_MSG handled without any mailbox body read
     v
   Pipeline scheduler (Zephyr LL domain, DMA timer)
     |
     | host-copier  -- DMA from SDRAM0 host buffer
     | gain         -- volume adjustment (IPC4 module)
     | dai-copier   -- DMA to SAI TX FIFO via SDMA3
     v
   SAI3 peripheral -> WM8960 codec -> headphone / speaker

On reply, the DSP writes the compact response header to
``MAILBOX_DSPBOX_HDR_BASE`` and asserts MU GIR1.  The Linux driver's ISR
reads the header slot, identifies the reply, and wakes the waiting
userspace thread.

----

File Map
---------

.. list-table::
   :header-rows: 1
   :widths: 55 45

   * - File
     - Purpose
   * - ``src/include/ipc/header.h``
     - IPC3 full protocol: global types, command types, ``sof_ipc_cmd_hdr``
   * - ``src/include/ipc4/header.h``
     - IPC4 core header: ``ipc4_message_request``, ``ipc4_message_reply``,
       global message type enum
   * - ``src/include/ipc4/module.h``
     - IPC4 module message structs: init, bind/unbind, large config, delete
   * - ``src/include/ipc4/pipeline.h``
     - IPC4 pipeline lifecycle structs and state enum
   * - ``src/include/ipc4/notification.h``
     - IPC4 DSP->host notification header and type enum
   * - ``src/include/ipc4/error_status.h``
     - IPC4 status code enum
   * - ``src/ipc/ipc3/handler.c``
     - IPC3 dispatcher: ``ipc_cmd()``, all IPC3 command handlers
   * - ``src/ipc/ipc4/handler-kernel.c``
     - IPC4 top-level dispatch, power management, compound message wait
   * - ``src/ipc/ipc4/handler-user.c``
     - IPC4 pipeline/module commands, pipeline state machine, bind/unbind
   * - ``src/platform/imx8m/include/platform/lib/memory.h``
     - Memory map; IPC4 header slot shift; SW regs window
   * - ``src/platform/imx8m/include/platform/lib/mailbox.h``
     - Mailbox base macros; SW regs read/write helpers
   * - ``src/platform/imx8m/platform.c``
     - Platform init; IPC4/IPC3 boot-complete branching
   * - ``src/drivers/imx/ipc.c``
     - MU IRQ handler; compact header read/write; IPC4 send
   * - ``app/boards/imx8mp_evk_mimx8ml8_adsp_ipc4.conf``
     - Kconfig overlay enabling ``CONFIG_IPC_MAJOR_4=y``
   * - ``tools/rimage/config/imx8m_ipc4.toml``
     - rimage config for IPC4 extended manifest image format
   * - ``scripts/xtensa-build-zephyr.py``
     - ``imx8m`` IPC4 default; ``imx8m-ipc3`` deprecated entry
   * - ``tools/topology/topology2/imx8-wm8960.conf``
     - IPC4 topology2: i.MX8MP + WM8960, playback only
   * - ``tools/topology/topology2/imx8-wm8960-duplex.conf``
     - IPC4 topology2: i.MX8MP + WM8960, duplex
   * - ``tools/topology/topology2/production/tplg-targets-imx8.cmake``
     - CMake build targets for both topology2 files

----

References
----------

- `SOF project <https://thesofproject.github.io/>`_
- `ALSA topology2 documentation <https://www.alsa-project.org/alsa-doc/alsa-lib/group__topology.html>`_
- i.MX 8M Plus Applications Processor Reference Manual (NXP document IMX8MPRM)
- `Linux SOF driver (sound/soc/sof/imx/) <https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/sound/soc/sof/imx>`_
