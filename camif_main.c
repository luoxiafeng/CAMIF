/* ****************************************************************************
 * **
 * ** Copyright (c) 2014~2112 ShangHai Infotm Ltd all rights reserved.
 * **
 * ** Use of Infotm's code is governed by terms and conditions
 * ** stated in the accompanying licensing statement.
 * **
 * ** Soc FPGA verify: csi_main.c :offer  csi module test items operation
 * **
 * ** Revision History:
 * ** -----------------
 * ** v0.0.1	sam.zhou@2014/06/23: first version.
 * **
 * *****************************************************************************/
#include <common.h>
#include <command.h>
#include <testbench.h>
#include <camera/camif/camif_core.h>
#include <camera/csi/csi_core.h>
#include <iic.h>
#include <camera/sensor/gc0308.h>
#include <camera/sensor/imx322.h>
#include <camera/isp/isp.h>
#include <irq.h>
#include <ids/ids_api.h>
#include <venc/h264-api.h>
#include <venc/h265-api.h>
#include <vstorage.h>
#include <malloc.h>
#include <mmc.h>
#include <isp/V2500/isp_define.h>

#define CSI_LOG_LEVEL (LOG_INFO)
#define CSI_LANES     (2)
#define CSI_CMD_LEN	  (2)
#define CSI_FREQ	  (240)
#define I2C_CHN_IM    (3)
//define csi test command
#define HP	("hp")
#define C1	("c1")
#define C2	("c2")
#define C3	("c3")
#define C4	("c4")
#define C5	("c5")
//#define ME  ("me")
#define C6	("c6")
#define C7	("c7")
#define C8	("c8")
#define C9	("c9")
#define C10	("ca")
#define C11	("cb")
#define C12	("cc")
#define C13 ("cd")
#define C14 ("sn")
#define RW  ("rw")
#define DS  ("ds")
#define EC	("ec")
#define ES	("es")
//The test run items
//No command: print csi test case help
#define exp1 ("display all test command")
#define exp2 ("itu601 ycbcr420 sp 1920*6 interrupt in:crycby")
#define exp3 ("itu601, ycbcr422, planar, resolution 1920x6, out file order : crycby")
#define exp4 ("itu601, ycbcr422,sp,resolution 1920x6, out file order : crycby")
#define exp5 ("itu656, ycbcr422, planar, resolution 1920x6, out file order : crycby")
#define exp6 ("itu601, ycbcr422, interlaced, resolution 1920x6, out file order : crycby")
#define exp7 ("itu601 ycbcr420 sp 1920*6 interrupt in:crycby, dmmu enable")
#define exp8 ("add later")
#define exp9 ("read and write camif registers")
#define exp10 ("display camera picture with lcd screen")
#define exp11 ("encode h265 video from camera")
#define exp12 ("ecode h264 video stream from camera")
#define exp13 ("itu601 ycbcr420 sp 1920*6 interrupt in:crycby")
#define exp14 ("itu601&itu656 ycbcr422p 720*576 interlaced in:crycby")
#define exp15 ("test mipi with camif")
#define exp16 ("camif dvp wraper system delay mode test")
#define exp17 ("test sony imx322 with camif on FPGA")

#define COMMAND_LIST_ENTRY(cmd, exp) {cmd, exp},
#define COMMAND_LIST \
	COMMAND_LIST_ENTRY(HP, exp1) \
	COMMAND_LIST_ENTRY(C1, exp2) \
	COMMAND_LIST_ENTRY(C2, exp3) \
	COMMAND_LIST_ENTRY(C3, exp4) \
	COMMAND_LIST_ENTRY(C4, exp5) \
	COMMAND_LIST_ENTRY(C5, exp6) \
	COMMAND_LIST_ENTRY(C6, exp7) \
	COMMAND_LIST_ENTRY(C7, exp8) \
	COMMAND_LIST_ENTRY(RW, exp9) \
	COMMAND_LIST_ENTRY(DS, exp10) \
	COMMAND_LIST_ENTRY(EC, exp11) \
	COMMAND_LIST_ENTRY(ES, exp12) \
	COMMAND_LIST_ENTRY(C10, exp13) \
	COMMAND_LIST_ENTRY(C11, exp14) \
	COMMAND_LIST_ENTRY(C12, exp15) \
	COMMAND_LIST_ENTRY(C13, exp16) \
	COMMAND_LIST_ENTRY(C14, exp17) \

#define CMD_NUM (10)
#define CMD_WID (2)

extern struct camif_config_t camif_init_config;
extern struct camifdvp_config_t camifdvp_init_config;
extern int csi_config_test(int width, int height, int csi_fmt);
static const char *g_camif_cmd[][2] = {COMMAND_LIST};
volatile int ret_flag = 0;
volatile int venc_flag = 0;
volatile unsigned long enc_frames = 0;
void *outBuf = NULL;
void *handler = NULL;
volatile unsigned long offset = 0;
extern volatile int intt_fd;
volatile int nframe = 0;

void camif_usage(void)
{
	printf("Usage for camif testbench:\n");
	printf("\tcamif command [argument]:\n");
	printf("\texample: camif c1\n");
}


static int atoi(int *pint, const char *pchar, uint8_t hex)
{
	int sum = 0;
	if(10 == hex) {
		while(*pchar != '\0') {

			sum = sum*10 + (*pchar - 48);
			pchar++;
		}
		*pint = sum;
	}
	else if (16 == hex) {
		if(0 == strncmp("0x", pchar, 2)) {
			pchar += 2;
			while(*pchar != '\0') {
				switch (*pchar) {
					case 'A':
				    case 'B':
					case 'C':
					case 'D':
					case 'E':
					case 'F':
					 	{
							sum = sum*16 + (*pchar - 55);
							break;
						}
					case 'a':
					case 'b':
					case 'c':
					case 'd':
					case 'e':
				    case 'f':

						{
							sum = sum*16 + (*pchar - 87);
							break;
						}
					default:
						{
							sum = sum*16 + (*pchar - 48);
							break;
						}
				}
				pchar++;

			}
			*pint = sum;

		}
		else {
			printf("Hex is not valid: suffix:[0x] example: 0x12344\n");

			return -1;
		}
	}
	printf("dec sum = %d ;;;; hex sum =%x\n", sum ,sum);
	return 0;
}

int camif_hp_command(void)
{
	uint8_t i,j;
	for(i = 0; i < (sizeof(g_camif_cmd)/sizeof(char *))/2; i++) {
		for(j = 0; j < CMD_WID; j++) {
				printf("%s	", g_camif_cmd[i][j]);
			}
			printf("\n");
	}

	return 0;
}

static int run_camif_rtl_case(void)
{
	//initialize case input & output format
	//open camif device
	camif_open();
	//power on sensor
#if SENSOR_SOURCE == IMX322
	imx322_power_on();
#else
	gc0308_power_on();
#endif
	//sensor initialize
	if(camif_init_config.imgsource) {
		i2c_init(I2C_CHN_IM, 100000);
		i2c_init(PMU_I2C_CHN, 100000);
#if SENSOR_SOURCE == IMX322
		if(imx322_id_check()){
			//init sensor gc0308
			imx322_init();
			printf("sensor init finished\n");
		}else {
			CSI_ERR("image sensor imx322 is not work.\n");
			return -1;
		}
#else
		if(gc0308_id_check()){
			//init sensor gc0308
			gc0308_init();
			printf("sensor init finished\n");
		}else {
			CSI_ERR("image sensor gc0308 is not work.\n");
			return -1;
		}
#endif
	}
#if SENSOR_SOURCE == IMX322
	imx322_enable(1);
	//enable dvp wraper
	writel(0x10, g_camif_host.dvp_base + 0);
#endif
	//camif stream start
	camif_stream_start();
	while(1) {
		if(1 == ret_flag){
			printf("dump picture data from memory to rtl_data.bin\n");
			return 1;
		}
	}

	return 0;
}

static int camif_capture_config(void)
{
	//initialize i2c for sensor
	i2c_init(I2C_CHN_IM, 100000);
	//i2c_init(PMU_I2C_CHN, 100000);
	//open camif device
	camif_open();
	//power on sensor
	gc0308_power_on();
	//camif stream start
	if(gc0308_id_check()){
		//init sensor gc0308
		gc0308_init();
	}else {
		CSI_ERR("image sensor gc0308 is not work.\n");
		return -1;
	}
	return 0;

}

int camif_test_main(int argc, char * argv[])
{
	if(1 >= argc) {
		camif_usage();
		return 0;
	}

	static uint8_t flag = 0;
	if(flag == 0)
	{
		//initialize camif module
		imapx_camif_probe();
		//register interrupt
		request_irq(CAMIF_INTR_NUM, camif_irq_handle, "camif");
		//initialize mmc
		init_mmc();
		//set iopad for camera data2~5
		int val = 0;
		val = readl(PAD_SYSM_ADDR);
		val &= ~0x01;
		writel(val, PAD_SYSM_ADDR);

		flag = 1;
	}
	if(0 == strncmp(HP, argv[1], CSI_CMD_LEN)) {
		//display command help
		//TODO
		camif_hp_command();
	}
	else if(0 == strncmp(C1, argv[1], CSI_CMD_LEN)) { //ncase5
		/*
		*(1)设置host的输出图像格式：NV12
		*(2)设置host的输出图像大小：
		*(3)设置原图像的个数：1
		*/
		g_camif_host.outfmt.opix_fmt = PIX_FMT_NV12;
		g_camif_host.outfmt.st_order = PIX_SEMP;
#ifdef CONFIG_COMPILE_RTL
		g_camif_host.outfmt.opix_w = 1920;
		g_camif_host.outfmt.opix_h = 6;
#else
		g_camif_host.outfmt.opix_w = VGA_WIDTH;
		g_camif_host.outfmt.opix_h = VGA_HEIGHT;
		camif_init_config.imgsource = 1;
#endif
		return 	run_camif_rtl_case();

	}
	else if(0 == strncmp(C2, argv[1], CSI_CMD_LEN)) { //ncase0
		g_camif_host.outfmt.opix_fmt = PIX_FMT_YUYV;
		g_camif_host.outfmt.st_order = PIX_PLANAR;
#ifdef CONFIG_COMPILE_RTL
		g_camif_host.outfmt.opix_w = 1920;
		g_camif_host.outfmt.opix_h = 6;
#else
		g_camif_host.outfmt.opix_w = VGA_WIDTH;
		g_camif_host.outfmt.opix_h = VGA_HEIGHT;
		camif_init_config.imgsource = 1;
#endif
		return run_camif_rtl_case();

	}
	else if(0 == strncmp(C3, argv[1], CSI_CMD_LEN)) { //ncase2
		g_camif_host.outfmt.opix_fmt = PIX_FMT_YUYV;
		g_camif_host.outfmt.st_order = PIX_SEMP;
		g_camif_host.outfmt.opix_w = 1920;
		g_camif_host.outfmt.opix_h = 6;
		return run_camif_rtl_case();

	}
	else if(0 == strncmp(C4, argv[1], CSI_CMD_LEN)) { //ncase1
		g_camif_host.outfmt.opix_fmt = PIX_FMT_YUYV;
		g_camif_host.outfmt.st_order = PIX_PLANAR;
		g_camif_host.outfmt.opix_w = 1920;
		g_camif_host.outfmt.opix_h = 6;

		camif_init_config.itu601 = 0; //set bt656
		return run_camif_rtl_case();

	}
	else if(0 == strncmp(C5, argv[1], CSI_CMD_LEN)) { //ncase3
		g_camif_host.outfmt.opix_fmt = PIX_FMT_YUYV;
		g_camif_host.outfmt.st_order = PIX_INTERLEAVED;
		g_camif_host.outfmt.opix_w = 1920;
		g_camif_host.outfmt.opix_h = 6;
		camif_init_config.itu601 = 1;
		return run_camif_rtl_case();

	}
	else if(0 == strncmp(C6, argv[1], CSI_CMD_LEN)) { //ncase5 dmmu
		g_camif_host.outfmt.opix_fmt = PIX_FMT_NV12;
		g_camif_host.outfmt.st_order = PIX_SEMP;
#ifdef CONFIG_COMPILE_RTL
		g_camif_host.outfmt.opix_w = 1920;
		g_camif_host.outfmt.opix_h = 6;
#else
		camif_init_config.imgsource = 1;
		g_camif_host.outfmt.opix_w = VGA_WIDTH;
		g_camif_host.outfmt.opix_h = VGA_HEIGHT;
#endif
		g_camif_host.dmmu_en = 1;//dmmu enable
		return run_camif_rtl_case();

	}
	else if(0 == strncmp(C8, argv[1], CSI_CMD_LEN)) { //ncase5 c8 c9 for test tvif
		printf("c8\n");
		g_camif_host.outfmt.opix_fmt = PIX_FMT_NV12;
		g_camif_host.outfmt.st_order = PIX_SEMP;
		camif_init_config.itu601 = 0;
		camif_init_config.yuv_sequence = CAMIF_INPUT_YUV_ORDER_CrYCbY;
		g_camif_host.outfmt.opix_w = 720;
		g_camif_host.outfmt.opix_h = 576;
		return run_camif_rtl_case();
	}
	else if(0 == strncmp(C9, argv[1], CSI_CMD_LEN)) { //ncase0
		printf("c9\n");
		g_camif_host.outfmt.opix_fmt = PIX_FMT_NV12;
		g_camif_host.outfmt.st_order = PIX_SEMP;
		camif_init_config.yuv_sequence = CAMIF_INPUT_YUV_ORDER_CrYCbY;
		g_camif_host.outfmt.opix_w = 720;
		g_camif_host.outfmt.opix_h = 576;
		return run_camif_rtl_case();
	}
	else if(0 == strncmp(C10, argv[1], CSI_CMD_LEN)) { //ncase5
		printf("camif c10- scrrenshot test begin:\n");
		g_camif_host.outfmt.opix_fmt = PIX_FMT_NV12;
		g_camif_host.outfmt.st_order = PIX_SEMP;
#ifdef CONFIG_COMPILE_RTL
		g_camif_host.outfmt.opix_w = 1920;
		g_camif_host.outfmt.opix_h = 4;
		g_camif_host.outfmt.winverofst = 3;
		g_camif_host.outfmt.winhorofst = 3;
#else
		//g_camif_host.outfmt.opix_w = VGA_WIDTH / 2;
		//g_camif_host.outfmt.opix_h = VGA_HEIGHT / 2;
		g_camif_host.outfmt.opix_w = 480;
		g_camif_host.outfmt.opix_h = 480;
		g_camif_host.outfmt.winverofst = 0;
		g_camif_host.outfmt.winhorofst = 80;
		camif_init_config.imgsource = 1;
#endif
		return 	run_camif_rtl_case();
	}
	else if(0 == strncmp(C11, argv[1], CSI_CMD_LEN)) { //ncase5
		printf("C11- interlaced input :\n");

		if(!strncmp(argv[2], "itu601", 6))
			camif_init_config.itu601 = 1;
		else if(!strncmp(argv[2], "itu656", 6))
			camif_init_config.itu601 = 0;

		camif_init_config.interlaced = 1;
		//camif_init_config.yuv_sequence = CAMIF_INPUT_YUV_ORDER_CbYCrY;
		camif_init_config.yuv_sequence = CAMIF_INPUT_YUV_ORDER_CrYCbY;

		g_camif_host.outfmt.opix_fmt = PIX_FMT_YUYV;
		g_camif_host.outfmt.st_order = PIX_PLANAR;

#ifdef CONFIG_COMPILE_RTL
		g_camif_host.outfmt.opix_w =  720;
		g_camif_host.outfmt.opix_h = 6;
#else
		/* FPGA TEST NOT Ok */
		g_camif_host.outfmt.opix_w = 720;
		g_camif_host.outfmt.opix_h = 240;
#endif
		return 	run_camif_rtl_case();
	}
	else if(0 == strncmp(C14, argv[1], CSI_CMD_LEN)) {
		printf("C14 test sony imx322 on FPGA\n");
		g_camif_host.outfmt.opix_fmt = PIX_FMT_YUYV;
		g_camif_host.outfmt.st_order = PIX_INTERLEAVED;
		g_camif_host.outfmt.opix_w = 2176 / 2;
		g_camif_host.outfmt.opix_h = 1116;//1080;
		camif_init_config.invvsync = 1; //reverse the v sync singal
		camif_init_config.imgsource = 1;
		return  run_camif_rtl_case();
	}
	else if(0 == strncmp(C12, argv[1], CSI_CMD_LEN)) { //ncase5
		int csi_width, csi_height, csi_fmt = 0;
		atoi(&csi_width, argv[2], 10);      
		atoi(&csi_height, argv[3], 10);     
		if(!strncmp(argv[4], "yuyv", 4))
			csi_fmt = 0x1e;
		else if(!strncmp(argv[4], "raw8", 4))
			csi_fmt = 0x2a;
		else if(!strncmp(argv[4], "raw10", 5))
			csi_fmt = 0x2b;
		else if(!strncmp(argv[4], "raw12", 5))
			csi_fmt = 0x2c;
		else if(!strncmp(argv[4], "raw14", 5))
			csi_fmt = 0x2d;

		printf("C12: MIPI-->CAMIF Test: CSI-WIDTH:%d CSI-HEIGHT-%d CSI-FMT-%s\n" ,csi_width, csi_height, argv[4]);

		writel(readl(CAMIF_SYSM_ADDR+0x20) | 0x4, CAMIF_SYSM_ADDR +0x20);
		csi_config_test(csi_width,csi_height,csi_fmt);
		g_camif_host.outfmt.opix_fmt = PIX_FMT_YUYV;
		g_camif_host.outfmt.st_order = PIX_INTERLEAVED;

		camif_init_config.invvsync = 1;
		camif_init_config.dvpsel = 1;
#ifdef CONFIG_COMPILE_RTL
		if(csi_fmt == 0x1e) {
			g_camif_host.outfmt.opix_w = 1920;
			g_camif_host.outfmt.opix_h = 6;
		} else {
			g_camif_host.outfmt.opix_w = 1920 / 2;
			g_camif_host.outfmt.opix_h = 6;
		}
#else
		g_camif_host.outfmt.opix_w = 1920 / 2;
		g_camif_host.outfmt.opix_h = 1080;
#endif
		return  run_camif_rtl_case();
	}
	else if(0 == strncmp(C13, argv[1], CSI_CMD_LEN)) { //ncase5
		printf("C13- camif dvp wraper mode: %s\n", argv[2]);   
		if(!strncmp(argv[2], "sysdelay", 8)) {                 
			camifdvp_init_config.hmode = 0x2;                  
			camifdvp_init_config.vmode = 0x1;                  
#ifdef CONFIG_COMPILE_RTL                                     
			camifdvp_init_config.hnum = 1920 * 2;              
			camifdvp_init_config.vnum = 6;                     
			camifdvp_init_config.hdlycnt = 20;                 
			camifdvp_init_config.vdlycnt = 2;                  
			g_camif_host.outfmt.opix_w =  1920;                    
			g_camif_host.outfmt.opix_h = 6;                        
#else                                                         
			camifdvp_init_config.hnum = 480 * 2;               
			camifdvp_init_config.vnum = 240;                   
			camifdvp_init_config.hdlycnt = 160;                
			camifdvp_init_config.vdlycnt = 120;                
			camif_init_config.imgsource = 1;
			g_camif_host.outfmt.opix_w = 480;            
			g_camif_host.outfmt.opix_h = 240;
#endif
			g_camif_host.outfmt.opix_fmt = PIX_FMT_NV12;           
			g_camif_host.outfmt.st_order = PIX_SEMP;               

		} else if(!strncmp(argv[2], "synccode", 8)) {          
			camifdvp_init_config.hmode = 0x3;                  
			camifdvp_init_config.vmode = 0x1;                  
			camifdvp_init_config.syncode0  = 0x9b;
			camifdvp_init_config.syncode1  = 0xbc;
			camifdvp_init_config.syncode2  = 0x30;
			camifdvp_init_config.syncode3  = 0x42;
			camifdvp_init_config.syncmask  = 0xff;
#ifdef CONFIG_COMPILE_RTL                                     
			camifdvp_init_config.hnum = 1920 * 2 ;                  
			camifdvp_init_config.vnum = 6;                     
			g_camif_host.outfmt.opix_w =  1920;                    
			g_camif_host.outfmt.opix_h = 6;
#else
			camif_init_config.interlaced = 1;
			camif_init_config.yuv_sequence = CAMIF_INPUT_YUV_ORDER_CbYCrY;
			camifdvp_init_config.hnum = 716 * 2;               
			camifdvp_init_config.vnum = 240;                   
			g_camif_host.outfmt.opix_w = 716;            
			g_camif_host.outfmt.opix_h = 240;
#endif
		//	g_camif_host.outfmt.opix_fmt = PIX_FMT_NV12;           
		//	g_camif_host.outfmt.st_order = PIX_SEMP;               
			g_camif_host.outfmt.opix_fmt = PIX_FMT_YUYV;
			g_camif_host.outfmt.st_order = PIX_PLANAR;
		}
		return  run_camif_rtl_case();
	}
	else if(0 == strncmp(RW, argv[1],  CSI_CMD_LEN)) {

		imapx_camif_cfg_clock(CAMIF_CLK_DFT, CAMERA_OCLK_DFT);
		imapx_camif_register_test();

	}
	else if(0 == strncmp(DS, argv[1], CSI_CMD_LEN)) { //from camera to ids

		replace_irq(CAMIF_INTR_NUM, camif_irq_handle_with_encoder, "camif");
		g_camif_host.outfmt.opix_fmt = PIX_FMT_NV12;
		g_camif_host.outfmt.st_order = PIX_SEMP;
		g_camif_host.outfmt.opix_w = VGA_WIDTH;
		g_camif_host.outfmt.opix_h = VGA_HEIGHT;
		//initialize queue head
		INIT_LIST_HEAD(&(g_camif_host.cam_queue));
		INIT_LIST_HEAD(&(g_camif_host.pre_queue));
		//initialize ids
		if (ids_init() < 0) {
			CSI_ERR("initialize ids failed.\n");
			return -1;
		}
		camif_capture_config();
		/* configure ids reg */
		ids_set_format(IDS_YUV_420SP);
		ids_set_vm_width(VGA_WIDTH);
		ids_set_resolution(VGA_WIDTH, VGA_HEIGHT);
		//start camif
		camif_stream_start();
		//start ids
		//ids_enable(1);
		//loop encode
		struct camif_dma_buf * buf = NULL;
		int cflag = 0;
		while(1) {
			while(!(list_empty(&(g_camif_host.cam_queue)))) {
				buf = imapx_camif_pop_queue();

				if(NULL != buf) {
					//set ids dma address ,send the picture data in memory buffer to ids
					if(0 != cflag) {
						lcd_wait_frame();
					}
					ids_set_dma_addr((uint8_t *)(buf->dma_buf_saddr), (uint8_t *)(buf->dma_buf_saddr + g_camif_host.addr.y_len));
					if(0 == cflag) {
						//start ids
						ids_enable(1);
						cflag = 1;
					}
					//push buffer
					imapx_camif_push_pre_queue(buf);
					buf = NULL;
				//	printf("encode one frame done and then push frame buffer successfully!\n");
				}

			}

		}

	}
	else if(0 == strncmp(ES, argv[1], CSI_CMD_LEN)) {
#if !defined(CONFIG_COMPILE_ENC_H2)
		if(3 > argc) {
			CSI_ERR("the command argument is less than need, please check!\n");
			return -1;
		}

		int ret = 0, nwrites = 0;
	    H264EncodePara h264Para;
		offset = 0;
		nframe = 0;
		atoi((int *)&enc_frames, argv[2], 10);
		printf("get run frame size from command %lu\n", enc_frames);
		replace_irq(CAMIF_INTR_NUM, camif_irq_handle_with_encoder, "camif");
		venc_h1_module_enable();

		//initialize queue head
		INIT_LIST_HEAD(&(g_camif_host.cam_queue));
		INIT_LIST_HEAD(&(g_camif_host.pre_queue));
		//set camif output format
		g_camif_host.outfmt.opix_fmt = PIX_FMT_NV12;
		g_camif_host.outfmt.st_order = PIX_SEMP;
		g_camif_host.outfmt.opix_w = VGA_WIDTH;
		g_camif_host.outfmt.opix_h = VGA_HEIGHT;
		//set h264 parament
		h264Para.width = VGA_WIDTH;
		h264Para.height = VGA_HEIGHT;
		h264Para.pFrameNum = 1; //need to be 1
		h264Para.pollMode = 1;

		//reset mmc1
		vs_assign_by_name("mmc1", 1);
		outBuf = malloc(CAMIF_DMA_BUF_LEN);
		if(NULL == outBuf) {
			CSI_ERR("malloc encoder output buffer failed\n");
			goto ERR_TAB;
		}

		camif_capture_config();

		//initialize h264
		handler = h264_encode_initial(&h264Para);
		if (NULL == handler) {
			CSI_ERR("initialize h264n encoder failed\n");
			goto ERR_TAB;
		}
		//get venc stream header
		ret = h264_encode_get_start_stream(handler, (uint32_t)outBuf, CAMIF_DMA_BUF_LEN);
		if(0 > ret) {
			CSI_ERR("get h264 venc header failed\n");
			goto ERR_TAB;
		}
		else {
			printf("get venc stream header size =%d\n", ret);
		}

		//write stream header to sd card
//#if defined(USE_SDCARD_FOR_TEST)
		nwrites = vs_write(outBuf, 0, ret, 0);
		if(0 > nwrites) {
			CSI_ERR("write stream header to sd failed\n");
			goto ERR_TAB;
		}
//#else
		nwrites = write(intt_fd, outBuf, ret);
		if(nwrites != ret) {
			CSI_ERR("write steam header to file failed\n");
			goto ERR_TAB;
		}
//#endif
		offset += ret;
		//start camif to capture
		camif_stream_start();

		//loop encode
		struct camif_dma_buf * buf = NULL;
		while(1) {
			while(!(list_empty(&(g_camif_host.cam_queue)))) {
				buf = imapx_camif_pop_queue();
				if(NULL != buf) {
					//encode
					ret = h264_encode_frame(handler,(uint32_t)buf->dma_buf_saddr,
					  	(uint32_t)buf->dma_buf_saddr+g_camif_host.addr.y_len,
						(uint32_t)outBuf,
						CAMIF_DMA_BUF_LEN);
					//wait frame to encode done
					if(0 > ret ) {
						//ret = h264_encode_frame_is_done(handler);
						//udelay(1000);
						//printf("encode is processing!\n");
						printf("current frame encode is failed ret:%d\n ", ret);
					}
					printf("encode frame size = %d\n", ret);
					//write encode stream to sd card
		//#if defined(USE_SDCARD_FOR_TEST)
					nwrites = vs_write(outBuf, offset, ret, 0);
					if(0 > nwrites) {
						CSI_ERR("write encode stream to sd failed\n");
						imapx_camif_disable_capture();
						if(0 < intt_fd) {
							close(intt_fd);
						}
						return 0;
					}
					offset += ret;
		//#else
					nwrites = write(intt_fd, outBuf, ret);
					if(nwrites != ret) {
						CSI_ERR("write encode stream to file failed\n");
						imapx_camif_disable_capture();
						if(0 < intt_fd) {
							close(intt_fd);
						}
						return 0;
					}
		//#endif
					//done then get the buffer to pre queue
					imapx_camif_push_pre_queue(buf);
					buf = NULL;
					printf("encode one frame done and then push frame buffer successfully!\n");

					if(1 == ret_flag) {
						//stop capture
						imapx_camif_disable_capture();
						printf("stop camif ok\n");
					}
				}

			}

			if(1 == ret_flag && list_empty(&(g_camif_host.cam_queue))) {
				printf("all frames has encode done ok!\n");
				ret_flag = 0;
				//imapx_camif_disable_capture();
				break;
			}
		}

		ret = h264_encode_get_end_stream(handler, (uint32_t )outBuf, CAMIF_DMA_BUF_LEN);
		if(ret <= 0 ){
			CSI_ERR("h264 get end stream  error\n");
			goto ERR_TAB;
		}
		else {
			printf("get h264 end stream size =%d\n", ret);
		}
//#if defined(USE_SDCARD_FOR_TEST)
		nwrites = vs_write(outBuf, offset, ret, 0);
		if(0 > nwrites) {
			CSI_ERR("write encode stream end to sd failed\n");
			goto ERR_TAB;
		}
//#else
		nwrites = write(intt_fd, outBuf, ret);
		if(nwrites != ret) {
			CSI_ERR("write encode stream end to file failed\n");
			goto ERR_TAB;
		}
//#endif
ERR_TAB:
		camif_close();
		if(NULL != handler)	{
			h264_encode_deinital(handler);
		}

		if(NULL != outBuf) {
			free(outBuf);
		}
		if(NULL != g_camif_host.dma_buf_saddr) {
			free(g_camif_host.dma_buf_saddr);
		}
		venc_h1_module_disable();
		venc_flag = 0;
		if(0 < intt_fd) {
			close(intt_fd);
		}

		while(!(list_empty(&(g_camif_host.pre_queue)))) {
				buf = imapx_camif_pop_pre_queue();
				free(buf);
		}
#endif

	}
	else if(0 == strncmp(EC, argv[1], CSI_CMD_LEN)) {
#if defined(CONFIG_COMPILE_ENC_H2)

		if(3 > argc) {
			CSI_ERR("the command argument is less than need, please check!\n");
			return -1;
		}
		H265EncodePara h265Para;
		//set h265 parament
		h265Para.width = VGA_WIDTH;
		h265Para.height = VGA_HEIGHT;
		h265Para.pFrameNum = 1;
		h265Para.pollMode = 1;
		venc_h2_module_enable();
		//initialize h265
		handler = h265_encode_initial(&h265Para);
		if (NULL == handler) {
			CSI_ERR("initialize h265 encoder failed\n");
			goto ERR_TAB1;
		}

		int ret = 0, nwrites = 0;
		offset = 0;
		nframe = 0;
		atoi((int *)&enc_frames, argv[2], 10);
		printf("get run frame size from command %lu\n", enc_frames);
		replace_irq(CAMIF_INTR_NUM, camif_irq_handle_with_encoder, "camif");

		//initialize queue head
		INIT_LIST_HEAD(&(g_camif_host.cam_queue));
		INIT_LIST_HEAD(&(g_camif_host.pre_queue));
		//set camif output format
		g_camif_host.outfmt.opix_fmt = PIX_FMT_NV12;
		g_camif_host.outfmt.st_order = PIX_SEMP;
		g_camif_host.outfmt.opix_w = VGA_WIDTH;
		g_camif_host.outfmt.opix_h = VGA_HEIGHT;

		//reset mmc1
		vs_assign_by_name("mmc1", 1);
		outBuf = malloc(CAMIF_DMA_BUF_LEN);
		if(NULL == outBuf) {
			CSI_ERR("malloc encoder output buffer failed\n");
			goto ERR_TAB1;
		}

		camif_capture_config();

				//get venc stream header
		ret = h265_encode_get_start_stream(handler, (uint32_t)outBuf, CAMIF_DMA_BUF_LEN);
		if(0 > ret) {
			CSI_ERR("get h265 venc header failed\n");
			goto ERR_TAB1;
		}
		else {
			printf("get venc stream header size =%d\n", ret);
		}

		//write stream header to sd card
//#if defined(USE_SDCARD_FOR_TEST)
		nwrites = vs_write(outBuf, 0, ret, 0);
		if(0 > nwrites) {
			CSI_ERR("write stream header to sd failed\n");
			goto ERR_TAB1;
		}
//#else
		nwrites = write(intt_fd, outBuf, ret);
		if(nwrites != ret) {
			CSI_ERR("write steam header to file failed\n");
			goto ERR_TAB1;
		}
//#endif
		offset += ret;
		//start camif to capture
		camif_stream_start();

		//loop encode
		struct camif_dma_buf * buf = NULL;
		while(1) {
			while(!(list_empty(&(g_camif_host.cam_queue)))) {
				buf = imapx_camif_pop_queue();
				if(NULL != buf) {
					//encode
					ret = h265_encode_frame(handler,(uint32_t)buf->dma_buf_saddr,
					  	(uint32_t)buf->dma_buf_saddr+g_camif_host.addr.y_len,
						(uint32_t)outBuf,
						CAMIF_DMA_BUF_LEN);
					//wait frame to encode done
					if(0 > ret ) {
						//ret = h265_encode_frame_is_done(handler);
						//udelay(1000);
						//printf("encode is processing!\n");
						printf("current frame encode is failed ret:%d, y_buffer=%x, outBuf = %x\n ",
								ret, (uint32_t)buf->dma_buf_saddr,(uint32_t)outBuf);
					}
					printf("encode frame size = %d\n", ret);
					//write encode stream to sd card
		//#if defined(USE_SDCARD_FOR_TEST)
					nwrites = vs_write(outBuf, offset, ret, 0);
					if(0 > nwrites) {
						CSI_ERR("write encode stream to sd failed\n");
						imapx_camif_disable_capture();
						if(0 < intt_fd) {
							close(intt_fd);
						}
						return 0;
					}
					offset += ret;
		//#else
					nwrites = write(intt_fd, outBuf, ret);
					if(nwrites != ret) {
						CSI_ERR("write encode stream to file failed\n");
						imapx_camif_disable_capture();
						if(0 < intt_fd) {
							close(intt_fd);
						}
						return 0;
					}
		//#endif
					//done then get the buffer to pre queue
					imapx_camif_push_pre_queue(buf);
					buf = NULL;
					printf("encode one frame done and then push frame buffer successfully!\n");

					if(1 == ret_flag) {
						//stop capture
						imapx_camif_disable_capture();
						printf("stop camif ok\n");
					}
				}

			}

			if(1 == ret_flag && list_empty(&(g_camif_host.cam_queue))) {
				printf("all frames has encode done ok!\n");
				ret_flag = 0;
				//imapx_camif_disable_capture();
				break;
			}
		}

		ret = h265_encode_get_end_stream(handler, (uint32_t )outBuf, CAMIF_DMA_BUF_LEN);
		if(ret <= 0 ){
			CSI_ERR("h265 get end stream  error\n");
			goto ERR_TAB1;
		}
		else {
			printf("get h265 end stream size =%d\n", ret);
		}
//#if defined(USE_SDCARD_FOR_TEST)
		nwrites = vs_write(outBuf, offset, ret, 0);
		if(0 > nwrites) {
			CSI_ERR("write encode stream end to sd failed\n");
			goto ERR_TAB1;
		}
//#else
		nwrites = write(intt_fd, outBuf, ret);
		if(nwrites != ret) {
			CSI_ERR("write encode stream end to file failed\n");
			goto ERR_TAB1;
		}
//#endif
ERR_TAB1:
		camif_close();
		if(NULL != handler)	{
			h265_encode_deinital(handler);
		}

		if(NULL != outBuf) {
			free(outBuf);
		}
		if(NULL != g_camif_host.dma_buf_saddr) {
			free(g_camif_host.dma_buf_saddr);
		}
		venc_h2_module_disable();
		venc_flag = 0;
		if(0 < intt_fd) {
			close(intt_fd);
		}

		while(!(list_empty(&(g_camif_host.pre_queue)))) {
				buf = imapx_camif_pop_pre_queue();
				free(buf);
		}
#endif
	}


	return 0;
}


int camif_test_rtl(int id)
{
	CSI_INFO("invoking id: %d\n", id);
	return 0;
}

void camif_testbench_init(void)
{
	register_testbench("camif", camif_test_main, camif_test_rtl);
}

testbench_init(camif_testbench_init);
