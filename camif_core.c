/*****************************************************************************
**
** Copyright (c) 2014~2112 ShangHai Infotm Ltd all rights reserved.
**
** Use of Infotm's code is governed by terms and conditions
** stated in the accompanying licensing statement.
**
** camif core layer file
**
** Revision History:
** -----------------
** v1.0.1	sam@2014/10/20: first version.
**
*****************************************************************************/
#include <camera/camif/camif_core.h>
#include <camera/camif/camif_reg.h>
#include <camera/csi/csi_core.h>
#include <irq.h>
#include <camera/sensor/gc0308.h>
#include <malloc.h>
#include <dmmu.h>
#include <ids/ids_api.h>
#include <vstorage.h>
#include <venc/h264-api.h>
#define USE_CAMIF_REST_BIT
#define yuv_file ("camif/camif_verify_file.yuv")
#define even_file ("camif/camif_even_file.yuv")
#define odd_file ("camif/camif_odd_file.yuv")
volatile int intt_fd = 0,even_fd = 0, odd_fd;
extern volatile int ret_flag;
extern volatile int venc_flag;
extern volatile unsigned long enc_frames; 
extern void *handler;
extern volatile unsigned long offset;
extern void *outBuf;
static struct camif_dma_buf *camif_buf = NULL;
extern volatile int nframe;
static int count = 0;

int camif_irq_handle(int irq, void *arg)
{
	uint32_t status = 0;
	static	uint32_t nframe = 0;
	status = camif_read(&g_camif_host, CAMIF_CICPTSTATUS);
	//printf("get status: 0x%x\n", status);
	//one frame dma transfer done
#if 1
	if (nframe == 2)
	{
		uint32_t H = 0, W = 0;
		W =  camif_read(&g_camif_host, CAMIF_INPUT_SUMPIXS);
		printf("Get input active sum pixels : %d\n", W);
		
		W = (camif_read(&g_camif_host, CAMIF_ACTIVE_SIZE) >> 16) & 0xFFFF;
		H = (camif_read(&g_camif_host, CAMIF_ACTIVE_SIZE)) & 0xFFFF;
		printf("Get input active picture : width = %d height = %d\n", W, H);

	}
#endif
	count++;

	if ((status & (1 << CICPTSTATUS_C_PATH_DMA_SUCCESS))&& 
		(!(status & (1 << CICPTSTATUS_C_PATH_FIFO_DIRTY_STATUS))))
	{
		printf("get status: 0x%x\n", status);
		nframe++;
		if(imapx_scanmode_interlaced()) {
			printf(" status-0x%x Frame %d,Field %s should be %s\n",
							status,nframe, (status & 0x10) ? "Even": "Odd",
							(nframe % 2) ? "Odd": "Even");

			if(2 == nframe) {
				/*
				 * with the speed limit of file write, we need change the dma addr if want capture the continued frame 
				 */
				camif_write(&g_camif_host, CAMIF_CICPTSTATUS, status);
				write(odd_fd, (void *)g_camif_host.dma_buf_paddr, 
						g_camif_host.addr.y_len + g_camif_host.addr.cb_len + g_camif_host.addr.cr_len);
				close(odd_fd);

				/*
				g_camif_host.dma_buf_saddr_t = g_camif_host.dma_buf_saddr; 
				g_camif_host.dma_buf_paddr_t = g_camif_host.dma_buf_saddr;
				g_camif_host.dma_buf_saddr = g_camif_host.dma_buf_saddr2;
				g_camif_host.dma_buf_paddr = g_camif_host.dma_buf_saddr2; 
				imapx_camif_prepare_addr();
				imapx_camif_cfg_addr(0);
				*/
				printf("a good frame in interrupt status :0x%x\n", status);
			}
			if(3 == nframe) {

				camif_write(&g_camif_host, CAMIF_CICPTSTATUS, status);
				imapx_camif_disable_capture();
				write(even_fd, (void *)g_camif_host.dma_buf_paddr, 
						g_camif_host.addr.y_len + g_camif_host.addr.cb_len + g_camif_host.addr.cr_len);
				close(even_fd);
				printf("a good frame in interrupt status :0x%x\n", status);
				/*
				write(odd_fd, (void *)g_camif_host.dma_buf_paddr_t, 
						g_camif_host.addr.y_len + g_camif_host.addr.cb_len + g_camif_host.addr.cr_len);
				close(odd_fd);
				*/
				printf("Get picture frame finished in interrupt\n");
				ret_flag = 1;
			}
		}
#ifdef CONFIG_COMPILE_RTL
		else if(3 == nframe) {
			camif_write(&g_camif_host, CAMIF_CICPTSTATUS, status);
			imapx_camif_disable_capture();
			write(intt_fd, (void *)g_camif_host.dma_buf_paddr, 
				g_camif_host.addr.y_len + g_camif_host.addr.cb_len + g_camif_host.addr.cr_len);
			//imapx_camif_disable_capture();
			close(intt_fd);
			imapx_camif_enable_capture();
			printf("a good frame in interrupt status :0x%x\n", status);
			printf("Get picture frame finished in interrupt\n");
			ret_flag = 1;
		}
#else
		else if(10 == nframe) {
			imapx_camif_disable_capture();
			printf("a good frame in interrupt status :0x%x\n", status);
			write(intt_fd, (void *)g_camif_host.dma_buf_paddr, 
				g_camif_host.addr.y_len + g_camif_host.addr.cb_len + g_camif_host.addr.cr_len);
#ifdef CONFIG_COMPILE_FPGA
			//power down gc0308 sensor
			gpio_set_output_val(PDOWN_FRONT_PIN, 1);
#endif
			close(intt_fd);
			printf("Get picture frame finished in interrupt\n");
			ret_flag = 1;
		}
#endif
	}
	//clear interrupt
	camif_write(&g_camif_host, CAMIF_CICPTSTATUS, status);
	return 0;
}

int camif_irq_handle_with_encoder(int irq, void *arg)
{
	uint32_t status = 0;	
	uint32_t index = 0;
	status = camif_read(&g_camif_host, CAMIF_CICPTSTATUS);
	//printf("interrupt status :0x%x\n", status);
	//one frame dma transfer done
	if ((status & (1 << CICPTSTATUS_C_PATH_DMA_SUCCESS))&& 
		(!(status & (1 << CICPTSTATUS_C_PATH_FIFO_DIRTY_STATUS))))
	{
		printf("a good frame in interrupt status :0x%x\n", status);
		//a good frame ok then  push dma buffer to queue for encode
		if(!(list_empty(&(g_camif_host.pre_queue)))) {
			imapx_camif_push_queue(camif_buf);
			index = (camif_read(&g_camif_host, CAMIF_CH2DMACC1) >> 28) & 0x03;	
			//printf("dma current transfer frame %d, after=%d\n", index, index%2);
			index = index % 2;
			//config new dma buffer for camif 
			camif_buf = imapx_camif_pop_pre_queue();
			g_camif_host.dma_buf_saddr = (void *)(camif_buf->dma_buf_saddr);
			imapx_camif_prepare_addr();
			imapx_camif_cfg_addr(index);
			imapx_camif_cfg_addr(index + 2);
			if( nframe == enc_frames ) {
				ret_flag = 1;
				imapx_camif_disable_capture();
				nframe = 1;
			}
			nframe++;
		}	
	}
	//clear interrupt
	camif_write(&g_camif_host, CAMIF_CICPTSTATUS, status);
	return 0;
}

int camif_irq_handle_with_ids(int irq, void *arg)
{
	uint32_t status = 0;

	status = camif_read(&g_camif_host, CAMIF_CICPTSTATUS);
	//printf("interrupt status :0x%x\n", status);
	//one frame dma transfer done
	if ((status & (1 << CICPTSTATUS_C_PATH_DMA_SUCCESS))&& 
		(!(status & (1 << CICPTSTATUS_C_PATH_FIFO_DIRTY_STATUS))))
	{
			//set ids dma address ,send the picture data in memory buffer to ids
			ids_set_dma_addr((uint8_t *)g_camif_host.addr.y, (uint8_t *)g_camif_host.addr.cb);
	}
	//clear interrupt
	camif_write(&g_camif_host, CAMIF_CICPTSTATUS, status);

	return 0;
}

error_t camif_open(void)
{
	//imapx_camif_probe(); //initialize camif module
	//request camif irq
	//request_irq(CAMIF_INTR_NUM, camif_irq_handle, "camif");
	imapx_camif_cfg_clock(CAMIF_CLK_DFT, CAMERA_OCLK_DFT);
	int clk;
	clk = module_get_clock("bus1");
	printf("get camif clock : %d\n", clk);
	//set dma buffer address
#if defined(MACRO_FOR_DEMO_USED)	
	int i = 0;
	for( i = 0; i < 3; i++) {
		camif_buf = (struct camif_dma_buf *)malloc(sizeof(struct camif_dma_buf));
		if(NULL == camif_buf) {
			printf("malloc camif dma buffer in camif handle with encoder failed\n");
			return 0;
		}
		camif_buf->index = i;
		//push to prepare queue
		imapx_camif_push_pre_queue(camif_buf);
		camif_buf = NULL;
		printf("malloc camif buffer ok!\n");
	}
	//shold config first frame addr to camif 
	camif_buf = imapx_camif_pop_pre_queue();
	nframe = 1;
	//config new dma buffer for camif 
	g_camif_host.dma_buf_saddr = (void *)(camif_buf->dma_buf_saddr);
	
#else	
	if(g_camif_host.dmmu_en) {
		printf("running camif dmmu enable.\n");
		dmmu_enable("cam");
		g_camif_host.dma_buf_paddr = malloc(CAMIF_DMA_BUF_LEN);
		g_camif_host.dma_buf_saddr = (void *)dmmu_map("cam", (uint32_t)g_camif_host.dma_buf_paddr, CAMIF_DMA_BUF_LEN);
	}
	else {
		g_camif_host.dma_buf_saddr = malloc(CAMIF_DMA_BUF_LEN);
		g_camif_host.dma_buf_paddr = g_camif_host.dma_buf_saddr;
		g_camif_host.dma_buf_saddr2 = malloc(CAMIF_DMA_BUF_LEN);
		g_camif_host.dma_buf_paddr2 = g_camif_host.dma_buf_saddr2;
	}
	
#endif	
	imapx_camifdvp_init();
	imapx_camif_host_init();
	imapx_camif_set_fmt();
	imapx_camif_prepare_addr();
#if defined(MACRO_FOR_DEMO_USED)
	for (i = 0; i < 3; i++) {
		imapx_camif_cfg_addr(i);
	}
#else	
	//for (i = 0; i < 4; i++) {
	//	imapx_camif_cfg_addr(i);
	//}

	imapx_camif_cfg_addr(0);
#endif	
	//enable_irq(CAMIF_INTR_NUM);
	//open file to store one frame
	if(imapx_scanmode_interlaced()){
		even_fd = open(even_file, O_WRONLY | O_CREAT);
		if(0 > even_fd) {
			CSI_ERR("open even_file error\n");
			return -1;
		}
		odd_fd = open(odd_file, O_WRONLY | O_CREAT);
		if(0 > odd_fd) {
			CSI_ERR("open odd_file error\n");
			return -1;
		}
	} else {
		intt_fd = open(yuv_file, O_WRONLY | O_CREAT);
		if(0 > intt_fd) {
			CSI_ERR("open yuv_file error\n");
			return -1;
		}
	}

	return SUCCESS;
}

error_t camif_close(void)
{
	imapx_camif_host_deinit();
	disable_irq(CAMIF_INTR_NUM);
	if(intt_fd > 0) {
		close(intt_fd);
	}
	if(even_fd > 0) {
		close(even_fd);
	}
	if(odd_fd > 0) {
		close(odd_fd);
	}
	return SUCCESS;
}

error_t camif_stream_start(void)
{
#if 0
	uint32_t status = 0;
	int fd = 0;
	uint32_t nframe = 0;
	fd = open(yuv_file, O_WRONLY | O_CREAT);
	if(0 > fd) {
		CSI_ERR("open yuv_file error\n");
		return -1;
	}
#endif
	//start camif
	imapx_camif_enable_capture();
#ifdef CONFIG_COMPILE_RTL
	//add for rtl verification only use as a assert for external DG enable.
#if defined(USE_CAMIF_REST_BIT)
	gpio_set_mode(RESET_PIN, RESET_PIN, GPIO_MODE_FUNC, 0);
	imapx_camif_set_bit(CAMIF_CIGCTRL, 1, CIGCTRL_CamRst, 1);
#else	
	//pads_set_value(RESET_PIN, 1);
	gpio_set_output_val(RESET_PIN, 1);
#endif	

#endif

#if 0
	//poll get picture
	while(1){
	status = camif_read(&g_camif_host, CAMIF_CICPTSTATUS);
	//one frame dma transfer done
	if (status &( 1 << CICPTSTATUS_C_PATH_DMA_SUCCESS))
	{

		nframe ++;
		printf("get frame :%d\n", nframe);
		if (5 == nframe) {
			write(fd, (void *)g_camif_host.addr.y, g_camif_host.addr.y_len + g_camif_host.addr.cb_len + g_camif_host.addr.cr_len);
			break;
		}
		//imapx_camif_disable_capture();
	}
	//clear interrupt
	camif_write(&g_camif_host, CAMIF_CICPTSTATUS, status);
	}
	close(fd);
#endif
	return SUCCESS;
}

error_t camif_stream_off(void)
{
	//power down sensor
	imapx_camif_disable_capture();
	return SUCCESS;
}
