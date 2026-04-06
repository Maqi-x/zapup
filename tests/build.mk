TESTS_DIR := tests
UNITY_DIR := external/unity/src
TESTS_OUT_DIR := $(OUT_DIR)/tests

TESTS_SRCS := $(wildcard $(TESTS_DIR)/*.c)
TESTS_BINS := $(patsubst $(TESTS_DIR)/%.c,$(TESTS_OUT_DIR)/%$(EXE_EXT),$(TESTS_SRCS))

UNITY_OBJ := $(OBJ_ROOT_DIR)/unity.o

TESTS_CFLAGS := $(CFLAGS) -I$(UNITY_DIR)

.PHONY: test-dirs test

test-dirs:
	$(call MKDIR,$(TESTS_OUT_DIR))

$(UNITY_OBJ): $(UNITY_DIR)/unity.c
	$(call MKDIR,$(dir $@))
	$(CC) $(TESTS_CFLAGS) -c $< -o $@

$(TESTS_OUT_DIR)/%$(EXE_EXT): $(TESTS_DIR)/%.c $(LIB_OBJ_STATIC) $(UNITY_OBJ) | test-dirs
	$(CC) $(TESTS_CFLAGS) $< $(LIB_OBJ_STATIC) $(UNITY_OBJ) $(EXT_LIBS) $(LDFLAGS) -o $@

test: $(TESTS_BINS)
	$(foreach t,$(TESTS_BINS),$(call FIXPATH,$(t)) &&) echo "All tests passed."
