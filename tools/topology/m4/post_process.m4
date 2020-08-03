divert(-1)

dnl Define macro for Post Process effect widget

dnl N_POST_PROCESS(name)
define(`N_POST_PROCESS', `POSTPROCESS'PIPELINE_ID`.'$1)

dnl W_POST_PROCCESS(name, format, periods_sink, periods_source, core)
define(`W_POST_PROCESS',
`SectionVendorTuples."'N_POST_PROCESS($1)`_tuples_w" {'
`	tokens "sof_comp_tokens"'
`	tuples."word" {'
`		SOF_TKN_COMP_PERIOD_SINK_COUNT'		STR($3)
`		SOF_TKN_COMP_PERIOD_SOURCE_COUNT'	STR($4)
`		SOF_TKN_COMP_CORE_ID'			STR($5)
`	}'
`}'
`SectionData."'N_POST_PROCESS($1)`_data_w" {'
`	tuples "'N_POST_PROCESS($1)`_tuples_w"'
`}'
`SectionVendorTuples."'N_POST_PROCESS($1)`_tuples_str" {'
`	tokens "sof_comp_tokens"'
`	tuples."string" {'
`		SOF_TKN_COMP_FORMAT'	STR($2)
`	}'
`}'
`SectionData."'N_POST_PROCESS($1)`_data_str" {'
`	tuples "'N_POST_PROCESS($1)`_tuples_str"'
`}'
`SectionVendorTuples."'N_POST_PROCESS($1)`_tuples_str_type" {'
`	tokens "sof_process_tokens"'
`	tuples."string" {'
`		SOF_TKN_PROCESS_TYPE'	"EQIIR"
`	}'
`}'
`SectionData."'N_POST_PROCESS($1)`_data_str_type" {'
`	tuples "'N_POST_PROCESS($1)`_tuples_str_type"'
`}'
`SectionWidget."'N_POST_PROCESS($1)`" {'
`	index "'PIPELINE_ID`"'
`	type "effect"'
`	no_pm "true"'
`	data ['
`		"'N_POST_PROCESS($1)`_data_w"'
`		"'N_POST_PROCESS($1)`_data_str"'
`		"'N_POST_PROCESS($1)`_data_str_type"'
`	]'
`}')

divert(0)dnl
