#include <inttypes.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define __packed __attribute__((packed))

struct snd_sof_fw_header {
	unsigned char sig[4];
	uint32_t file_size;
	uint32_t num_modules;
	uint32_t abi;
} __packed;

enum snd_sof_fw_mod_type {
	SOF_FW_BASE=0,
	SOF_FW_MODULE=1
};

struct snd_sof_mod_hdr {
	enum snd_sof_fw_mod_type type;
	uint32_t size;
	uint32_t num_blocks;
} __packed;

void fail(const char *format, ...) {
	va_list list;

	va_start(list, format);
	vprintf(format, list);
	va_end(list);

	exit(1);
}

void parsermain(FILE *file);

int main(int argc, char **argv) {
	if (argc != 2)
		fail("Usage: %s <filename>\n", argv[0]);
	FILE *f = fopen(argv[1], "rb");
	if (!f)
		fail("Unable to open %s\n", argv[1]);
	parsermain(f);
	fclose(f);
}

void parsermodule(FILE *file, uint32_t mod_index);

void parsermain(FILE *file) {
	struct snd_sof_fw_header hdr;
	int rc;

	rc = fread(&hdr, sizeof(hdr), 1, file);
	if (rc != 1)
		fail("Unable to read header\n");
	if (strncmp("Reef", hdr.sig, 4))
		printf("Wrong header signature, trying to parse anyway\n");
	
	printf("File size: %"PRIu32"\n", hdr.file_size);
	printf("There are %"PRIu32" modules.\n", hdr.num_modules);
	printf("ABI version %"PRIx32".\n", hdr.abi);

	for (uint32_t i = 0; i < hdr.num_modules; i++)
		parsermodule(file, i);

	if (ftell(file) != hdr.file_size)
		printf("Wrong file size; current position is %li\n",
		     ftell(file));
	printf("File finished (trailing data not checked).\n");
}

void parse_block(FILE *file, uint32_t mod_index, uint32_t block_index);

void parsermodule(FILE *file, uint32_t mod_index)
{
	struct snd_sof_mod_hdr hdr;
	long curr_position;
	int rc;

	printf("Parsing module %d...\n", mod_index);
	rc = fread(&hdr, sizeof(hdr), 1, file);
	if (rc != 1)
		fail("Unable to read module header\n");
	switch (hdr.type) {
	case SOF_FW_BASE:
		printf("Beginning a base \"module\"...\n");
		break;
	case SOF_FW_MODULE:
		printf("Beginning an additional module...\n");
		break;
	default:
		fail("Unrecognized module type %d\n", hdr.type);
		break;
	}

	curr_position = ftell(file);

	for (int i = 0; i < hdr.num_blocks; i++)
		parse_block(file, mod_index, i);

	long final_position = ftell(file);
	if (final_position - curr_position != hdr.size)
		fail("Mismatched file size with block sizes in module %d\n",
		     mod_index);
	printf("Finished parsing module %d\n", mod_index);
}

enum snd_sof_fw_blk_type {
	SOF_FW_BLK_TYPE_INVALID = -1,
	SOF_FW_BLK_TYPE_START = 0,
	SOF_FW_BLK_TYPE_IRAM = 1,
	SOF_FW_BLK_TYPE_DRAM = 2,
	SOF_FW_BLK_TYPE_SRAM = 3,
	SOF_FW_BLK_TYPE_RSRVD14 = 14,
	SOF_FW_BLK_TYPE_NUM
};

struct snd_sof_blk_hdr {
	enum snd_sof_fw_blk_type type;
	uint32_t size;
	uint32_t offset;
} __packed;

void parse_block(FILE *file, uint32_t mod_index, uint32_t block_index)
{
	struct snd_sof_blk_hdr hdr;
	char *tag;

	int ret = fread(&hdr, sizeof(hdr), 1, file);
	if (ret != 1)
		fail("Unable to read block header %d:%d\n", mod_index, block_index);
	switch(hdr.type) {
	case SOF_FW_BLK_TYPE_START:
		tag = "START";
		break;
	case SOF_FW_BLK_TYPE_IRAM:
		tag = "IRAM";
		break;
	case SOF_FW_BLK_TYPE_DRAM:
		tag = "DRAM";
		break;
	case SOF_FW_BLK_TYPE_SRAM:
		tag = "SRAM";
		break;
	default:
		tag = NULL;
		break;
	}
	if (tag)
		printf("%d:%d type %s size %"PRIu32" off 0x%"PRIx32"\n",
		       mod_index, block_index, tag, hdr.size, hdr.offset);
	else
		printf("%d:%d type %d size %"PRIu32" off 0x%"PRIx32"\n",
		       mod_index, block_index, hdr.type, hdr.size, hdr.offset);
	// Skip the actual block data
	fseek(file, hdr.size, SEEK_CUR);
}
