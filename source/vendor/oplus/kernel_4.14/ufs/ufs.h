/*
 * Universal Flash Storage Host controller driver
 *
 * This code is based on drivers/scsi/ufs/ufs.h
 * Copyright (C) 2011-2013 Samsung India Software Operations
 *
 * Authors:
 *	Santosh Yaraganavi <santosh.sy@samsung.com>
 *	Vinayak Holikatti <h.vinayak@samsung.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * See the COPYING file in the top-level directory or visit
 * <http://www.gnu.org/licenses/gpl-2.0.html>
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * This program is provided "AS IS" and "WITH ALL FAULTS" and
 * without warranty of any kind. You are solely responsible for
 * determining the appropriateness of using and distributing
 * the program and assume all risks associated with your exercise
 * of rights with respect to the program, including but not limited
 * to infringement of third party rights, the risks and costs of
 * program errors, damage to or loss of data, programs or equipment,
 * and unavailability or interruption of operations. Under no
 * circumstances will the contributor of this Program be liable for
 * any damages of any kind arising from your use or distribution of
 * this program.
 */

#ifndef _UFS_H
#define _UFS_H

#include <linux/mutex.h>
#include <linux/types.h>

#define MAX_CDB_SIZE	16
#define GENERAL_UPIU_REQUEST_SIZE 32
#define QUERY_DESC_MAX_SIZE       255
#define QUERY_DESC_MIN_SIZE       2
#define QUERY_DESC_HDR_SIZE       2
#define QUERY_OSF_SIZE            (GENERAL_UPIU_REQUEST_SIZE - \
					(sizeof(struct utp_upiu_header)))
#define RESPONSE_UPIU_SENSE_DATA_LENGTH	18

#define UPIU_HEADER_DWORD(byte3, byte2, byte1, byte0)\
			cpu_to_be32((byte3 << 24) | (byte2 << 16) |\
			 (byte1 << 8) | (byte0))
/*
 * UFS device may have standard LUs and LUN id could be from 0x00 to
 * 0x7F. Standard LUs use "Peripheral Device Addressing Format".
 * UFS device may also have the Well Known LUs (also referred as W-LU)
 * which again could be from 0x00 to 0x7F. For W-LUs, device only use
 * the "Extended Addressing Format" which means the W-LUNs would be
 * from 0xc100 (SCSI_W_LUN_BASE) onwards.
 * This means max. LUN number reported from UFS device could be 0xC17F.
 */
#define UFS_UPIU_MAX_UNIT_NUM_ID	0x7F
#define UFS_MAX_LUNS		(SCSI_W_LUN_BASE + UFS_UPIU_MAX_UNIT_NUM_ID)
#define UFS_UPIU_WLUN_ID	(1 << 7)
#define UFS_UPIU_MAX_GENERAL_LUN	8

/* Well known logical unit id in LUN field of UPIU */
enum {
	UFS_UPIU_REPORT_LUNS_WLUN	= 0x81,
	UFS_UPIU_UFS_DEVICE_WLUN	= 0xD0,
	UFS_UPIU_BOOT_WLUN		= 0xB0,
	UFS_UPIU_RPMB_WLUN		= 0xC4,
};

/**
 * ufs_is_valid_unit_desc_lun - checks if the given LUN has a unit descriptor
 * @lun: LU number to check
 * @return: true if the lun has a matching unit descriptor, false otherwise
 */
static inline bool ufs_is_valid_unit_desc_lun(u8 lun)
{
	return (lun == UFS_UPIU_RPMB_WLUN || (lun < UFS_UPIU_MAX_GENERAL_LUN));
}

/*
 * UFS Protocol Information Unit related definitions
 */

/* Task management functions */
enum {
	UFS_ABORT_TASK		= 0x01,
	UFS_ABORT_TASK_SET	= 0x02,
	UFS_CLEAR_TASK_SET	= 0x04,
	UFS_LOGICAL_RESET	= 0x08,
	UFS_QUERY_TASK		= 0x80,
	UFS_QUERY_TASK_SET	= 0x81,
};

/* UTP UPIU Transaction Codes Initiator to Target */
enum {
	UPIU_TRANSACTION_NOP_OUT	= 0x00,
	UPIU_TRANSACTION_COMMAND	= 0x01,
	UPIU_TRANSACTION_DATA_OUT	= 0x02,
	UPIU_TRANSACTION_TASK_REQ	= 0x04,
	UPIU_TRANSACTION_QUERY_REQ	= 0x16,
};

/* UTP UPIU Transaction Codes Target to Initiator */
enum {
	UPIU_TRANSACTION_NOP_IN		= 0x20,
	UPIU_TRANSACTION_RESPONSE	= 0x21,
	UPIU_TRANSACTION_DATA_IN	= 0x22,
	UPIU_TRANSACTION_TASK_RSP	= 0x24,
	UPIU_TRANSACTION_READY_XFER	= 0x31,
	UPIU_TRANSACTION_QUERY_RSP	= 0x36,
	UPIU_TRANSACTION_REJECT_UPIU	= 0x3F,
};

/* UPIU Read/Write flags */
enum {
	UPIU_CMD_FLAGS_NONE	= 0x00,
	UPIU_CMD_FLAGS_WRITE	= 0x20,
	UPIU_CMD_FLAGS_READ	= 0x40,
};

/* UPIU Task Attributes */
enum {
	UPIU_TASK_ATTR_SIMPLE	= 0x00,
	UPIU_TASK_ATTR_ORDERED	= 0x01,
	UPIU_TASK_ATTR_HEADQ	= 0x02,
	UPIU_TASK_ATTR_ACA	= 0x03,
};

/* UPIU Query request function */
enum {
	UPIU_QUERY_FUNC_STANDARD_READ_REQUEST           = 0x01,
	UPIU_QUERY_FUNC_STANDARD_WRITE_REQUEST          = 0x81,
};

/* Flag idn for Query Requests*/
enum flag_idn {
	QUERY_FLAG_IDN_FDEVICEINIT      = 0x01,
	QUERY_FLAG_IDN_PWR_ON_WPE	= 0x03,
	QUERY_FLAG_IDN_BKOPS_EN         = 0x04,
	/* MTK PATCH: flag for fw update feasibility check */
	QUERY_FLAG_IDN_PERMANENTLY_DISABLE_FW_UPDATE = 0xB,
#if defined(CONFIG_UFSTW)
	QUERY_FLAG_IDN_TW_EN				= 0x0E,
	QUERY_FLAG_IDN_TW_BUF_FLUSH_EN			= 0x0F,
	QUERY_FLAG_IDN_TW_FLUSH_DURING_HIBERN = 0x10,
#endif
#if defined(CONFIG_SCSI_SKHPB)
	QUERY_FLAG_IDN_HPB_RESET	= 0x11,  /* JEDEC version */
#endif
};

/* Attribute idn for Query requests */
enum attr_idn {
	/* MTK PATCH: attribute for BootLUN configuration */
	QUERY_ATTR_IDN_BOOT_LUN_EN	= 0x00,
	QUERY_ATTR_IDN_ACTIVE_ICC_LVL	= 0x03,
	QUERY_ATTR_IDN_BKOPS_STATUS	= 0x05,
	QUERY_ATTR_IDN_EE_CONTROL	= 0x0D,
	QUERY_ATTR_IDN_EE_STATUS	= 0x0E,
	/* MTK PATCH: attribute for FFU status check */
	QUERY_ATTR_IDN_DEVICE_FFU_STATUS = 0x14,
#if defined(CONFIG_UFSTW)
	QUERY_ATTR_IDN_TW_FLUSH_STATUS		= 0x1C,
	QUERY_ATTR_IDN_TW_BUF_SIZE		= 0x1D,
	QUERY_ATTR_IDN_TW_BUF_LIFETIME_EST	= 0x1E,
	QUERY_ATTR_CUR_TW_BUF_SIZE		= 0x1F,
#endif
#if defined(CONFIG_UFSFEATURE)
	QUERY_ATTR_IDN_SUP_VENDOR_OPTIONS	= 0xFF,
#endif
};

/* MTK PATCH: status of FFU */
enum ufs_ffu_status {
	UFS_FFU_STATUS_NO_INFORMATION   = 0x0,
	UFS_FFU_STATUS_OK               = 0x1,
	UFS_FFU_STATUS_CORRUPTION       = 0x2,
	UFS_FFU_STATUS_INTERNAL_ERROR   = 0x3,
	UFS_FFU_STATUS_VERSION_MISMATCH = 0x4,
	UFS_FFU_STATUS_GENERAL_ERROR    = 0xFF,
};

/* Descriptor idn for Query requests */
enum desc_idn {
	QUERY_DESC_IDN_DEVICE		= 0x0,
	QUERY_DESC_IDN_CONFIGURATION	= 0x1,
	QUERY_DESC_IDN_UNIT		= 0x2,
	QUERY_DESC_IDN_RFU_0		= 0x3,
	QUERY_DESC_IDN_INTERCONNECT	= 0x4,
	QUERY_DESC_IDN_STRING		= 0x5,
	QUERY_DESC_IDN_RFU_1		= 0x6,
	QUERY_DESC_IDN_GEOMETRY		= 0x7,
	QUERY_DESC_IDN_POWER		= 0x8,
	QUERY_DESC_IDN_HEALTH		= 0x9,/* MTK PATCH */
	QUERY_DESC_IDN_MAX,
};

enum desc_header_offset {
	QUERY_DESC_LENGTH_OFFSET	= 0x00,
	QUERY_DESC_DESC_TYPE_OFFSET	= 0x01,
};

enum ufs_desc_def_size {
	QUERY_DESC_DEVICE_DEF_SIZE		= 0x40,
	QUERY_DESC_CONFIGURATION_DEF_SIZE	= 0x90,
	QUERY_DESC_UNIT_DEF_SIZE		= 0x23,
	QUERY_DESC_INTERCONNECT_DEF_SIZE	= 0x06,

	/* MTK PATCH: Geometry Descriptor size shall be 0x48 since UFS 2.1 */
	QUERY_DESC_GEOMETRY_DEF_SIZE		= 0x48,
	QUERY_DESC_POWER_DEF_SIZE		= 0x62,
	QUERY_DESC_HEALTH_MAX_SIZE		= 0x25, /* MTK PATCH */
};

#ifdef VENDOR_EDIT
/* Health descriptor parameters offsets in bytes*/
enum health_desc_param {
       HEALTH_DESC_PARAM_LEN                   = 0x0,
       HEALTH_DESC_PARAM_TYPE                  = 0x1,
       HEALTH_DESC_PARAM_EOL_INFO              = 0x2,
       HEALTH_DESC_PARAM_LIFE_TIME_EST_A       = 0x3,
       HEALTH_DESC_PARAM_LIFE_TIME_EST_B       = 0x4,
       //HEALTH_DESC_PARAM_VENDOR_PROPINFO	   = 0x5,
};
#endif

/* MTK PATCH: Read Geometry Descriptor for RPMB initialization */
enum geometry_desc_param_offset {
	GEOMETRY_DESC_LEN		= 0x0,
	GEOMETRY_DESC_TYPE		= 0x1,
	GEOMETRY_DESC_RPMB_RW_SIZE	= 0x17,
};

/* Unit descriptor parameters offsets in bytes*/
enum unit_desc_param {
	UNIT_DESC_PARAM_LEN			= 0x0,
	UNIT_DESC_PARAM_TYPE			= 0x1,
	UNIT_DESC_PARAM_UNIT_INDEX		= 0x2,
	UNIT_DESC_PARAM_LU_ENABLE		= 0x3,
	UNIT_DESC_PARAM_BOOT_LUN_ID		= 0x4,
	UNIT_DESC_PARAM_LU_WR_PROTECT		= 0x5,
	UNIT_DESC_PARAM_LU_Q_DEPTH		= 0x6,
	UNIT_DESC_PARAM_MEM_TYPE		= 0x8,
	UNIT_DESC_PARAM_DATA_RELIABILITY	= 0x9,
	UNIT_DESC_PARAM_LOGICAL_BLK_SIZE	= 0xA,
	UNIT_DESC_PARAM_LOGICAL_BLK_COUNT	= 0xB,
	UNIT_DESC_PARAM_ERASE_BLK_SIZE		= 0x13,
	UNIT_DESC_PARAM_PROVISIONING_TYPE	= 0x17,
	UNIT_DESC_PARAM_PHY_MEM_RSRC_CNT	= 0x18,
	UNIT_DESC_PARAM_CTX_CAPABILITIES	= 0x20,
	UNIT_DESC_PARAM_LARGE_UNIT_SIZE_M1	= 0x22,
#if defined(CONFIG_UFSHPB) || defined(CONFIG_SCSI_SKHPB)
	UNIT_DESC_HPB_LU_MAX_ACTIVE_REGIONS		= 0x23,
	UNIT_DESC_HPB_LU_PIN_REGION_START_OFFSET	= 0x25,
	UNIT_DESC_HPB_LU_NUM_PIN_REGIONS		= 0x27,
#endif
#if defined(CONFIG_UFSTW)
	UNIT_DESC_TW_LU_MAX_BUF_SIZE			= 0x29,
#endif
};

/* Device descriptor parameters offsets in bytes*/
enum device_desc_param {
	DEVICE_DESC_PARAM_LEN			= 0x0,
	DEVICE_DESC_PARAM_TYPE			= 0x1,
	DEVICE_DESC_PARAM_DEVICE_TYPE		= 0x2,
	DEVICE_DESC_PARAM_DEVICE_CLASS		= 0x3,
	DEVICE_DESC_PARAM_DEVICE_SUB_CLASS	= 0x4,
	DEVICE_DESC_PARAM_PRTCL			= 0x5,
	DEVICE_DESC_PARAM_NUM_LU		= 0x6,
	DEVICE_DESC_PARAM_NUM_WLU		= 0x7,
	DEVICE_DESC_PARAM_BOOT_ENBL		= 0x8,
	DEVICE_DESC_PARAM_DESC_ACCSS_ENBL	= 0x9,
	DEVICE_DESC_PARAM_INIT_PWR_MODE		= 0xA,
	DEVICE_DESC_PARAM_HIGH_PR_LUN		= 0xB,
	DEVICE_DESC_PARAM_SEC_RMV_TYPE		= 0xC,
	DEVICE_DESC_PARAM_SEC_LU		= 0xD,
	DEVICE_DESC_PARAM_BKOP_TERM_LT		= 0xE,
	DEVICE_DESC_PARAM_ACTVE_ICC_LVL		= 0xF,
	DEVICE_DESC_PARAM_SPEC_VER		= 0x10,
	DEVICE_DESC_PARAM_MANF_DATE		= 0x12,
	DEVICE_DESC_PARAM_MANF_NAME		= 0x14,
	DEVICE_DESC_PARAM_PRDCT_NAME		= 0x15,
	DEVICE_DESC_PARAM_SN			= 0x16,
	DEVICE_DESC_PARAM_OEM_ID		= 0x17,
	DEVICE_DESC_PARAM_MANF_ID		= 0x18,
	DEVICE_DESC_PARAM_UD_OFFSET		= 0x1A,
	DEVICE_DESC_PARAM_UD_LEN		= 0x1B,
	DEVICE_DESC_PARAM_RTT_CAP		= 0x1C,
	DEVICE_DESC_PARAM_FRQ_RTC		= 0x1D,
	DEVICE_DESC_PARAM_FEAT_SUP		= 0x1F,
	/* MTK PATCH: Product Revision Level index in String Descriptor */
	DEVICE_DESC_PARAM_PRDCT_REV		= 0x2A,
#if defined(CONFIG_UFSHPB) || defined(CONFIG_SCSI_SKHPB)
	DEVICE_DESC_PARAM_HPB_VER		= 0x40,
	DEVICE_DESC_PARAM_HPB_CONTROL		= 0x42, /* JEDEC version */
#endif
#if defined(CONFIG_UFSFEATURE)
	DEVICE_DESC_PARAM_EX_FEAT_SUP		= 0x4F,
#endif
#if defined(CONFIG_UFSTW)
	DEVICE_DESC_PARAM_TW_RETURN_TO_USER	= 0x53,
	DEVICE_DESC_PARAM_TW_BUF_TYPE		= 0x54,
	DEVICE_DESC_PARAM_NUM_SHARED_WB_BUF_AU	= 0x55, /* JEDEC version */
#endif
};

enum geometry_desc_param {
	GEOMETRY_DESC_SEGMENT_SIZE = 0x0D,
#if defined(CONFIG_UFSHPB) || defined(CONFIG_SCSI_SKHPB)
	GEOMETRY_DESC_HPB_REGION_SIZE			= 0x48,
	GEOMETRY_DESC_HPB_NUMBER_LU			= 0x49,
	GEOMETRY_DESC_HPB_SUBREGION_SIZE		= 0x4A,
	GEOMETRY_DESC_HPB_DEVICE_MAX_ACTIVE_REGIONS	= 0x4B,
#endif
#if defined(CONFIG_UFSTW)
	GEOMETRY_DESC_TW_MAX_SIZE			= 0x4F,
	GEOMETRY_DESC_TW_NUMBER_LU			= 0x53,
	GEOMETRY_DESC_TW_CAP_ADJ_FAC			= 0x54,
	GEOMETRY_DESC_TW_SUPPORT_USER_REDUCTION_TYPES	= 0x55,
	GEOMETRY_DESC_TW_SUPPORT_BUF_TYPE		= 0x56,
	GEOMETRY_DESC_TW_GROUP_NUM_CAP			= 0x57,
#endif
};

/*
 * Logical Unit Write Protect
 * 00h: LU not write protected
 * 01h: LU write protected when fPowerOnWPEn =1
 * 02h: LU permanently write protected when fPermanentWPEn =1
 */
enum ufs_lu_wp_type {
	UFS_LU_NO_WP		= 0x00,
	UFS_LU_POWER_ON_WP	= 0x01,
	UFS_LU_PERM_WP		= 0x02,
};

/* bActiveICCLevel parameter current units */
enum {
	UFSHCD_NANO_AMP		= 0,
	UFSHCD_MICRO_AMP	= 1,
	UFSHCD_MILI_AMP		= 2,
	UFSHCD_AMP		= 3,
};

#define POWER_DESC_MAX_SIZE			0x62
#define POWER_DESC_MAX_ACTV_ICC_LVLS		16

/* Attribute  bActiveICCLevel parameter bit masks definitions */
#define ATTR_ICC_LVL_UNIT_OFFSET	14
#define ATTR_ICC_LVL_UNIT_MASK		(0x3 << ATTR_ICC_LVL_UNIT_OFFSET)
#define ATTR_ICC_LVL_VALUE_MASK		0x3FF

/* Power descriptor parameters offsets in bytes */
enum power_desc_param_offset {
	PWR_DESC_LEN			= 0x0,
	PWR_DESC_TYPE			= 0x1,
	PWR_DESC_ACTIVE_LVLS_VCC_0	= 0x2,
	PWR_DESC_ACTIVE_LVLS_VCCQ_0	= 0x22,
	PWR_DESC_ACTIVE_LVLS_VCCQ2_0	= 0x42,
};

/* Exception event mask values */
enum {
	MASK_EE_STATUS		= 0xFFFF,
	MASK_EE_URGENT_BKOPS	= (1 << 2),
#if defined(CONFIG_UFSTW)
	MASK_EE_TW		= (1 << 5),
#endif
};

#if defined(CONFIG_UFSTW)
/* TW buffer type */
enum {
	WB_LU_DEDICATED_BUFFER_TYPE	= 0x0,
	WB_SINGLE_SHARE_BUFFER_TYPE	= 0x1
};
#endif

/* Background operation status */
enum bkops_status {
	BKOPS_STATUS_NO_OP               = 0x0,
	BKOPS_STATUS_NON_CRITICAL        = 0x1,
	BKOPS_STATUS_PERF_IMPACT         = 0x2,
	BKOPS_STATUS_CRITICAL            = 0x3,
	BKOPS_STATUS_MAX		 = BKOPS_STATUS_CRITICAL,
};

/* UTP QUERY Transaction Specific Fields OpCode */
enum query_opcode {
	UPIU_QUERY_OPCODE_NOP		= 0x0,
	UPIU_QUERY_OPCODE_READ_DESC	= 0x1,
	UPIU_QUERY_OPCODE_WRITE_DESC	= 0x2,
	UPIU_QUERY_OPCODE_READ_ATTR	= 0x3,
	UPIU_QUERY_OPCODE_WRITE_ATTR	= 0x4,
	UPIU_QUERY_OPCODE_READ_FLAG	= 0x5,
	UPIU_QUERY_OPCODE_SET_FLAG	= 0x6,
	UPIU_QUERY_OPCODE_CLEAR_FLAG	= 0x7,
	UPIU_QUERY_OPCODE_TOGGLE_FLAG	= 0x8,
};

/* Query response result code */
enum {
	QUERY_RESULT_SUCCESS                    = 0x00,
	QUERY_RESULT_NOT_READABLE               = 0xF6,
	QUERY_RESULT_NOT_WRITEABLE              = 0xF7,
	QUERY_RESULT_ALREADY_WRITTEN            = 0xF8,
	QUERY_RESULT_INVALID_LENGTH             = 0xF9,
	QUERY_RESULT_INVALID_VALUE              = 0xFA,
	QUERY_RESULT_INVALID_SELECTOR           = 0xFB,
	QUERY_RESULT_INVALID_INDEX              = 0xFC,
	QUERY_RESULT_INVALID_IDN                = 0xFD,
	QUERY_RESULT_INVALID_OPCODE             = 0xFE,
	QUERY_RESULT_GENERAL_FAILURE            = 0xFF,
};

/* UTP Transfer Request Command Type (CT) */
enum {
	UPIU_COMMAND_SET_TYPE_SCSI	= 0x0,
	UPIU_COMMAND_SET_TYPE_UFS	= 0x1,
	UPIU_COMMAND_SET_TYPE_QUERY	= 0x2,
};

/* UTP Transfer Request Command Offset */
#define UPIU_COMMAND_TYPE_OFFSET	28

/* Offset of the response code in the UPIU header */
#define UPIU_RSP_CODE_OFFSET		8

enum {
	MASK_SCSI_STATUS		= 0xFF,
	MASK_TASK_RESPONSE              = 0xFF00,
	MASK_RSP_UPIU_RESULT            = 0xFFFF,
	MASK_QUERY_DATA_SEG_LEN         = 0xFFFF,
	MASK_RSP_UPIU_DATA_SEG_LEN	= 0xFFFF,
	MASK_RSP_EXCEPTION_EVENT        = 0x10000,
	MASK_TM_SERVICE_RESP		= 0xFF,
#if defined(CONFIG_UFSHPB) || defined(CONFIG_SCSI_SKHPB)
	MASK_RSP_UPIU_HPB_UPDATE_ALERT	= 0x20000, /* JEDEC version */
#endif
};

/* Task management service response */
enum {
	UPIU_TASK_MANAGEMENT_FUNC_COMPL		= 0x00,
	UPIU_TASK_MANAGEMENT_FUNC_NOT_SUPPORTED = 0x04,
	UPIU_TASK_MANAGEMENT_FUNC_SUCCEEDED	= 0x08,
	UPIU_TASK_MANAGEMENT_FUNC_FAILED	= 0x05,
	UPIU_INCORRECT_LOGICAL_UNIT_NO		= 0x09,
};

/* UFS device power modes */
enum ufs_dev_pwr_mode {
	UFS_ACTIVE_PWR_MODE	= 1,
	UFS_SLEEP_PWR_MODE	= 2,
	UFS_POWERDOWN_PWR_MODE	= 3,
};

/**
 * struct utp_upiu_header - UPIU header structure
 * @dword_0: UPIU header DW-0
 * @dword_1: UPIU header DW-1
 * @dword_2: UPIU header DW-2
 */
struct utp_upiu_header {
	__be32 dword_0;
	__be32 dword_1;
	__be32 dword_2;
};

/**
 * struct utp_upiu_cmd - Command UPIU structure
 * @data_transfer_len: Data Transfer Length DW-3
 * @cdb: Command Descriptor Block CDB DW-4 to DW-7
 */
struct utp_upiu_cmd {
	__be32 exp_data_transfer_len;
	u8 cdb[MAX_CDB_SIZE];
};

/**
 * struct utp_upiu_query - upiu request buffer structure for
 * query request.
 * @opcode: command to perform B-0
 * @idn: a value that indicates the particular type of data B-1
 * @index: Index to further identify data B-2
 * @selector: Index to further identify data B-3
 * @reserved_osf: spec reserved field B-4,5
 * @length: number of descriptor bytes to read/write B-6,7
 * @value: Attribute value to be written DW-5
 * @reserved: spec reserved DW-6,7
 */
struct utp_upiu_query {
	u8 opcode;
	u8 idn;
	u8 index;
	u8 selector;
	__be16 reserved_osf;
	__be16 length;
	__be32 value;
	__be32 reserved[2];
};

/**
 * struct utp_upiu_req - general upiu request structure
 * @header:UPIU header structure DW-0 to DW-2
 * @sc: fields structure for scsi command DW-3 to DW-7
 * @qr: fields structure for query request DW-3 to DW-7
 */
struct utp_upiu_req {
	struct utp_upiu_header header;
	union {
		struct utp_upiu_cmd sc;
		struct utp_upiu_query qr;
	};
};

/**
 * struct utp_cmd_rsp - Response UPIU structure
 * @residual_transfer_count: Residual transfer count DW-3
 * @reserved: Reserved double words DW-4 to DW-7
 * @sense_data_len: Sense data length DW-8 U16
 * @sense_data: Sense data field DW-8 to DW-12
 */
struct utp_cmd_rsp {
	__be32 residual_transfer_count;
	__be32 reserved[4];
	__be16 sense_data_len;
	u8 sense_data[RESPONSE_UPIU_SENSE_DATA_LENGTH];
};

/**
 * struct utp_upiu_rsp - general upiu response structure
 * @header: UPIU header structure DW-0 to DW-2
 * @sr: fields structure for scsi command DW-3 to DW-12
 * @qr: fields structure for query request DW-3 to DW-7
 */
struct utp_upiu_rsp {
	struct utp_upiu_header header;
	union {
		struct utp_cmd_rsp sr;
		struct utp_upiu_query qr;
	};
};

/**
 * struct utp_upiu_task_req - Task request UPIU structure
 * @header - UPIU header structure DW0 to DW-2
 * @input_param1: Input parameter 1 DW-3
 * @input_param2: Input parameter 2 DW-4
 * @input_param3: Input parameter 3 DW-5
 * @reserved: Reserved double words DW-6 to DW-7
 */
struct utp_upiu_task_req {
	struct utp_upiu_header header;
	__be32 input_param1;
	__be32 input_param2;
	__be32 input_param3;
	__be32 reserved[2];
};

/**
 * struct utp_upiu_task_rsp - Task Management Response UPIU structure
 * @header: UPIU header structure DW0-DW-2
 * @output_param1: Ouput parameter 1 DW3
 * @output_param2: Output parameter 2 DW4
 * @reserved: Reserved double words DW-5 to DW-7
 */
struct utp_upiu_task_rsp {
	struct utp_upiu_header header;
	__be32 output_param1;
	__be32 output_param2;
	__be32 reserved[3];
};

/**
 * struct ufs_query_req - parameters for building a query request
 * @query_func: UPIU header query function
 * @upiu_req: the query request data
 */
struct ufs_query_req {
	u8 query_func;
	struct utp_upiu_query upiu_req;
};

/**
 * struct ufs_query_resp - UPIU QUERY
 * @response: device response code
 * @upiu_res: query response data
 */
struct ufs_query_res {
	u8 response;
	struct utp_upiu_query upiu_res;
};

#define UFS_VREG_VCC_MIN_UV	   2700000 /* uV */
#define UFS_VREG_VCC_MAX_UV	   3600000 /* uV */
#define UFS_VREG_VCC_1P8_MIN_UV    1700000 /* uV */
#define UFS_VREG_VCC_1P8_MAX_UV    1950000 /* uV */
#define UFS_VREG_VCCQ_MIN_UV	   1100000 /* uV */
#define UFS_VREG_VCCQ_MAX_UV	   1300000 /* uV */
#define UFS_VREG_VCCQ2_MIN_UV	   1650000 /* uV */
#define UFS_VREG_VCCQ2_MAX_UV	   1950000 /* uV */

/*
 * VCCQ & VCCQ2 current requirement when UFS device is in sleep state
 * and link is in Hibern8 state.
 */
#define UFS_VREG_LPM_LOAD_UA	1000 /* uA */

struct ufs_vreg {
	struct regulator *reg;
	const char *name;
	bool enabled;
	bool unused;
	int min_uV;
	int max_uV;
	int min_uA;
	int max_uA;
};

/* MTK PATCH */
enum ufs_vreg_state {
	UFS_REG_HBA_INIT,
	UFS_REG_HBA_EXIT,
	UFS_REG_SUSPEND_SET_LPM,
	UFS_REG_SUSPEND_SET_HPM,
	UFS_REG_RESUME_SET_LPM,
	UFS_REG_RESUME_SET_HPM,
};

struct ufs_vreg_info {
	struct ufs_vreg *vcc;
	struct ufs_vreg *vccq;
	struct ufs_vreg *vccq2;
	struct ufs_vreg *vdd_hba;
	enum ufs_vreg_state state; /* MTK PATCH */
};

struct ufs_dev_info {
	bool f_power_on_wp_en;
	/* Keeps information if any of the LU is power on write protected */
	bool is_lu_power_on_wp;
};

#define MAX_MODEL_LEN 16
#define MAX_PRL_LEN   5 /* MTK PATCH */

/**
 * ufs_dev_desc - ufs device details from the device descriptor
 *
 * @wmanufacturerid: card details
 * @model: card model
 */
struct ufs_dev_desc {
	u16 wmanufacturerid;
	char model[MAX_MODEL_LEN + 1];
	char prl[MAX_PRL_LEN + 1]; /* MTK PATCH */
};

#endif /* End of Header */
