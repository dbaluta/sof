/* SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright(c) 2025 NXP. All rights reserved.
 */

#ifndef __RIMAGE_IPC4_H__
#define __RIMAGE_IPC4_H__

#include <stdint.h>

/* IPC4 manifest magic numbers */
#define SOF_EXT_MAN4_MAGIC_NUMBER	0x31454124  /* $AE1 in ASCII */
#define SOF_MAN4_FW_HDR_MAGIC		0x314D4124  /* $AM1 in ASCII */

#define MAX_MODULE_NAME_LEN		8
#define MAX_FW_BINARY_NAME		8
#define DEFAULT_HASH_SHA256_LEN		32

/*
 * Extended manifest header for IPC4
 */
struct sof_ext_manifest4_hdr {
	uint32_t id;			/* $AE1 magic */
	uint32_t len;			/* length of extension manifest */
	uint16_t version_major;		/* header version */
	uint16_t version_minor;
	uint32_t num_module_entries;
} __attribute__((packed));

/*
 * IPC4 Firmware binary header
 */
struct sof_man4_fw_binary_header {
	uint32_t id;			/* firmware ID: $AM1 */
	uint32_t len;			/* sizeof(sof_man4_fw_binary_header) */
	uint8_t name[MAX_FW_BINARY_NAME];
	uint32_t preload_page_count;	/* number of pages of preloaded image */
	uint32_t fw_image_flags;
	uint32_t feature_mask;
	uint16_t major_version;		/* Firmware version */
	uint16_t minor_version;
	uint16_t hotfix_version;
	uint16_t build_version;
	uint32_t num_module_entries;
	uint32_t hw_buf_base_addr;
	uint32_t hw_buf_length;
	uint32_t load_offset;
} __attribute__((packed));

/*
 * IPC4 Module segment descriptor
 */
struct sof_man4_segment_desc {
	uint32_t flags;
	uint32_t v_base_addr;
	uint32_t file_offset;
} __attribute__((packed));

/*
 * IPC4 Module entry
 */
struct sof_man4_module {
	uint32_t id;
	uint8_t name[MAX_MODULE_NAME_LEN];
	uint8_t uuid[16];		/* 16-byte UUID */
	uint32_t type;
	uint8_t hash[DEFAULT_HASH_SHA256_LEN];
	uint32_t entry_point;
	uint16_t cfg_offset;
	uint16_t cfg_count;
	uint32_t affinity_mask;
	uint16_t instance_max_count;
	uint16_t instance_stack_size;
	struct sof_man4_segment_desc segments[3];
} __attribute__((packed));

/*
 * IPC4 Module configuration
 */
struct sof_man4_module_config {
	uint32_t par[4];		/* module parameters */
	uint32_t is_bytes;		/* actual size of instance .bss */
	uint32_t cps;			/* cycles per second */
	uint32_t ibs;			/* input buffer size */
	uint32_t obs;			/* output buffer size */
	uint32_t module_flags;		/* flags */
	uint32_t cpc;			/* cycles per single run */
	uint32_t obls;			/* output block size */
} __attribute__((packed));

#endif /* __RIMAGE_IPC4_H__ */
