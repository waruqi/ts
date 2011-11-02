# sub-makefile
include $(PRO_DIR)/prefix.mak

# module name
NAMES 				= ts

# module type
ts_TYPE 			= LIB

# basic files
ts_C_FILES 		+= \
					ts \
					epg \
					psi/section \
					psi/table \
					psi/descriptor \
					psi/table/pat \
					psi/table/pmt \
					psi/table/sdt \
					psi/table/eit \
					psi/table/nit \
					psi/descriptor/0x40 \
					psi/descriptor/0x41 \
					psi/descriptor/0x48 \
					psi/descriptor/0x4d

# sub-projects
ifeq ($(TS_CONFIG_DEMO),y)
SUB_PROS 			= demo/$(TS_CONFIG_PLAT)
endif

include $(PRO_DIR)/suffix.mak

