#ifndef __MBSDEFINES_HH
#define __MBSDEFINES_HH

// MBS defines
#define MBS_ID_WORD				"%MBS_RAW_DATA%"			/*!< magic id */
#define MBS_SIZEOF_DATA_W		0x4000						/*!< length of data (words) */
#define MBS_SIZEOF_DATA_B		0x8000						/*!< length of data (bytes) */

#define MBS_RTYPE_ERROR			0xffffffff					/*!< error */

#define MBS_CTYPE_FILE			0x1 						/*!< connection: file */
#define MBS_CTYPE_SYNC			0x2 						/*!< connection: synchronous via transport manager */
#define MBS_CTYPE_ASYNC 		0x4 						/*!< connection: asynchronous via stream server */
#define MBS_CTYPE_REMOTE		0x8 						/*!< connection: remote tape */
#define MBS_CTYPE_FILE_LMD		(MBS_CTYPE_FILE | 0x10) 	/*!< connection: file, format is LMD */
#define MBS_CTYPE_FILE_MED		(MBS_CTYPE_FILE | 0x20) 	/*!< connection: file, format is MED */

#define MBS_TY_FHEADER			0x1 						/*!< type: file header */
#define MBS_X_FHEADER			0 							/*!< index: file header */

#define MBS_TY_BUFFER			0x2 						/*!< type: buffer */
#define MBS_X_BUFFER			1 							/*!< index: buffer */
#define MBS_BTYPE_FHEADER		0x000107d0					/*!< file header [1,2000] */
#define MBS_BTYPE_VME			0x0001000a					/*!< vme event [1,10] */
#define MBS_BTYPE_EOF			0							/*!< end of file */
#define MBS_BTYPE_ERROR			0xffffffff					/*!< error */
#define MBS_BTYPE_ABORT			0xfffffffe					/*!< abort */
#define MBS_BTYPE_RAW			0xfffffffd					/*!< raw mode */

#define MBS_TY_EVENT			0x4 						/*!< type: event */
#define MBS_X_EVENT				2 							/*!< index: event */
#define MBS_ETYPE_VME			0x0001000a					/*!< vme event [1,10] */
#define MBS_ETYPE_EOF			MBS_BTYPE_EOF				/*!< end of file */
#define MBS_ETYPE_ERROR			MBS_BTYPE_ERROR				/*!< error */
#define MBS_ETYPE_ABORT			MBS_BTYPE_ABORT				/*!< abort */
#define MBS_ETYPE_RAW			MBS_BTYPE_RAW				/*!< raw mode */
#define MBS_ETYPE_WAIT			0xfffffffc					/*!< wait for write complete */
#define MBS_ETYPE_EOW			0xaffec0c0					/*!< end of wait loop */
#define MBS_ETYPE_START			0xfffffffb					/*!< start event */
#define MBS_ETYPE_STOP			0xfffffffa					/*!< stop event */

#define MBS_TY_SUBEVENT				0x8							/*!< type: subevent */
#define MBS_X_SUBEVENT				3							/*!< index: subevent */
#define MBS_STYPE_CAMAC_1			0x0001000a					/*!< subevent [1,10]: camac (chn,data) */
#define MBS_STYPE_CAMAC_WO_ID_1		0x000b000a					/*!< subevent [11,10]: camac w/o chn number */
#define MBS_STYPE_CAMAC_MULT_MOD	0x000c000a					/*!< subevent [12,10]: camac multi module */
#define MBS_STYPE_CAMAC_2			0x000d000a					/*!< subevent [13,10]: camac, data stored in hit buffer */
#define MBS_STYPE_CAMAC_WO_ID_2		0x000e000a					/*!< subevent [14,10]: camac, data stored in hit buffer */
#define MBS_STYPE_CAMAC_RAW 		0x000F000a					/*!< subevent [15,10]: camac, raw format */
#define MBS_STYPE_CAMAC_DGF_1		0x0015000a					/*!< subevent [21,10]: camac, dgf, accumulate histos directly */
#define MBS_STYPE_CAMAC_DGF_2		0x0016000a					/*!< subevent [22,10]: camac, dgf, data stored in hit buffer */
#define MBS_STYPE_CAMAC_DGF_3		0x0017000a					/*!< subevent [23,10]: camac, dgf, data stored in hit buffer */
#define MBS_STYPE_CAMAC_SILENA_1	0x001f000a					/*!< subevent [31,10]: camac, silena 4418 */
#define MBS_STYPE_CAMAC_SILENA_2	0x0020000a					/*!< subevent [32,10]: camac, silena 4418, data stored in hit buffer */
#define MBS_STYPE_VME_CAEN_V556_1	0x0021000a					/*!< subevent [33,10]: vme, caen v556 */
#define MBS_STYPE_VME_CAEN_V556_2	0x0022000a					/*!< subevent [34,10]: vme, caen v556, data stored in hit buffer */
#define MBS_STYPE_VME_CAEN_V556_3	0x0023000a					/*!< subevent [35,10]: vme, caen v556, data stored in hit buffer */
#define MBS_STYPE_VME_CAEN_1		0x0029000a					/*!< subevent [41,10]: vme, caen v875, v775 */
#define MBS_STYPE_VME_CAEN_2		0x002a000a					/*!< subevent [42,10]: vme, caen v875, v775, data stored in hit buffer */
#define MBS_STYPE_VME_CAEN_3		0x002b000a					/*!< subevent [43,10]: vme, caen v875, v775, data stored in hit buffer */
#define MBS_STYPE_VME_CAEN_V1X90_1	0x002d000a					/*!< subevent [45,10]: vme, caen v1190, v1290 */
#define MBS_STYPE_VME_CAEN_V1X90_2	0x002e000a					/*!< subevent [46,10]: vme, caen v1190, v1290, data stored in hit buffer */
#define MBS_STYPE_VME_CAEN_V1X90_3	0x002f000a					/*!< subevent [47,10]: vme, caen v1190, v1290, data stored in hit buffer */
#define MBS_STYPE_VME_SIS_1			0x0033000a					/*!< subevent [51,10]: vme, struck/sis */
#define MBS_STYPE_VME_SIS_2			0x0034000a					/*!< subevent [52,10]: vme, struck/sis, data stored in hit buffer */
#define MBS_STYPE_VME_SIS_3			0x0035000a					/*!< subevent [53,10]: vme, struck/sis, data stored in hit buffer */
#define MBS_STYPE_VME_SIS_3300		0x0036000a					/*!< subevent [54,10]: vme, struck/sis 3300, data stored in hit buffer */
#define MBS_STYPE_VME_SIS_3302_1	0x0038000a					/*!< subevent [55,10]: vme, struck/sis 3302 */
#define MBS_STYPE_VME_SIS_3302_2	0x0038000a					/*!< subevent [56,10]: vme, struck/sis 3302, data stored in hit buffer */
#define MBS_STYPE_VME_SIS_3302_3	0x0039000a					/*!< subevent [57,10]: vme, struck/sis 3302, data stored in hit buffer */
#define MBS_STYPE_CAMAC_CPTM		0x003d000a					/*!< subevent [61,10]: camac, cptm Cologne */
#define MBS_STYPE_DATA_SHORT		0x0040000a					/*!< subevent [64,10]: universal data container, short integer, 2 bytes */
#define MBS_STYPE_DATA_INT			0x0041000a					/*!< subevent [65,10]: universal data container, integer, 4 bytes */
#define MBS_STYPE_DATA_FLOAT		0x0042000a					/*!< subevent [66,10]: universal data container, float/double */
#define MBS_STYPE_HITBUF_1			0x0047000a					/*!< subevent [71,10]: hit buffer */
#define MBS_STYPE_VME_MADC_1		0x0051000a					/*!< subevent [81,10]: vme, mesytec madc32 */
#define MBS_STYPE_VME_MADC_2		0x0052000a					/*!< subevent [82,10]: vme, mesytec madc32, data stored in hit buffer */
#define MBS_STYPE_VME_MADC_3		0x0053000a					/*!< subevent [83,10]: vme, mesytec madc32, data stored in hit buffer */
#define MBS_STYPE_VME_SIS_3820_1	0x005b000a					/*!< subevent [91,10]: vme, sis 3820 scaler */
#define MBS_STYPE_VME_SIS_3820_2	0x005c000a					/*!< subevent [92,10]: vme, sis 3820 scaler, data stored in hit buffer */
#define MBS_STYPE_VME_SIS_3820_3	0x005d000a					/*!< subevent [93,10]:vme, sis 3820 scaler, data stored in hit buffer */
#define MBS_STYPE_TIME_STAMP		0x00012328					/*!< subevent [1,9000]: time stamp */
#define MBS_STYPE_DEAD_TIME 		0x00022328					/*!< subevent [2,9000]: dead time */
#define MBS_STYPE_DUMMY 	 		0x006f006f					/*!< subevent [111,111]: dummy */
#define MBS_STYPE_EOE				MBS_BTYPE_EOF				/*!< end of event */
#define MBS_STYPE_ERROR				MBS_BTYPE_ERROR				/*!< error */
#define MBS_STYPE_ABORT				MBS_BTYPE_ABORT				/*!< abort */
#define MBS_STYPE_RAW		 		MBS_BTYPE_RAW				/*!< raw mode */

#define MBS_TY_STAT				0x10						 	/*!< type: statistics */
#define MBS_X_STAT				4						 		/*!< index: statistics */

#define MBS_N_BELEMS			5								/*!< number if buffer elements */
#define MBS_N_TRIGGERS			16								/*!< number of triggers */
#define MBS_N_BUFFERS			5								/*!< number of buffers in buffer ring */

#define MBS_L_STR				256 							/*!< string length */
#define MBS_L_NAME				64								/*!< lenght of a name string */

#define MBS_ODD_NOF_PARAMS		1	///< indicates an odd number of params

#define BYTE_ORDER_1_TO_1   0    ///< 1:1, nothing to do
#define BYTE_ORDER_BSW      1    ///< byte swap
#define BYTE_ORDER_LSW      2    ///< long word swap
#define BYTE_ORDER_REV      3    ///< reverse order


// Module data defines
#define CAEN7X5_NBOFCHAN  32
#define CAEN_V7X5_SH_TYPE_OF_WORD 0x18
#define CAEN_V7X5_M_TYPE_OF_WORD 0x7
#define CAEN_V7X5_D_HDR 0x2
#define CAEN_V7X5_D_VALID_DATUM 0x0
#define CAEN_V7X5_D_EOB 0x4
#define CAEN_V7X5_SH_CRATENR 0x10
#define CAEN_V7X5_M_CRATENR 0xff
#define CAEN_V7X5_SH_WC 0x8
#define CAEN_V7X5_M_WC  0x3f
#define CAEN_V7X5_M_MSERIAL 0xff
#define CAEN_V7X5_SH_CHN 0x10
#define CAEN_V7X5_M_CHN 0x3f
#define CAEN_V7X5_B_OVERFLOW 0x1000
#define CAEN_V7X5_B_UNDERTHRESH 0x2000
#define CAEN_V7X5_M_ADCDATA 0xfff
#define CAEN_V7X5_M_EVENTCOUNTER 0xffffff

#define SIS3600_D_HDR     (0x1 << 15)
#define SIS3600_MSERIAL   (0xff)

#define SCALER_SH_CRATENR 0x10
#define SCALER_M_CRATENR 0xff
#define SCALER_M_SERIAL 0xffff

#define DGF_SCALER_END_OF_BUFFER 0x5252
#define DGF_SCALER_MAGIC_WORD 0x2525
#define DGF_SCALER_INDEX_REALTIME 0
#define DGF_SCALER_INDEX_RUNTIME 3
#define DGF_SCALER_INDEX_GSLTTIME 6
#define DGF_SCALER_INDEX_NEVENTS 9
#define DGF_SCALER_INDEX_CH_OFFSET 62
#define DGF_SCALER_INDEX_CH_SIZE 24
#define DGF_SCALER_MIN_SIZE 144

#define BRAGG_CHAMBER_HEADER        (0x1 << 15)
#define BRAGG_CHAMBER_MODULE_NUMBER  0xFF
#define BRAGG_CHAMBER_MODULE_ID      0x7F
#define BRAGG_CHAMBER_SH_ModuleId   8

#define MESYTEC_MADC_MODULE_ID 0xff
#define MESYTEC_MADC_OUTPUT_FORMAT 0x8000 //should be zero
#define MESYTEC_MADC_ADC_RESOLUTION 0x7000 //0=2k,1=4k,2=4k hires,3=8k,4=8k hires
#define MESYTEC_MADC_WORD_COUNT 0xfff
#define MESYTEC_MADC_CHANNEL_NUMBER 0x1f
#define MESYTEC_MADC_VALUE 0x1fff
#define MESYTEC_MADC_OUT_OF_RANGE 0x4000
#define MESYTEC_MADC_EXTENDED_TIMESTAMP 0x80
#define MESYTEC_MADC_EXTENDED_TIMESTAMP_SHIFT 30
#define MESYTEC_MADC_EXTENDED_TIMESTAMP_MASK 0x3fffc0000000
#define MESYTEC_MADC_EXTENDED_TIMESTAMP_DATA 0xffff
#define MESYTEC_MADC_END_OF_EVENT 0xc0000000
#define MESYTEC_MADC_TIMESTAMP 0x3fffffff

// XIA defines
#define BUFHEADLEN 6
#define EVENTHEADLEN 3
#define CHANHEADLEN 9

// standard list mode
#define STD_LM               0
#define STD_LM_RUNTASK     256
#define STD_LM_BUFFORMAT   256

// compressed list mode
#define COMP_LM              1
#define COMP_LM_RUNTASK    257
#define COMP_LM_BUFFORMAT  257

// standard fast list mode
#define STD_FLM              2
#define STD_FLM_RUNTASK    512
#define STD_FLM_BUFFORMAT  512

// compressed fast list mode
#define COMP_FLM             3
#define COMP_FLM_RUNTASK   513
#define COMP_FLM_BUFFORMAT 513

// list mode - Compression 3
#define COMP3_LM             4
#define COMP3_LM_RUNTASK   259
#define COMP3_LM_BUFFORMAT 259

// default run mode
#define DEFAULT_RUNTASK   STD_LM_RUNTASK
#define DEFAULT_BUFFORMAT STD_LM_BUFFORMAT

// XIA subevent type
#define XIA_EVENT          666

#define XIA_STD_LM         (XIA_EVENT+STD_LM)
#define XIA_COMP_LM        (XIA_EVENT+COMP_LM)
#define XIA_STD_FLM         (XIA_EVENT+STD_FLM)
#define XIA_COMP_FLM        (XIA_EVENT+COMP_FLM)


#endif
