YYJSON_DIR := external/yyjson
YYJSON_SRC := $(YYJSON_DIR)/src/yyjson.c
YYJSON_LIB := $(BUILD_DIR)/libyyjson.a
YYJSON_OBJ := $(OBJ_ROOT_DIR)/$(YYJSON_SRC:.c=.o)

COMMON_CFLAGS += -I$(YYJSON_DIR)/src
EXT_LIBS += $(YYJSON_LIB)

DEPS += $(DEP_ROOT_DIR)/$(YYJSON_SRC:.c=.d)

$(YYJSON_LIB): $(YYJSON_OBJ)
	$(call MKDIR,$(dir $@))
	$(AR) rcs $@ $^
