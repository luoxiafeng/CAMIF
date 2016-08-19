include $(SRCTREE)/build/config.mk
ARCHIVE := testbench_csi.a

SRCS := \
	csi_main.c \
	camif_main.c

include $(BUILD_LIB)

