
# 传递该变量到下一层Makefile中
TOP_DIR = $(shell pwd)
BUILD_DIR = $(TOP_DIR)/build/
BIN_PATH = $(BUILD_DIR)/bin/
BUILD_LIBS_PATH = $(BUILD_DIR)/libs/

# export TOP_DIR
export DEBUG_ING = TRUE


# 生成静态库和应用程序
all:
	@test -d $(BUILD_LIBS_PATH) || mkdir -p $(BUILD_LIBS_PATH)
	@test -d $(BIN_PATH) || mkdir -p $(BIN_PATH)
	$(MAKE) -C src
	$(MAKE) -C apps

# 只把各个模块生成静态库
module:
	@test -d $(BUILD_LIBS_PATH) || mkdir -p $(BUILD_LIBS_PATH)
	$(MAKE) -C src


# 只生成app1
app1:
	@test -d $(BIN_PATH) || mkdir -p $(BIN_PATH)
	$(MAKE) -C src
	$(MAKE) -C apps app1


obj:
	$(MAKE) -C src obj
	$(MAKE) -C apps obj


tags:
	$(MAKE) -C src tags
	$(MAKE) -C apps tags
	ctags -R

clean:
	$(MAKE) -C src clean
	$(MAKE) -C apps clean
	@rm -rf $(BUILD_DIR)

.PHONY: all tags obj clean module app1


