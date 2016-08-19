/***************************************************************************** 
** 
** Copyright (c) 2014~2112 ShangHai Infotm Ltd all rights reserved. 
** 
** Use of Infotm's code is governed by terms and conditions 
** stated in the accompanying licensing statement. 
** 
** camif register operation  file
**
** Revision History: 
** ----------------- 
** v1.0.1	sam@2014/10/20: first version.
**
*****************************************************************************/
#include <camera/camif/camif.h>
#include <camera/isp/isp.h>
#include <camera/csi/csi.h>
#include <common.h>
#include <camera/csi/log.h>
#include <camera/csi/csi_core.h>
#include <malloc.h>
struct camif_host g_camif_host;
struct sys_manager g_camif_sys;

struct camifdvp_config_t camifdvp_init_config ={
	.debugon = 1,
	.invclk = 0,
	.invvsync = 0,
	.invhsync = 0,
	.hmode = 0,
	.vmode = 0,
	.hnum = 0,
	.vnum = 0,
	.syncmask = 0,
	.syncode0 = 0,
	.syncode1 = 0,
	.syncode2 = 0,
	.syncode3 = 0,
	.hdlycnt = 0,
	.vdlycnt = 0,
};

struct camif_config_t camif_init_config= {
	.imgsource   = 0, /* 0: for RTL, MIPI, IDS inptut  1: for sensor input*/
	.dvpsel      = 0,
	.in_width	 = VGA_WIDTH,
	.in_height	 = VGA_HEIGHT,
	.xoffset	 = 0,
	.yoffset	 = 0,
	.itu601		 = 1,
	.uvoffset	 = 0,
	.interlaced  = 0,
	.yuv_sequence = CAMIF_INPUT_YUV_ORDER_YCbYCr,
	.invpclk	 = 0,
	.invvsync    = 0,
	.invhref     = 0,
	.invhref     = 0,
	.invfield    = 0,
	.c_enable    = 1,
	.c_width     = VGA_WIDTH,
	.c_height    = VGA_HEIGHT,
	.c_format    = CODEC_IMAGE_FORMAT_YUV420,
	.c_store_format = CODEC_STORE_FORMAT_PLANAR,
	.c_hwswap    = 0,
	.c_byteswap  = 0,
	.pre_enable  = 0,
	.pre_width   = VGA_WIDTH,
	.pre_height  = VGA_HEIGHT,
	.pre_store_format =	PREVIEW_SPLANAR_420,
	.pre_hwswap  = 0,
	.pre_byteswap = 0,
	.pre_bpp16fmt = PREVIEW_BPP16_RGB565,
	.pre_rgb24_endian = PREVIEW_BPP24BL_LITTLE_ENDIAN,
	.intt_type	 = CAMIF_INTR_SOURCE_DMA_DONE	

};

int imapx_camif_cfg_clock(uint32_t camif_clock, uint32_t camo_clk)
{
	
	//frist power on camif module
//	module_power_on(&g_camif_sys);
//	module_sys_reset(&g_camif_sys);
	module_enable("cam");
	module_reset("cam");
	writel(readl(CAMIF_SYSM_ADDR+0x20) | (camif_init_config.dvpsel << 2), CAMIF_SYSM_ADDR +0x20);
	//camera mclk
	return 0;
}

int imapx_camif_register_test(void)
{
	uint8_t data = 0x55;
	camif_write(&g_camif_host, CAMIF_CIWDOFST, data);
	data = camif_read(&g_camif_host, CAMIF_CIWDOFST);
	printf("Get camif register data:0x%x\n", data);
	return 0;
}

//initialize camif moudle
int imapx_camif_probe(void)
{
	g_camif_host.base = (void *)IMAPX_CAMIF_REG_BASE;
	g_camif_sys.base  = (void *)SYSMGR_CAMIF_BASE;
	g_camif_host.dvp_base = (void *)CAMIF_DVP_BASE_ADDR;
	g_camif_host.log_level = LOG_ERR;
	g_camif_host.outfmt.opix_fmt = PIX_FMT_NV12; 
	g_camif_host.outfmt.st_order = PIX_SEMP;
	g_camif_host.outfmt.opix_w = VGA_WIDTH;
	g_camif_host.outfmt.opix_h = VGA_HEIGHT;
	g_camif_host.outfmt.winverofst = 0;
	g_camif_host.outfmt.winhorofst = 0;
	g_camif_host.dmmu_en = 0;
	//g_camif_host.dma_buf_saddr = malloc(CAMIF_DMA_BUF_LEN);
	//imapx_camif_prepare_addr();
	
	return 0;
}

void imapx_camif_dump_regs(void)
{
	CSI_ERR("Reg[0 ,4, 8, 44 ]	%8x		%8x		%8x		%8x\n",
			camif_read(&g_camif_host, CAMIF_CISRCFMT), camif_read(&g_camif_host, CAMIF_CIWDOFST),
			camif_read(&g_camif_host, CAMIF_CIGCTRL), camif_read(&g_camif_host, CAMIF_CICOTRGSIZE));
	CSI_ERR("Reg[48, A0, A4, C4]	%8x		%8x		%8x		%8x\n",
			camif_read(&g_camif_host,CAMIF_CICOTRGFMT), camif_read(&g_camif_host,CAMIF_CIIMGCPT),
			camif_read(&g_camif_host,CAMIF_CICPTSTATUS),
			camif_read(&g_camif_host,CAMIF_CICOFIFOSTATUS));
	CSI_ERR("Reg[100, 104,108, 10c]	%8x		%8x		%8x		%8x\n",
			camif_read(&g_camif_host,CAMIF_CH0DMAFB1), camif_read(&g_camif_host,CAMIF_CH0DMACC1),
			camif_read(&g_camif_host,CAMIF_CH0DMAFB2), camif_read(&g_camif_host,CAMIF_CH0DMACC2));
	CSI_ERR("Reg[120,124,128,12c]	%8x		%8x		%8x		%8x\n",
			camif_read(&g_camif_host,CAMIF_CH1DMAFB1), camif_read(&g_camif_host,CAMIF_CH1DMACC1),
			camif_read(&g_camif_host,CAMIF_CH1DMAFB2), camif_read(&g_camif_host,CAMIF_CH1DMACC2));
	CSI_ERR("Reg[130,134,138,13c]	%8x		%8x		%8x		%8x\n",
			camif_read(&g_camif_host,CAMIF_CH1DMAFB3), camif_read(&g_camif_host,CAMIF_CH1DMACC3),
			camif_read(&g_camif_host,CAMIF_CH1DMAFB4), camif_read(&g_camif_host,CAMIF_CH1DMACC4));
	
	CSI_ERR("Reg[140,144,148,14c]	%8x		%8x		%8x		%8x\n",
			camif_read(&g_camif_host,CAMIF_CH2DMAFB1), camif_read(&g_camif_host,CAMIF_CH2DMACC1),
			camif_read(&g_camif_host,CAMIF_CH2DMAFB2), camif_read(&g_camif_host,CAMIF_CH2DMACC2));
	CSI_ERR("Reg[150,154,158,15c]	%8x		%8x		%8x		%8x\n",
			camif_read(&g_camif_host,CAMIF_CH2DMAFB3), camif_read(&g_camif_host,CAMIF_CH2DMACC3),
			camif_read(&g_camif_host,CAMIF_CH2DMAFB4), camif_read(&g_camif_host,CAMIF_CH2DMACC4));

	return;
}
void imapx_camif_set_bit(uint32_t addr, int bits, int offset, uint32_t value)
{
	uint32_t tmp;
	tmp = camif_read(&g_camif_host, addr);
	tmp = (value << offset) | (~(((1 << bits)- 1) << offset)  & tmp);
	camif_write(&g_camif_host, addr, tmp);
}

void imapx_camifdvp_set_bit(uint32_t addr, int bits, int offset, uint32_t value)
{
       uint32_t tmp;
       tmp = readl(g_camif_host.dvp_base  + addr);
       tmp = (value << offset) | (~(((1 << bits)- 1) << offset)  & tmp);
       writel(tmp, g_camif_host.dvp_base  + addr);
}


uint32_t imapx_camif_get_bit(uint32_t addr,	int bits, int offset)
{
	uint32_t tmp;
	tmp = camif_read(&g_camif_host, addr);
	tmp = (tmp >> offset) & ((1 << bits)- 1) ;
	return  tmp;
}

uint32_t imapx_camifdvp_get_bit(uint32_t addr, int bits, int offset)
{
       uint32_t tmp;
       tmp = readl(g_camif_host.dvp_base + addr);
       tmp = (tmp >> offset) & ((1 << bits)- 1) ;
       return  tmp;
}


void imapx_camif_mask_irq(uint32_t mbit) 
{
	uint32_t tmp;

	tmp = camif_read(&g_camif_host, CAMIF_CIGCTRL);
	tmp |= mbit;
	camif_write(&g_camif_host, CAMIF_CIGCTRL, tmp);
}

void imapx_camif_unmask_irq(uint32_t umbit) 
{
	uint32_t tmp;

	tmp = camif_read(&g_camif_host, CAMIF_CIGCTRL);
	tmp &= ~umbit;
	camif_write(&g_camif_host, CAMIF_CIGCTRL, tmp);
}

int  imapx_camif_set_fmt(void)
{
	/*
	 *config camif output source :use codec path no scaler
	 */

	//camif output config 
	switch (g_camif_host.outfmt.opix_fmt){
		case PIX_FMT_NV12:
			   /* YCBCR420 */
				//set codec target format
			 {
				 //set yuv420
				imapx_camif_set_bit(CAMIF_CICOTRGFMT, 1, CICOTRGFMT_ycc422,	
						CODEC_IMAGE_FORMAT_YUV420);
				//set store format
				imapx_camif_set_bit(CAMIF_CICOTRGFMT, 1, CICOTRGFMT_StoredFormat,
						g_camif_host.outfmt.st_order);
						//CODEC_STORE_FORMAT_SPLANAR);
				break;
			 }
		case PIX_FMT_YUYV:
				/* YCRCB422*/
				//set codec target format
			 {
				 //set yuv422
				imapx_camif_set_bit(CAMIF_CICOTRGFMT,
						1, CICOTRGFMT_ycc422,
						CODEC_IMAGE_FORMAT_YUV422);
				//set store format
				imapx_camif_set_bit(CAMIF_CICOTRGFMT,
						1, CICOTRGFMT_StoredFormat,
						g_camif_host.outfmt.st_order);
						//CODEC_STORE_FORMAT_SPLANAR);
				break;
			 }

		default:
			break;
	}
	CSI_INFO("camif driver :set code size: x = %d, y= %d\n",g_camif_host.outfmt.opix_w,
			g_camif_host.outfmt.opix_h);
	//set output size_t
	camif_write(&g_camif_host, CAMIF_CICOTRGSIZE,
			((g_camif_host.outfmt).opix_w << CICOTRGSIZE_TargetHsize) | 
			((g_camif_host.outfmt).opix_h << CICOTRGSIZE_TargetVsize));
	camif_write(&g_camif_host, CAMIF_CIWDOFST,
			((g_camif_host.outfmt).winverofst << CIWDOFST_WinVerOfst) | 
			((g_camif_host.outfmt).winhorofst << CIWDOFST_WinHorOfst)); 
	return 0;

}

int imapx_camif_prepare_addr(void)
{
	switch(g_camif_host.outfmt.st_order) {
	//should use output format, but this use input format, just for show a picture
		case  PIX_SEMP :
			{
				g_camif_host.addr.y = (long)g_camif_host.dma_buf_saddr;
				g_camif_host.addr.y_len = g_camif_host.outfmt.opix_w * g_camif_host.outfmt.opix_h;
				g_camif_host.udma.ch2 = 1;
				g_camif_host.addr.cb = (long)(g_camif_host.addr.y + g_camif_host.addr.y_len);
				//for yuv420sp or yuv422sp	
				switch(g_camif_host.outfmt.opix_fmt)
				{
					case PIX_FMT_NV12:
					case PIX_FMT_NV21:
						{
							g_camif_host.addr.cb_len = g_camif_host.addr.y_len >> 1;
							break;
						}
					default:
						{
							g_camif_host.addr.cb_len = g_camif_host.addr.y_len;
							break;
						}
				}

				g_camif_host.udma.ch1 = 1;
				g_camif_host.addr.cr = 0;
				g_camif_host.addr.cr_len = 0;
				g_camif_host.udma.ch0 = 0;
				break;
					
			}
		case PIX_PLANAR :
			{
				g_camif_host.addr.y = (long)g_camif_host.dma_buf_saddr;
				g_camif_host.addr.y_len = g_camif_host.outfmt.opix_w * g_camif_host.outfmt.opix_h;
				g_camif_host.udma.ch2 = 1;
				g_camif_host.addr.cb = (long)(g_camif_host.addr.y + g_camif_host.addr.y_len);
				//for yuv420p yuv422p	
				switch(g_camif_host.outfmt.opix_fmt)
				{
					case PIX_FMT_NV12:
					case PIX_FMT_NV21:
						{
							g_camif_host.addr.cb_len = g_camif_host.addr.y_len >> 2;
							g_camif_host.addr.cr_len = g_camif_host.addr.y_len >> 2;
							break;
						}
					default:
						{
							g_camif_host.addr.cb_len = g_camif_host.addr.y_len >> 1;
							g_camif_host.addr.cr_len = g_camif_host.addr.y_len >> 1;
							break;
						}
				}
				g_camif_host.udma.ch1 = 1;

				g_camif_host.addr.cr = (long)(g_camif_host.addr.cb + g_camif_host.addr.cb_len);
				g_camif_host.udma.ch0 = 1;	
				break;

			}
		case PIX_INTERLEAVED: //only for yuv422
			{
				g_camif_host.addr.y = (long)g_camif_host.dma_buf_saddr;
				g_camif_host.addr.y_len = g_camif_host.outfmt.opix_w * g_camif_host.outfmt.opix_h << 1;
				g_camif_host.udma.ch2 = 1;
				g_camif_host.udma.ch1 = 0;
				g_camif_host.addr.cb = 0;
				g_camif_host.addr.cb_len = 0;
				g_camif_host.addr.cr = 0;
				g_camif_host.addr.cr_len = 0;
				g_camif_host.udma.ch0 = 0;
				break;
			}
		default:
			break;

 	}

	return 0;
}

void imapx_camif_cfg_addr(int index)
{
	u32 len = 0;
	
	if(g_camif_host.addr.cr_len && g_camif_host.udma.ch0){
		camif_write(&g_camif_host, CAMIF_CH0DMA_FB(index), g_camif_host.addr.cr);
		imapx_camif_set_bit(CAMIF_CH0DMA_CC(index),
						  25, 0, (((g_camif_host.addr).cr_len) >> 3));
	}

	if(g_camif_host.addr.cb_len && g_camif_host.udma.ch1){
		camif_write(&g_camif_host, CAMIF_CH1DMA_FB(index),(g_camif_host.addr.cb));
		len = (g_camif_host.addr.cb_len) >> 3;
		imapx_camif_set_bit(CAMIF_CH1DMA_CC(index),
						  25, 0, len);//(( (buf->paddr).cb_len) >> 3));

	}

	if(g_camif_host.addr.y_len && g_camif_host.udma.ch2){
		camif_write(&g_camif_host, CAMIF_CH2DMA_FB(index),(g_camif_host.addr.y));
		len = (g_camif_host.addr.y_len) >> 3;
		imapx_camif_set_bit(CAMIF_CH2DMA_CC(index), 
						  25, 0, len);
	}

}

void imapx_camif_enable_capture(void)
{

	int idx;

	for(idx = 0; idx < 4; idx++){
		if(g_camif_host.udma.ch0){   //set dma buffer autoload, next frame enable
#if defined(MACRO_FOR_DEMO_USED)			
			imapx_camif_set_bit(CAMIF_CH0DMA_CC(idx), 3,
					CICHxALTCTRL_Dir, 0x07);
#else
			imapx_camif_set_bit(CAMIF_CH0DMA_CC(idx), 3,
					CICHxALTCTRL_Dir, 0x03);
#endif
		}

		if(g_camif_host.udma.ch1){
#if defined(MACRO_FOR_DEMO_USED)		
			imapx_camif_set_bit(CAMIF_CH1DMA_CC(idx), 3,
					CICHxALTCTRL_Dir, 0x07);
#else
			imapx_camif_set_bit(CAMIF_CH1DMA_CC(idx), 3,
					CICHxALTCTRL_Dir, 0x03);
#endif
		}

		if(g_camif_host.udma.ch2){
#if defined(MACRO_FOR_DEMO_USED)		
			imapx_camif_set_bit(CAMIF_CH2DMA_CC(idx), 3,
					CICHxALTCTRL_Dir, 0x03);
#else
			imapx_camif_set_bit(CAMIF_CH2DMA_CC(idx), 3,
					CICHxALTCTRL_Dir, 0x03);
#endif

		}
	}

	//enable dma channel
	if(g_camif_host.udma.ch0){
		imapx_camif_set_bit(CAMIF_CH0DMA_CC(0), 1,
				CICHxCTRL_DMAEn, 0x01);
	}

	if(g_camif_host.udma.ch1){	
		imapx_camif_set_bit(CAMIF_CH1DMA_CC(0), 1,
				CICHxCTRL_DMAEn, 0x01);
	}
	
	if(g_camif_host.udma.ch2){	
		imapx_camif_set_bit(CAMIF_CH2DMA_CC(0), 1,
				CICHxCTRL_DMAEn, 0x01);
	}
	

	//enable camif and code path 
	imapx_camif_set_bit(CAMIF_CIIMGCPT, 3, 
				CIIMGCPT_ImgCptEn_PrSc, 0x06);	

}

void imapx_camif_disable_capture(void)
{
	//disable dma channel

	if(g_camif_host.udma.ch0){
		imapx_camif_set_bit(CAMIF_CH0DMA_CC(0), 2,
				CICHxCTRL_RST, 0x01);
	}

	if(g_camif_host.udma.ch1){	
		imapx_camif_set_bit(CAMIF_CH1DMA_CC(0), 2,
				CICHxCTRL_RST, 0x01);
	}
	
	if(g_camif_host.udma.ch2){	
		imapx_camif_set_bit(CAMIF_CH2DMA_CC(0), 2,
				CICHxCTRL_RST, 0x01);
	}
	
	//disable camif and code path 
	imapx_camif_set_bit(CAMIF_CIIMGCPT, 3, 
				CIIMGCPT_ImgCptEn_PrSc, 0x00);	
}


void imapx_camif_host_deinit(void)
{
	//disable camif
	imapx_camif_set_bit(CAMIF_CIIMGCPT, 1, CIIMGCPT_CAMIF_EN, 0);
	imapx_camif_set_bit(CAMIF_CIIMGCPT, 1, CIIMGCPT_ImgCptEn_CoSc, 0);
	imapx_camif_set_bit(CAMIF_CIIMGCPT, 1, CIIMGCPT_ImgCptEn_PrSc, 0);
	//power off camif module
	module_disable("cam");
	if(NULL != g_camif_host.dma_buf_saddr){
		if(!g_camif_host.dmmu_en) {
			free(g_camif_host.dma_buf_saddr);
			g_camif_host.dma_buf_saddr = NULL;
		}
	}
	return;

}

void imapx_camif_host_init(void)
{

	/* camif soft reset*/
	imapx_camif_set_bit(CAMIF_CIGCTRL, 1, CIGCTRL_SwRst, 1); 
	//udelay(5000);
	imapx_camif_set_bit(CAMIF_CIGCTRL, 1, CIGCTRL_SwRst, 0);
	// set camif  x,y offset ,input w,h
	camif_write(&g_camif_host, CAMIF_CIWDOFST, 
				(camif_init_config.xoffset << CIWDOFST_WinHorOfst) |
				(camif_init_config.yoffset << CIWDOFST_WinVerOfst));
	//set camif input format 
	camif_write(&g_camif_host, CAMIF_CISRCFMT,
				(camif_init_config.itu601 << CISRCFMT_ITU601or656) |
				(camif_init_config.uvoffset << CISRCFMT_UVOffset) |
			//	(camif_init_config.interlaced << CISRCFMT_ScanMode) |
				(0 << CISRCFMT_ScanMode) |
				(camif_init_config.yuv_sequence << CISRCFMT_Order422));
	
	//set camif interrupt and signal polarity
	camif_write(&g_camif_host, CAMIF_CIGCTRL, (1 << CIGCTRL_IRQ_OvFiCH4_en) |
			(1 << CIGCTRL_IRQ_OvFiCH3_en) | (1 << CIGCTRL_DEBUG_EN) |
			(1 << CIGCTRL_IRQ_OvFiCH2_en) | (1 << CIGCTRL_IRQ_en) |
			(1 << CIGCTRL_IRQ_OvFiCH1_en) | (1 << CIGCTRL_IRQ_Bad_SYN_en) | 
			(1 << CIGCTRL_IRQ_OvFiCH0_en) | 
			(camif_init_config.intt_type << CIGCTRL_IRQ_Int_Mask_Pr) |
			(camif_init_config.intt_type << CIGCTRL_IRQ_Int_Mask_Co) |
			(camif_init_config.invpclk   << CIGCTRL_InvPolCAMPCLK)   |
			(camif_init_config.invvsync  << CIGCTRL_InvPolCAMVSYNC)  |
			(camif_init_config.invhref   << CIGCTRL_InvPolCAMHREF)   |
			(camif_init_config.invfield  << CIGCTRL_InvPolCAMFIELD)); 
	
	// set codec output 
	
	camif_write(&g_camif_host, CAMIF_CICOTRGSIZE,
			(camif_init_config.c_width << CICOTRGSIZE_TargetHsize) |
			(camif_init_config.c_height << CICOTRGSIZE_TargetVsize));
	
	return;
}

void imapx_camifdvp_init(void) {
	/* wrapper bypass */
	if(camifdvp_init_config.hmode != 0x2 && camifdvp_init_config.hmode != 0x3
			&& camifdvp_init_config.vmode != 0x1) {
		printf("Camif DVP wrapper bypass!!\n");
		return ;
	}
	printf("Camif DVP wrapper begin to config!!\n");

	writel((camifdvp_init_config.debugon << 3)  | 
			(camifdvp_init_config.invclk << 2) |
			(camifdvp_init_config.invvsync) << 1 |
			camifdvp_init_config.invhsync , g_camif_host.dvp_base + 0);

	writel((camifdvp_init_config.hmode << 4) |
			camifdvp_init_config.vmode, g_camif_host.dvp_base + 0x4);

	writel(((camifdvp_init_config.hnum -1) << 16) |
			(camifdvp_init_config.vnum -1), g_camif_host.dvp_base + 0x8);

	writel(camifdvp_init_config.syncmask, g_camif_host.dvp_base + 0xc);
	writel(camifdvp_init_config.syncode0, g_camif_host.dvp_base + 0x10);
	writel(camifdvp_init_config.syncode1, g_camif_host.dvp_base + 0x14);
	writel(camifdvp_init_config.syncode2, g_camif_host.dvp_base + 0x18);
	writel(camifdvp_init_config.syncode3, g_camif_host.dvp_base + 0x1c);

	writel(((camifdvp_init_config.hdlycnt) << 16) |
			(camifdvp_init_config.vdlycnt), g_camif_host.dvp_base + 0x20);
}

int imapx_scanmode_interlaced(void)
{
	/*
	return (camif_init_config.interlaced && 
					imapx_camif_get_bit(CAMIF_CISRCFMT, 1,CISRCFMT_ScanMode));
					*/
	return camif_init_config.interlaced;
}

void imapx_camif_push_queue(struct camif_dma_buf *buf)
{
	list_add_tail(&(buf->buf_node), &(g_camif_host.cam_queue));
	return;
}

struct camif_dma_buf * imapx_camif_pop_queue(void)
{
	struct camif_dma_buf *buf = list_first_entry(&(g_camif_host.cam_queue),
					struct camif_dma_buf, buf_node);
	list_del(&(buf->buf_node));
	return buf;
}

void imapx_camif_push_pre_queue(struct camif_dma_buf *buf)
{
	list_add_tail(&(buf->buf_node), &(g_camif_host.pre_queue));
	return;
}

struct camif_dma_buf * imapx_camif_pop_pre_queue(void)
{
	struct camif_dma_buf *buf = list_first_entry(&(g_camif_host.pre_queue),
					struct camif_dma_buf, buf_node);
	list_del(&(buf->buf_node));
	return buf;
}

















