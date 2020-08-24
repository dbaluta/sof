/* SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright(c) 2020 Intel Corporation. All rights reserved.
 *
 * Author: Marcin Rajwa <marcin.rajwa@linux.intel.com>
 */

#include <sof/audio/post_process/lib_api.h>

#define CNTX 24

static struct post_process_lib_data pp_lib_data;

static inline void *allocate_lib_obj(size_t obj_size)
{

	return rballoc(0, SOF_MEM_CAPS_RAM, obj_size);
}
#if 0
static int validate_config(void) {
	//TODO:
	return 0;
}
#endif
static void handle_error(struct comp_dev *dev, int code) {
	WORD is_fatal     = XA_ERROR_SEVERITY(code) != 0;
	WORD err_class    = XA_ERROR_CLASS(code);
	WORD err_sub_code = XA_ERROR_SUBCODE(code);

	if (code == XA_NO_ERROR)
		return;
	comp_err(dev, "handle_eror is_fatal (%d) err_class %d err_sub_code %d",
		 is_fatal, err_class, err_sub_code);
}

int pp_set_proc_func(struct comp_dev *dev, int type) {
	struct comp_data *cd = comp_get_drvdata(dev);

	cd->pp_proc_func = xa_mp3_dec;

	return 0;
}

int pp_init_lib(struct comp_dev *dev) {
	int ret;


///static struct post_process_lib_data pp_lib_data;

	size_t lib_obj_size;
///	struct comp_data *cd = comp_get_drvdata(dev);

	comp_err(dev, "pp_init_lib() start");
	
	pp_lib_data.p_xa_process_api = xa_mp3_dec;

	ret = PP_LIB_API_CALL(pp_lib_data,
			      XA_API_CMD_GET_LIB_ID_STRINGS,
			      XA_CMD_TYPE_LIB_NAME,
			      pp_lib_data.name);
	if (ret != LIB_NO_ERROR) {
		comp_err(dev, "pp_init_lib() error %x: failed to get lib name",
			 ret);
		goto out;
	}

	comp_err(dev, "pp_init_lib() name %x %x", (int)pp_lib_data.name[0],  (int)pp_lib_data.name[1]);

	ret = PP_LIB_API_CALL(pp_lib_data, XA_API_CMD_GET_API_SIZE, 0, &lib_obj_size);
	if (ret != LIB_NO_ERROR) {
		comp_err(dev, "pp_init_lib() error %x: failed to get lib object size",
			 ret);
		goto out;
	}

	comp_err(dev, "pp_init_lib()size %d ", lib_obj_size);
	pp_lib_data.xa_process_handle = allocate_lib_obj(lib_obj_size);
	if (!pp_lib_data.xa_process_handle) {
		comp_err(dev, "pp_init_lib() error: failed to allocate space for lib object");
		goto out;
	} else {
		comp_dbg(dev, "pp_init_lib(): allocated space for lib object");
	}

	ret = PP_LIB_API_CALL(pp_lib_data, XA_API_CMD_INIT,
			      XA_CMD_TYPE_INIT_API_PRE_CONFIG_PARAMS,
			      NULL);
	if (ret != LIB_NO_ERROR) {
		comp_err(dev, "pp_init_lib(): error %x: failed to set default config",
			     ret);
		goto out;
	}
out:
	return ret;
}

int pp_lib_set_config(struct comp_dev *dev, void *cfg) {
	int ret;

	comp_dbg(dev, "pp_set_config() start");

	if (!cfg)
		comp_err(dev, "pp_set_config() error: NULL config passed!");

	ret = memcpy_s(&pp_lib_data.s_cfg, sizeof(pp_lib_data.s_cfg), cfg,
		       sizeof(struct post_process_setup_config));

	assert(!ret);
#if 0
	ret = validate_config();
	if (ret) {
		comp_err(dev, "pp_set_config() error: validation of config failed");
	}
#endif
	return ret;
}

static inline void *allocate_lib_memory(size_t size, size_t alignment) {

	return rballoc_align(0, SOF_MEM_CAPS_RAM, size, alignment);
}

static int init_memory_tables(struct comp_dev *dev) {
	int ret, no_mem_tables, i, mem_type, mem_size, mem_alignment;
	void *ptr;

	/* calculate the size of all memory blocks required */
	ret = PP_LIB_API_CALL(pp_lib_data, XA_API_CMD_INIT,
			      XA_CMD_TYPE_INIT_API_POST_CONFIG_PARAMS,
			      NULL);
	if (ret != LIB_NO_ERROR) {
		comp_err(dev, "init_memory_tables() error %x: failed to calculate memory blocks size",
			 ret);
		goto err;
	}

	/* Get number of memory tables */
	ret = PP_LIB_API_CALL(pp_lib_data, XA_API_CMD_GET_N_MEMTABS, 0, &no_mem_tables);
	if (ret != LIB_NO_ERROR) {
		comp_err(dev, "init_memory_tables() error %x: failed to get number of memory tables",
			 ret);
		goto err;
	}

	/* Initialize each memory table */
	for (i = 0; i < no_mem_tables; i++) {
		ret = PP_LIB_API_CALL(pp_lib_data, XA_API_CMD_GET_MEM_INFO_TYPE, i,
				      &mem_type);
		if (ret != LIB_NO_ERROR) {
			comp_err(dev, "init_memory_tables() error %x: failed to get mem. type info of id %d out of %d",
				 ret, i, no_mem_tables);
			goto err;
		}

		ret = PP_LIB_API_CALL(pp_lib_data, XA_API_CMD_GET_MEM_INFO_SIZE, i,
				      &mem_size);
		if (ret != LIB_NO_ERROR) {
			comp_err(dev, "init_memory_tables() error %x: failed to get mem. size for mem. type %d",
				 ret, mem_type);
			goto err;
		}

		ret = PP_LIB_API_CALL(pp_lib_data, XA_API_CMD_GET_MEM_INFO_ALIGNMENT, i,
				      &mem_alignment);
		if (ret != LIB_NO_ERROR) {
			comp_err(dev, "init_memory_tables() error %x: failed to get mem. alignment of mem. type %d",
				 ret, mem_type);
			goto err;
		}

		ptr = allocate_lib_memory(mem_size, mem_alignment);
		if (!ptr) {
			comp_err(dev, "init_memory_tables() error %x: failed to allocate memory for %d",
				ret, mem_type);
			ret = -EINVAL;
			goto err;
		}

		ret = PP_LIB_API_CALL(pp_lib_data, XA_API_CMD_SET_MEM_PTR, i, ptr);
		if (ret != LIB_NO_ERROR) {
			comp_err(dev, "init_memory_tables() error %x: failed to set memory pointer for %d",
				 ret, mem_type);
			goto err;
		}

		switch((unsigned)mem_type) {
		case PP_MEM_TYPE_SCRATCH:
		case PP_MEM_TYPE_PERSISTENT:
			break;
		case PP_MEM_TYPE_IN_BUFFER:
			pp_lib_data.in_buff = ptr;
			pp_lib_data.in_buff_size = mem_size;
			break;
		case PP_MEM_TYPE_OUT_BUFFER:
			pp_lib_data.out_buff = ptr;
			pp_lib_data.out_buff_size = mem_size;
			break;
		default:
			comp_err(dev, "init_memory_tables() error %x: unrecognized memory type!",
                                 mem_type);
			ret = -EINVAL;
			goto err;
		}

		comp_info(dev, "init_memory_tables: allocated memory of %d bytes and alignment %d for mem. type %d",
			 mem_size, mem_alignment, mem_type);
	}

	return 0;
err:
	return ret;

}

static inline void *allocate_memtabs_container(size_t size) {

	return rballoc_align(0, SOF_MEM_CAPS_RAM, size, 4);
}

int pp_lib_prepare(struct comp_dev *dev,
		   struct post_process_shared_data *sdata)
{
	int ret, mem_tabs_size;//, lib_init_status;

	comp_dbg(dev, "pp_lib_prepare() start");
#if 0
	ret = PP_LIB_API_CALL(XA_API_CMD_INIT, XA_CMD_TYPE_INIT_API_PRE_CONFIG_PARAMS, NULL);
	comp_err(dev," pre config params returned %d\n", ret);
	comp_err(dev, "settign sample rate at %d\n", pp_lib_data.s_cfg.common.sample_rate);
#endif
#if 0
	ret = PP_LIB_API_CALL(XA_API_CMD_SET_CONFIG_PARAM,
			      XA_MP3DEC_CONFIG_PARAM_SAMP_FREQ,
			      &pp_lib_data.s_cfg.common.sample_rate);
	if (ret != LIB_NO_ERROR) {
		comp_err(dev, "pp_lib_prepare() error %x: failed to set sample rate",
			 ret);
		goto err;
	}

	ret = PP_LIB_API_CALL(XA_API_CMD_SET_CONFIG_PARAM,
			      XA_MP3DEC_CONFIG_PARAM_NUM_CHANNELS,
			      &pp_lib_data.s_cfg.common.channels);
	if (ret != LIB_NO_ERROR) {
		comp_err(dev, "pp_lib_prepare() error %x: failed to set max number of channels",
			 ret);
		goto err;
	}

	ret = PP_LIB_API_CALL(XA_API_CMD_SET_CONFIG_PARAM,
			      XA_MP3DEC_CONFIG_PARAM_PCM_WDSZ,
			      &pp_lib_data.s_cfg.common.sample_width);
	if (ret != LIB_NO_ERROR) {
		comp_err(dev, "pp_lib_prepare() error %x: failed to set sample width");
		goto err;
	}
#endif
	/* Allocate memory for the codec */
	ret = PP_LIB_API_CALL(pp_lib_data, XA_API_CMD_GET_MEMTABS_SIZE,
			      0,
			      &mem_tabs_size);
	if (ret != LIB_NO_ERROR) {
		comp_err(dev, "pp_lib_prepare() error %x: failed to get memtabs size",
			 ret);
		goto err;
	}
	comp_err(dev, "pp_lib_prepare memtab size %d\n", mem_tabs_size);

	pp_lib_data.mem_tabs = allocate_memtabs_container(mem_tabs_size);
	if (!pp_lib_data.mem_tabs) {
		comp_err(dev, "pp_lib_prepare() error: failed to allocate space for memtabs");
		ret = -EINVAL;
		goto err;
	} else {
		comp_dbg(dev, "pp_lib_prepare(): allocated %d bytes for memtabs",
			 mem_tabs_size);
	}

	ret = PP_LIB_API_CALL(pp_lib_data, XA_API_CMD_SET_MEMTABS_PTR, 0,
			      pp_lib_data.mem_tabs);
	if (ret != LIB_NO_ERROR) {
		comp_err(dev, "pp_lib_prepare() error %x: failed to set memtabs",
			ret);
		goto err;
	}

	ret = init_memory_tables(dev);
	if (ret != LIB_NO_ERROR) {
		comp_err(dev, "pp_lib_prepare() error %x: failed to init memory tables",
			 ret);
		goto err;
	}
#if 0
	ret = PP_LIB_API_CALL(pp_lib_data, XA_API_CMD_INIT,
			      XA_CMD_TYPE_INIT_PROCESS, NULL);
	if (ret != LIB_NO_ERROR) {
		comp_err(dev, "pp_lib_prepare() error %x: failed to initialize codec",

			 ret);
	//	goto err;
	}

	ret = PP_LIB_API_CALL(pp_lib_data, XA_API_CMD_INIT, XA_CMD_TYPE_INIT_DONE_QUERY,
			      &lib_init_status);
	comp_err(dev, "pp_lib_done query done %d status %d",
			 ret, lib_init_status);

	if (ret != LIB_NO_ERROR) {
		comp_err(dev, "pp_lib_prepare() error %x: failed to get lib init status",
			 ret);
		goto err;
	}a
	if (!lib_init_status) {
		comp_err(dev, "pp_lib_prepare() error: lib has not been initiated properly");
		ret = -EINVAL;
		goto err;
	} else {
		comp_dbg(dev, "pp_lib_prepare(): lib has been initialized properly");
	}
#endif
	/* Share lib buffer data with post processing component */
	sdata->lib_in_buff = pp_lib_data.in_buff;
	sdata->lib_out_buff = pp_lib_data.out_buff;
	sdata->lib_in_buff_size = pp_lib_data.in_buff_size;
	sdata->lib_out_buff_size = pp_lib_data.out_buff_size;

	pp_lib_data.state = PP_LIB_PREPARED;
	
///	return -EINVAL;
	return 0;
err:
//	return -EINVAL;
	return ret;

}



int pp_lib_process_init(struct comp_dev *dev, size_t avail, size_t *consumed)
{
	int ret = 0, init_done = 0;
	static int cntx = 0;

	cntx++;

	if (!avail) {
		PP_LIB_API_CALL(pp_lib_data, XA_API_CMD_INPUT_OVER, 0, NULL);
	}

	ret = PP_LIB_API_CALL(pp_lib_data, XA_API_CMD_SET_INPUT_BYTES, 0, &avail);
	if (ret != LIB_NO_ERROR) {
		comp_err(dev, "pp_lib_process_init() error %x: failed to set size of input data",
			 ret);
	}
	
	ret = PP_LIB_API_CALL(pp_lib_data, XA_API_CMD_INIT, XA_CMD_TYPE_INIT_PROCESS, NULL);
	if (ret != LIB_NO_ERROR) {
		if (cntx < CNTX) {
			comp_err(dev, "lib_abi_init (#%d) INIT_PROCESS failed ret = %x", cntx, ret);
		}
		return 1;
	}
	ret = PP_LIB_API_CALL(pp_lib_data, XA_API_CMD_INIT, XA_CMD_TYPE_INIT_DONE_QUERY, &init_done);
	if (ret != LIB_NO_ERROR) {
		if (cntx < CNTX) {
			comp_err(dev, "lib_abi_init (#%d) DONE QUERY failed ret = %x", cntx, ret);
		}

		return 1;
	}

	ret = PP_LIB_API_CALL(pp_lib_data, XA_API_CMD_GET_CURIDX_INPUT_BUF, 0, consumed);
	if (cntx < CNTX) {
		comp_err(dev, "lib_api_init(#%d) ret = %d init_done %d consumed %d", ret, cntx, init_done, *consumed);
	}
	return init_done;
}

int pp_lib_process_data(struct comp_dev *dev, size_t avail, size_t *consumed, size_t *produced) {
	int ret;
	int init_done;

	static int cntx = 0;
	cntx++;


	ret = PP_LIB_API_CALL(pp_lib_data, XA_API_CMD_SET_INPUT_BYTES, 0, &avail);
	if (ret != LIB_NO_ERROR) {
		if (cntx < CNTX) 
		comp_err(dev, "pp_lib_process_data() error %x: failed to set size of input data",
			 ret);
		goto err;
	}

	ret = PP_LIB_API_CALL(pp_lib_data, XA_API_CMD_EXECUTE, XA_CMD_TYPE_DO_EXECUTE,
			      NULL);
	if (ret != LIB_NO_ERROR) {
		if (cntx < CNTX) {
			comp_err(dev, "pp_lib_process_dat #%d XA_API_CMD_EXECUTE\n", cntx);
			handle_error(dev, ret);
		}
	}

	ret = PP_LIB_API_CALL(pp_lib_data, XA_API_CMD_EXECUTE, XA_CMD_TYPE_DONE_QUERY, &init_done);
	if (ret != LIB_NO_ERROR) {
		if (cntx < CNTX) 
			comp_err(dev, "pp_lib_process_data() DONE_QUERy ret = %x", ret);
	}
		
	ret = PP_LIB_API_CALL(pp_lib_data, XA_API_CMD_GET_OUTPUT_BYTES, 0, produced);
	if (ret != LIB_NO_ERROR) {
		if (cntx < CNTX)
		comp_err(dev, "pp_lib_process_data() error %x: could not get produced bytes",
			 ret);
	}

	ret = PP_LIB_API_CALL(pp_lib_data, XA_API_CMD_GET_CURIDX_INPUT_BUF, 0, consumed);
	if (ret != LIB_NO_ERROR) {
		if (cntx < CNTX)
		comp_err(dev, "pp_lib_process_data() error %x: could not get consumed bytes",
			 ret);
	}


	if (cntx < CNTX) {
		comp_err(dev, "pp_lib_proces_data(#%d) init_done %d consumed %d produced %d", cntx, init_done, *consumed, 
			 *produced);
	}

	return 0;
err:
	return ret;
}

int pp_lib_load_runtime_config(struct comp_dev *dev, void *cfg, int size) {
	int ret;

	if (!cfg)
		return -EINVAL;
	comp_dbg(dev, "RAJWA: loading system config, first bytes %d, whole size %d",
		*((uint32_t *)cfg), size);

	ret = memcpy_s(&pp_lib_data.r_cfg,
		       sizeof(struct post_process_runtime_config),
		       cfg, size);
	assert(!ret);

	return ret;
}

static int apply_general_config(struct comp_dev *dev) {
	int ret;
	struct pp_general_config *cfg = &pp_lib_data.r_cfg.general;

	/* Set system gain */
	comp_dbg(dev, "RAJWA: applying system gain of %d",cfg->system_gain);

	ret = PP_LIB_API_CALL(pp_lib_data, XA_API_CMD_SET_CONFIG_PARAM,
			      XA_DAP_VLLDP_CONFIG_PARAM_SYSTEM_GAIN,
			      &cfg->system_gain);
	if (ret != LIB_NO_ERROR) {
		comp_err(dev, "apply_general_config() error %x: failed to set system gain",
			 ret);
		goto end;
	}
#if 0
	/* Set post gain */
	ret = PP_LIB_API_CALL(XA_API_CMD_SET_CONFIG_PARAM,
			      XA_DAP_VLLDP_CONFIG_PARAM_POSTGAIN,
			      &cfg->post_gain);
	if (ret != LIB_NO_ERROR) {
		comp_err(dev, "apply_general_config() error %x: failed to set post gain",
			 ret);
		goto end;
	}

	/* Set audio optimizer enable */
	ret = PP_LIB_API_CALL(XA_API_CMD_SET_CONFIG_PARAM,
			      XA_DAP_VLLDP_CONFIG_PARAM_AUDIO_OPTIMIZER_ENABLE,
			      &cfg->optimizer_enable);
	if (ret != LIB_NO_ERROR) {
		comp_err(dev, "apply_general_config() error %x: failed to set optimizer enable",
			 ret);
		goto end;
	}

	/* Set speaker distortion mode */
	ret = PP_LIB_API_CALL(XA_API_CMD_SET_CONFIG_PARAM,
			      XA_DAP_VLLDP_CONFIG_PARAM_REGULATOR_SPEAKER_DISTORTION_ENABLE,
			      &cfg->speaker_distortion_mode);
	if (ret != LIB_NO_ERROR) {
		comp_err(dev, "apply_general_config() error %x: failed to set speaker distortion mode",
			 ret);
		goto end;
	}

	/* Set overdrive regulator */
	ret = PP_LIB_API_CALL(XA_API_CMD_SET_CONFIG_PARAM,
			      XA_DAP_VLLDP_CONFIG_PARAM_REGULATOR_OVERDRIVE,
			      &cfg->overdrive_regulator);
	if (ret != LIB_NO_ERROR) {
		comp_err(dev, "apply_general_config() error %x: failed to set overdrive regulator",
			 ret);
		goto end;
	}

	/* Set relaxation */
	ret = PP_LIB_API_CALL(XA_API_CMD_SET_CONFIG_PARAM,
			      XA_DAP_VLLDP_CONFIG_PARAM_REGULATOR_RELAXATION_AMOUNT,
			      &cfg->relaxation_amount);
	if (ret != LIB_NO_ERROR) {
		comp_err(dev, "apply_general_config() error %x: failed to set relaxation amount",
			 ret);
		goto end;
	}

	/* Set timbre regulation */
	ret = PP_LIB_API_CALL(XA_API_CMD_SET_CONFIG_PARAM,
			      XA_DAP_VLLDP_CONFIG_PARAM_REGULATOR_TIMBRE_PRESERVATION,
			      &cfg->timbre_regulation);
	if (ret != LIB_NO_ERROR) {
		comp_err(dev, "apply_general_config() error %x: failed to set timbre regulation",
			 ret);
		goto end;
	}

	/* Set isolated bands */
	ret = PP_LIB_API_CALL(XA_API_CMD_SET_CONFIG_PARAM,
			      XA_DAP_VLLDP_CONFIG_PARAM_REGULATOR_ISOLATED_BANDS,
			      &cfg->isolated_bands_regulation);
	if (ret != LIB_NO_ERROR) {
		comp_err(dev, "apply_general_config() error %x: failed to set isolated bands",
			 ret);
	}
#endif
end:
	return ret;

}

int pp_lib_apply_runtime_config(struct comp_dev *dev)
{
	int ret;

	ret = apply_general_config(dev);

	if (ret) {
		comp_err(dev, "post_process_apply_runtime_config() error %x: failed to apply general config",
			 ret);
	}

	return ret;
}

int pp_lib_get_max_blob_size(uint32_t *size) {
	*size = PP_LIB_MAX_BLOB_SIZE;

	return 0;
}

int pp_get_lib_state(bool *state) {
	*state = pp_lib_data.state;

	return 0;
}
