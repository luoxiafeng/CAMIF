include $(SRCTREE)/build/config.mk
ARCHIVE := drv_camif.a

SRCS := camif_reg.c \
		camif_core.c 

include $(BUILD_LIB)

