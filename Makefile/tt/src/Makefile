# 如果在此项目代码框架中，当前目录有c代码,添加该Makefile
TOP_DIR = ..
MODULE_NAME = src

CFLAGS += -g -Wall -O2
CPPFLAGS += -I./ -I$(TOP_DIR)/include/
# LDFALGS += -L$(BUILD_LIBS_PATH) -lmodlue1 -lmodlue2 -lmodlue3

BUILD_DIR = $(TOP_DIR)/build
# 编译过程中生成的静态库存放目录
BUILD_LIBS_PATH = $(BUILD_DIR)/libs/
# 生成的.o 文件目录，MODULE_NAME 是各个目录名，在各个模块Makefile中定义
OBJ_PATH = $(BUILD_DIR)/obj/$(MODULE_NAME)


SRC_FILES=$(wildcard *.c)
SRC_OBJ=$(SRC_FILES:.c=.o)


# 如果当前目录有c代码，则生成静态库
ifneq ($(SRC_FILES),)
SRC_MODULE = libsrc.a
endif


# 其他模块代码，各自生成静态库
MODULES=modlue1 modlue2 modlue3

all: $(MODULES) $(SRC_MODULE)

# 首先首先生成当前目录各自代码模块中的静态库
$(MODULES):
	$(MAKE) -C $@

# 如果modlue3依赖modlue1和modlue2的情况下, 添加下面
# modlue3:modlue1 modlue2



# 如果在当前目录下有c代码依赖其他模块静态库，下面是解决方法之一
# 假如依赖modlue1 modlue2 modlue3
LIBA_S = $(BUILD_LIBS_PATH)/libmodlue1.a $(BUILD_LIBS_PATH)/libmodlue2.a $(BUILD_LIBS_PATH)/libmodlue3.a

# LIB_A_O += $(shell ar -t $(BUILD_LIBS_PATH)/libmodlue1.a)
# LIB_A_O += $(shell ar -t $(BUILD_LIBS_PATH)/libmodlue2.a)
# LIB_A_O += $(shell ar -t $(BUILD_LIBS_PATH)/libmodlue3.a)

LIB_A_O += $(shell for liba in $(LIBA_S); do ar -t $$liba; done)


# 先把依赖的静态库中的.o文件解析处理，然后连接成一个新的库
# 再把该目录产生的.o文件移动到对应的目录中(如果目录不存在则创建)，
# 再删除依赖库的.o文件
$(SRC_MODULE): $(SRC_OBJ)
	@for liba in $(LIBA_S); \
		do ar -x $$liba; \
	done
	$(AR) rcs $@ $^ $(LIB_A_O)
	@mv -f $@ $(BUILD_LIBS_PATH)
	@test -d $(OBJ_PATH) || mkdir -p $(OBJ_PATH)
	@cp -f $^ $(OBJ_PATH)
	@$(RM) $(LIB_A_O)



# $(SRC_BIN):$(SRC_OBJ)
# 	$(CC) -o $@ $^ $(LDFALGS)
# 	mv -f $@ $(BUILD_LIBS_PATH)


tags:
	@for module in $(MODULES); \
	do $(MAKE) -C $$module $@;\
	done
	ctags -R


# 在shell命令中，要使用$$引用变量
obj: $(SRC_FILES)
	@for module in $(MODULES); \
	do $(MAKE) -C $$module $@; \
	done
	$(CC) -c $^

clean:
	@for module in $(MODULES); \
	do $(MAKE) -C $$module $@;\
	done
	$(RM) $(SRC_OBJ) *.o


# 注意要添加$(MODULES)伪目标, 否则编译会出现Nothing to be done for `all'`
.PHONY: all tags obj clean $(MODULES)

