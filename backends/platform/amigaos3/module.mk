MODULE := backends/platform/amigaos3

MODULE_OBJS = \
	amigaos3-main.o \
	amigaos3-aga.o \
	amigaos3-graphics-cgx.o

# We don't use rules.mk but rather manually update OBJS and MODULE_DIRS.
MODULE_OBJS := $(addprefix $(MODULE)/, $(MODULE_OBJS))
OBJS := $(MODULE_OBJS) $(OBJS)
MODULE_DIRS += $(sort $(dir $(MODULE_OBJS)))
