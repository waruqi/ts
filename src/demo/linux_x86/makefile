# sub-makefile
include $(PRO_DIR)/prefix.mak

# module name
NAMES = epg psi

define MAKE_MODULES
$(1)_TYPE 				= BIN
$(1)_C_FILES 			= $(1)
$(1)_LIBS 				= ts tbox
$(1)_LIB_DIR 			= ../../
$(1)_INC_DIR 			= ../../
$(1)_CFLAGS-$(DEBUG) 	= -DTB_TRACE_ENABLE -DTB_ASSERT_ENABLE
endef
$(foreach name, $(NAMES), $(eval $(call MAKE_MODULES,$(name))))

include $(PRO_DIR)/suffix.mak

