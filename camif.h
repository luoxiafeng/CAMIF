/* Copyright (c) 2014~2112 ShangHai Infotm Ltd all rights reserved. 
** 
** Use of Infotm's code is governed by terms and conditions 
** stated in the accompanying licensing statement. 
** 
** FPGA soc verify Q3 camif header file
**
** Revision History: 
** ----------------- 
** v0.0.1	sam.zhou@2014/10/20: first version.
**
*****************************************************************************/
#ifndef _CAMIF_H_
#define _cAMIF_H_

#ifdef __cplusplus
extern "C"
{
#endif
#include <camera/camif/camif_reg.h>
//for use isp common define structure , include isp header file
#include <camera/isp/isp.h>	
#include <linux/list.h>	

#define CAMIF_INTR_NUM (CAMIF_INT_ID0)

//for stream demo used
//#define USE_SDCARD_FOR_TEST
//#define MACRO_FOR_DEMO_USED

enum pixel_fmt {
	PIX_FMT_NV12,
	PIX_FMT_NV21,
	PIX_FMT_YUYV,
	PIX_FMT_YVYU,
	PIX_FMT_UYVY,
	PIX_FMT_VYUY
};

enum store_fmt {
	PIX_PLANAR,
	PIX_SEMP,
	PIX_INTERLEAVED
};


enum scaler_ctl_code {
	SCALER_BY_PASS_Y,
	SCALER_BY_PASS_N,
	SCALER_CTL_EN,
	SCALER_DIS
};


typedef struct ofmt_t {
	enum pixel_fmt opix_fmt;
	enum store_fmt st_order;
	uint32_t opix_w;
	uint32_t opix_h;
	uint32_t winverofst;
	uint32_t winhorofst;
	uint8_t		codec_enable;
}outfmt_t;

//for video stream use 
struct camif_dma_buf {
	char dma_buf_saddr[CAMIF_DMA_BUF_LEN];
	struct list_head buf_node;
	int index;
};

typedef struct scaler{

	uint16_t	src_hsize;
	uint16_t	src_vsize;
	uint16_t	des_hsize;
	uint16_t	des_vsize;
	uint8_t		ctl;
	uint8_t		bypass;
	uint8_t		scaler_zir_algo;
	uint8_t		scaler_zor_algo;
}Scaler,*PScaler;

enum preview_st_order{
		PREVIEW_RGB_16BIT,
		PREVIEW_RGB_24BIT,
		PREVIEW_SPLANE_420,
		PREVIEW_SPLANE_422,
};

enum preview_pixel_fmt{
		PREVIEW_RGB_16BIT_555,
		PREVIEW_RGB_16BIT_565,
};

typedef struct preview{
	enum	preview_pixel_fmt  pixel_fmt;
	enum    preview_st_order   st_order;
	uint8_t					   bpp16_fmt;
	uint8_t					   bpp24_bl;//big/little edition
	uint8_t					   byte_swap;
	uint8_t					   word_swap;
	uint8_t					   enable;
}Preview,*PPreview;
	
struct camif_host {
	void *base;  //isp base register
	void *dvp_base;
	uint8_t log_level; //log level
	uint8_t dmmu_en;
	struct dma_use_table udma;
	struct isp_addr addr;
	Preview  preview;
	outfmt_t outfmt;
	Scaler   scaler;
	void *dma_buf_saddr;
	void *dma_buf_paddr;

	void *dma_buf_saddr2;
	void *dma_buf_paddr2;

	void *dma_buf_saddr_t;
	void *dma_buf_paddr_t;

	struct list_head cam_queue;
	struct list_head pre_queue;
};

int imapx_camif_cfg_clock(uint32_t camif_clock, uint32_t camo_clk);
int imapx_camif_probe(void);
void imapx_camif_cfg_addr(int index);
void imapx_camif_enable_capture(void);
void imapx_camif_disable_capture(void);
void imapx_camif_host_deinit(void);
void imapx_camif_host_init(void);
void imapx_camifdvp_init(void);
int  imapx_scanmode_interlaced(void);
int  imapx_camif_set_fmt(void);
void imapx_camif_dump_regs(void);
int imapx_camif_prepare_addr(void);
void imapx_camif_set_bit(uint32_t addr, int bits, int offset, uint32_t value);
int imapx_camif_register_test(void);
void imapx_camif_push_queue(struct camif_dma_buf *buf);
struct camif_dma_buf * imapx_camif_pop_queue(void);
void imapx_camif_push_pre_queue(struct camif_dma_buf *buf);
struct camif_dma_buf * imapx_camif_pop_pre_queue(void);
int	imapx_camif_config_scaler(void);
int imapx_camif_prepare_preview_addr(void);
int imapx_camif_config_preview(void);
void imapx_camif_cfg_preview_addr(int index);

extern struct camif_host g_camif_host;
extern struct sys_manager g_camif_sys;
#ifdef __cplusplus
}
#endif

#endif

