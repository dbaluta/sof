// SPDX-License-Identifier: BSD-3-Clause
//
// Copyright 2025 NXP
//
// i.MX IPC4 extended manifest (v4) generator.
//
// This produces a minimal, unsigned SOF_EXT_MAN4 ("$AE1") manifest in the
// separate <output>.xman file (rimage -e / write_firmware_ext_man hook, same
// contract as the IPC3 ext_man_write()). The build's signing step ("west
// sign") prepends the .xman blob to the plain "Reef" payload written by
// simple_write_firmware(), yielding a single [ext-manifest v4][Reef payload]
// image that the i.MX SOF driver loads with its IPC4 memcpy loader. No
// CSS/CSE partitions or signing are emitted.

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

#include <rimage/ext_manifest_gen.h>
#include <rimage/rimage.h>
#include <rimage/manifest.h>
#include <rimage/elf_file.h>
#include <rimage/file_utils.h>
#include <rimage/sof/user/manifest.h>

/* In ASCII "$AE1" - must match SOF_EXT_MAN4_MAGIC_NUMBER in the kernel */
#define IMX_EXT_MAN4_MAGIC_NUMBER	0x31454124

/*
 * Reserved gap between the extended manifest header and the fw binary
 * header. Must match the SOF driver's ipc4_data->manifest_fw_hdr_offset for
 * i.MX (the generic kernel parser rejects an offset of zero).
 */
#define IMX_EXT_MAN4_FW_HDR_OFFSET	48

#define IMX_MAX_MODULE_NAME_LEN		8
#define IMX_MAX_FW_BINARY_NAME		8
#define IMX_HASH_SHA256_LEN		32

/* The i.MX IPC4 image starts with the extended manifest, so the FW binary
 * header sits immediately after it (manifest_fw_hdr_offset == 0 in the driver).
 */

struct imx_ext_man4_hdr {
	uint32_t id;			/* IMX_EXT_MAN4_MAGIC_NUMBER */
	uint32_t len;			/* length of the extended manifest */
	uint16_t version_major;
	uint16_t version_minor;
	uint32_t num_module_entries;
} __attribute__((packed));

struct imx_man4_segment_desc {
	uint32_t flags;
	uint32_t v_base_addr;
	uint32_t file_offset;
} __attribute__((packed));

struct imx_man4_fw_binary_header {
	uint32_t id;
	uint32_t len;			/* sizeof(this struct) */
	uint8_t name[IMX_MAX_FW_BINARY_NAME];
	uint32_t preload_page_count;
	uint32_t fw_image_flags;
	uint32_t feature_mask;
	uint16_t major_version;
	uint16_t minor_version;
	uint16_t hotfix_version;
	uint16_t build_version;
	uint32_t num_module_entries;
	uint32_t hw_buf_base_addr;
	uint32_t hw_buf_length;
	uint32_t load_offset;
} __attribute__((packed));

struct imx_man4_module {
	uint32_t id;
	uint8_t name[IMX_MAX_MODULE_NAME_LEN];
	uint8_t uuid[16];
	uint32_t type;
	uint8_t hash[IMX_HASH_SHA256_LEN];
	uint32_t entry_point;
	uint16_t cfg_offset;
	uint16_t cfg_count;
	uint32_t affinity_mask;
	uint16_t instance_max_count;
	uint16_t instance_stack_size;
	struct imx_man4_segment_desc segments[3];
} __attribute__((packed));

/* i.MX firmware binary header id, matching the kernel's expectation */
#define IMX_MAN4_FW_HDR_ID	{'$', 'A', 'M', '1'}
#define IMX_MAN4_FW_HDR_NAME	"ADSPFW"

/*
 * Convert a struct sof_man_uuid (from the ELF ".module" section) into the raw
 * 16-byte little-endian GUID layout that the kernel's guid_t expects.
 */
static void imx_uuid_to_guid(const struct sof_man_uuid *src, uint8_t dst[16])
{
	dst[0] = src->a & 0xff;
	dst[1] = (src->a >> 8) & 0xff;
	dst[2] = (src->a >> 16) & 0xff;
	dst[3] = (src->a >> 24) & 0xff;
	dst[4] = src->b & 0xff;
	dst[5] = (src->b >> 8) & 0xff;
	dst[6] = src->c & 0xff;
	dst[7] = (src->c >> 8) & 0xff;
	memcpy(&dst[8], src->d, 8);
}

static int imx_ext_man_open_file(struct image *image)
{
	int ret;

	ret = create_file_name(image->out_ext_man_file,
			       sizeof(image->out_ext_man_file),
			       image->out_file, "xman");
	if (ret)
		return ret;

	image->out_ext_man_fd = fopen(image->out_ext_man_file, "wb");
	if (!image->out_ext_man_fd)
		return file_error("unable to open file for writing",
				  image->out_ext_man_file);

	return 0;
}

int ext_man_write_imx_ipc4(struct image *image)
{
	const struct sof_man_module_manifest *sof_mod;
	const struct manifest_module *module = NULL;
	const uint8_t fw_hdr_gap[IMX_EXT_MAN4_FW_HDR_OFFSET] = { 0 };
	struct imx_man4_fw_binary_header fw_hdr;
	struct imx_ext_man4_hdr ext_hdr;
	struct imx_man4_module *mods = NULL;
	struct elf_section section = { 0 };
	unsigned int n_mod, i;
	uint32_t ext_man_len;
	size_t count;
	int ret;

	/* find the (single) base firmware module */
	for (i = 0; i < (unsigned int)image->num_modules; i++) {
		if (!image->module[i].is_bootloader) {
			module = &image->module[i];
			break;
		}
	}

	if (!module) {
		fprintf(stderr, "error: no base firmware module for IPC4 manifest\n");
		return -EINVAL;
	}

	ret = elf_section_read_by_name(&module->file.elf, ".module", &section);
	if (ret) {
		fprintf(stderr, "error: can't read '.module' section for IPC4 manifest\n");
		return ret;
	}

	n_mod = section.header.data.size / sizeof(*sof_mod);
	if (!n_mod || n_mod * sizeof(*sof_mod) != section.header.data.size) {
		fprintf(stderr, "error: invalid '.module' section for IPC4 manifest\n");
		ret = -EINVAL;
		goto out;
	}

	mods = calloc(n_mod, sizeof(*mods));
	if (!mods) {
		ret = -ENOMEM;
		goto out;
	}

	sof_mod = section.data;
	for (i = 0; i < n_mod; i++, sof_mod++) {
		struct sof_man_uuid uuid;
		uint32_t type;

		mods[i].id = i;
		memcpy(mods[i].name, sof_mod->module.name,
		       sizeof(mods[i].name));

		/* copy out of the packed source before taking its address */
		memcpy(&uuid, &sof_mod->module.uuid, sizeof(uuid));
		imx_uuid_to_guid(&uuid, mods[i].uuid);

		/* the module type is a bitfield struct; take its raw 32 bits */
		memcpy(&type, &sof_mod->module.type, sizeof(type));
		mods[i].type = type;

		memcpy(mods[i].hash, sof_mod->module.hash,
		       sizeof(mods[i].hash));
		mods[i].entry_point = sof_mod->module.entry_point;
		/* i.MX does not use per-module config blocks in the manifest */
		mods[i].cfg_offset = 0;
		mods[i].cfg_count = 0;
		mods[i].affinity_mask = sof_mod->module.affinity_mask;
		mods[i].instance_max_count = sof_mod->module.instance_max_count;
	}

	/*
	 * Extended manifest header. The kernel's generic IPC4 parser locates
	 * the fw binary header at (ext_hdr.len + manifest_fw_hdr_offset), i.e.
	 * PAST the region declared by len, and rejects a zero offset. So len
	 * covers only this header, and the fw binary header follows after a
	 * reserved gap of IMX_EXT_MAN4_FW_HDR_OFFSET bytes (which must match
	 * the driver's ipc4_data->manifest_fw_hdr_offset).
	 */
	memset(&ext_hdr, 0, sizeof(ext_hdr));
	ext_hdr.id = IMX_EXT_MAN4_MAGIC_NUMBER;
	ext_hdr.len = sizeof(ext_hdr);
	ext_hdr.version_major = 4;
	ext_hdr.version_minor = 0;
	ext_hdr.num_module_entries = n_mod;

	/* fw binary header */
	memset(&fw_hdr, 0, sizeof(fw_hdr));
	fw_hdr.id = (uint32_t)('$' | ('A' << 8) | ('M' << 16) | ('1' << 24));
	fw_hdr.len = sizeof(fw_hdr);
	memcpy(fw_hdr.name, IMX_MAN4_FW_HDR_NAME,
	       sizeof(IMX_MAN4_FW_HDR_NAME) - 1);
	fw_hdr.major_version = image->fw_ver_major;
	fw_hdr.minor_version = image->fw_ver_minor;
	fw_hdr.hotfix_version = image->fw_ver_micro;
	fw_hdr.build_version = image->fw_ver_build;
	fw_hdr.num_module_entries = n_mod;

	ext_man_len = sizeof(ext_hdr) + IMX_EXT_MAN4_FW_HDR_OFFSET +
		      sizeof(fw_hdr) + n_mod * sizeof(struct imx_man4_module);

	/*
	 * Write the manifest to the separate .xman file; the signing step
	 * prepends it to the firmware payload.
	 */
	ret = imx_ext_man_open_file(image);
	if (ret)
		goto out;

	count = fwrite(&ext_hdr, sizeof(ext_hdr), 1, image->out_ext_man_fd);
	if (count != 1) {
		ret = file_error("can't write ext_man4 header",
				 image->out_ext_man_file);
		goto out;
	}

	count = fwrite(fw_hdr_gap, sizeof(fw_hdr_gap), 1, image->out_ext_man_fd);
	if (count != 1) {
		ret = file_error("can't write fw header gap",
				 image->out_ext_man_file);
		goto out;
	}

	count = fwrite(&fw_hdr, sizeof(fw_hdr), 1, image->out_ext_man_fd);
	if (count != 1) {
		ret = file_error("can't write fw binary header",
				 image->out_ext_man_file);
		goto out;
	}

	count = fwrite(mods, sizeof(struct imx_man4_module), n_mod,
		       image->out_ext_man_fd);
	if (count != n_mod) {
		ret = file_error("can't write module entries",
				 image->out_ext_man_file);
		goto out;
	}

	fprintf(stdout, "imx: IPC4 extended manifest: %u bytes, %u module(s)\n",
		ext_man_len, n_mod);

	ret = 0;
out:
	if (image->out_ext_man_fd) {
		fclose(image->out_ext_man_fd);
		image->out_ext_man_fd = NULL;
	}
	free(mods);
	elf_section_free(&section);
	return ret;
}
