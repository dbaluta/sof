/* SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright(c) 2020 Intel Corporation. All rights reserved.
 *
 * Author: Marcin Rajwa <marcin.rajwa@linux.intel.com>
 */

#ifndef __SOF_POST_PROCESS__
#define __SOF_POST_PROCESS__
#include <sof/audio/post_process/pp_common.h>

#define PP_SETUP_CONFIG		0
#define PP_RUNTIME_PARAMS	1

enum pp_state {
	PP_STATE_DISABLED = 0,
	PP_STATE_CREATED,
	PP_STATE_PREPARED,
	PP_STATE_RUN,
};

/* Post_process private, runtime data */
struct comp_data {
	enum pp_state state;
	struct post_process_config pp_config;
	struct post_process_shared_data sdata;
	struct comp_buffer *pp_sink;
	struct comp_buffer *pp_source;
	bool lib_r_cfg_avail;
	void *pp_lib_runtime_config;
	xa_codec_func_t *pp_proc_func;
	int lib_init;
};

static  inline int validate_config(struct post_process_config *cfg)
{
	//TODO: custom validation of post processing paramters
	return 0;
}


#endif /*__SOF_POST_PROCESS__ */
