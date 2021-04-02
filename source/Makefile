APP1 = ChannelListSaver

# output object directory
CUR_DIR = $(shell pwd)
SRC_DIR = ${CUR_DIR}

# include default settings
BASE = $(shell cd /tapapi/TMS; pwd)
include ${BASE}/include/tool.mk

TAP_FLAGS += -fPIC -fno-strict-aliasing

ifeq (${DEBUG},y)
TAP_FLAGS += -g -DDEBUG
endif

#compile options
CFLAGS += -DLINUX -D_REENTRANT  -MD  -W -Wall  -O2 -static -pipe -funsigned-char -ffunction-sections -fdata-sections
CPPFLAGS += -DLINUX  -MD  -W -Wall  -O2 -static -pipe -funsigned-char -ffunction-sections -fdata-sections

INCLUDE_DIRS = ${BASE}/include /opt/crosstool/include
TAP_INCLUDE_DIRS = $(addprefix -I, $(INCLUDE_DIRS))
TAP_FLAGS += $(TAP_INCLUDE_DIRS)
TAP_LIBS = ${BASE}/tapinit.o -L ${BASE} -ltap -L /opt/crosstool/lib -lFireBird -ldl -lpthread -lc

all: $(APP1).tap clean

$(APP1).tap: $(APP1).o ImExportBin.o ImExportText.o FlashSatTablesSetInfo.o
	@echo "[Linking... $@]"
	@$(LD) -shared --no-undefined --allow-shlib-undefined -O1 --gc-sections -o $@ $^ $(TAP_LIBS)
	@$(JB) $@

%.o: %.c
	@echo "[Compiling... $@]"
	@$(CC) $(CFLAGS) $(TAP_FLAGS) -c $< -o $@

clean:
	@echo "[Cleaning up...]"
	@-${RM} *.d *.o
