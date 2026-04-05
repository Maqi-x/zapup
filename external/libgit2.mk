LIBGIT2_DIR       := external/libgit2
LIBGIT2_BUILD_DIR := $(BUILD_DIR)/libgit2
LIBGIT2_LIB       := $(LIBGIT2_BUILD_DIR)/libgit2.a

COMMON_CFLAGS += -I$(LIBGIT2_DIR)/include
EXT_LIBS      += $(LIBGIT2_LIB)

ifeq ($(PLATFORM),posix)
	LDFLAGS += -lssh2 -lssl -lcrypto -lz -lpthread -lrt
endif

$(LIBGIT2_LIB):
	$(call MKDIR,$(LIBGIT2_BUILD_DIR))
	cd $(LIBGIT2_BUILD_DIR) && cmake ../../../$(LIBGIT2_DIR) \
		-DCMAKE_BUILD_TYPE=$(if $(filter debug,$(BUILD)),Debug,Release) \
		-DBUILD_SHARED_LIBS=OFF \
		-DBUILD_TESTS=OFF \
		-DUSE_SSH=ON \
		-DUSE_HTTPS=ON \
		-DUSE_AUTH_NEGOTIATE=OFF \
		-DUSE_AUTH_NTLM=OFF \
		-DBUILD_EXAMPLES=OFF \
		-DUSE_BUNDLED_ZLIB=ON \
		-DREGEX_BACKEND=builtin \
		-DUSE_HTTP_PARSER=builtin \
		-DCMAKE_POSITION_INDEPENDENT_CODE=ON \
		-DCMAKE_C_FLAGS="$(CSTD)"
	$(MAKE) -C $(LIBGIT2_BUILD_DIR)
