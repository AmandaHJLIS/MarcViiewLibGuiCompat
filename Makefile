#---------------------------------------------------------------------------------
# MarcViiewLibGuiCompat - libgui Wii compatibility test
#---------------------------------------------------------------------------------
.SUFFIXES:

# Capture the repository root before the devkit rules add their own makefiles.
PROJECT_DIR := $(abspath $(dir $(firstword $(MAKEFILE_LIST))))

ifeq ($(strip $(DEVKITPPC)),)
$(error "Please set DEVKITPPC in your environment. export DEVKITPPC=<path to>devkitPPC")
endif

include $(DEVKITPPC)/wii_rules

# libgui is kept outside this test repository so the test tracks the upstream
# framework without copying its full source tree into MarcViiewLibGuiTest.
LIBGUI_DIR ?= $(PROJECT_DIR)/../libgui

ifeq ($(wildcard $(LIBGUI_DIR)/source/libgui/Gui.h),)
$(error "libgui not found at $(LIBGUI_DIR). Clone dborth/libgui beside this repository or set LIBGUI_DIR=/path/to/libgui")
endif

TARGET := $(notdir $(CURDIR))
BUILD := build

SOURCES := \
	$(PROJECT_DIR)/source \
	$(PROJECT_DIR)/source/compat \
	$(LIBGUI_DIR)/source \
	$(LIBGUI_DIR)/source/drivers \
	$(LIBGUI_DIR)/source/drivers/ogc \
	$(LIBGUI_DIR)/source/drivers/ogc/wii \
	$(LIBGUI_DIR)/source/libgui

INCLUDES := $(LIBGUI_DIR)/source
LIBDIRS := $(PORTLIBS)

export FREETYPE_CFLAGS := `$(DEVKITPRO)/portlibs/ppc/bin/powerpc-eabi-pkg-config --cflags freetype2`
export FREETYPE_LIBS := `$(DEVKITPRO)/portlibs/ppc/bin/powerpc-eabi-pkg-config --libs freetype2`

CFLAGS = -g -O2 -Wall -Wextra $(MACHDEP) $(INCLUDE) $(FREETYPE_CFLAGS)
LIBGUI_COMPAT_STAGE ?= 1
RAW_VIDEO_TEST ?= 0
VIDEO_DRIVER_TEST ?= 1

# Startup diagnostic stages:
# 1 = thread + video only
# 2 = + audio
# 3 = + Wii/GameCube input
# 4 = + Wii filesystem/USB/DVD
# 5 = full upstream-style startup
CXXFLAGS = $(CFLAGS) -std=c++11 -DVIDEO_WaitForFlush=VIDEO_WaitVSync -DLIBGUI_COMPAT_STAGE=$(LIBGUI_COMPAT_STAGE)
LDFLAGS = -g $(MACHDEP) -Wl,-Map,$(notdir $@).map

LIBS := -ldi -liso9660 -lpng -lz -lfat -lwiiuse -lbte -lasnd -logc -lvorbisidec -logg $(FREETYPE_LIBS)

ifneq ($(BUILD),$(notdir $(CURDIR)))

export OUTPUT := $(CURDIR)/$(TARGET)
export VPATH := $(SOURCES) $(LIBGUI_DIR)/data/images $(LIBGUI_DIR)/data/fonts $(LIBGUI_DIR)/data/sounds $(LIBGUI_DIR)/data/lang
export DEPSDIR := $(CURDIR)/$(BUILD)

CFILES := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
ifeq ($(RAW_VIDEO_TEST),1)
CPPFILES := main.cpp
else ifeq ($(VIDEO_DRIVER_TEST),1)
CPPFILES := main.cpp OgcVideoDriverCompat.cpp
else
CPPFILES := $(filter-out demo.cpp menu.cpp filebrowser.cpp OgcFileSystemDriver.cpp OgcSmbDriver.cpp OgcThreadDriver.cpp WiiFileSystemDriver.cpp WiiUsbMulti.cpp WiiPlatform.cpp,$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp))))
endif
sFILES := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
SFILES := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.S)))

# libgui's filelist.h expects these generated resource headers to exist.
# Generate them for compilation, but do not link the bundled resource objects
# into the minimal stage-1 hardware diagnostic.
ASSET_FILES := $(notdir $(wildcard $(LIBGUI_DIR)/data/images/*.png)) \
	$(notdir $(wildcard $(LIBGUI_DIR)/data/fonts/*.ttf)) \
	$(notdir $(wildcard $(LIBGUI_DIR)/data/sounds/*.ogg)) \
	$(notdir $(wildcard $(LIBGUI_DIR)/data/sounds/*.pcm)) \
	$(notdir $(wildcard $(LIBGUI_DIR)/data/lang/*.lang))

BINFILES :=

ifeq ($(strip $(CPPFILES)),)
export LD := $(CC)
else
export LD := $(CXX)
endif

export OFILES_BIN :=
export OFILES_SOURCES := $(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(sFILES:.s=.o) $(SFILES:.S=.o)
export OFILES := $(OFILES_BIN) $(OFILES_SOURCES)
export HFILES := $(addsuffix .h,$(subst .,_,$(ASSET_FILES)))

export INCLUDE := $(foreach dir,$(INCLUDES),-I$(dir)) \
	$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
	-I$(CURDIR)/$(BUILD) \
	-I$(LIBOGC_INC)

export LIBPATHS := $(foreach dir,$(LIBDIRS),-L$(dir)/lib) -L$(LIBOGC_LIB)

export OUTPUT := $(CURDIR)/$(TARGET)

.PHONY: $(BUILD) clean run

$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

clean:
	@echo clean ...
	@rm -fr $(BUILD) $(OUTPUT).elf $(OUTPUT).dol

run:
	wiiload $(TARGET).dol

else

DEPENDS := $(OFILES:.o=.d)

$(OUTPUT).dol: $(OUTPUT).elf
$(OUTPUT).elf: $(OFILES)

$(OFILES_SOURCES): $(HFILES)

%.ttf.o %_ttf.h : %.ttf
	@echo $(notdir $<)
	$(bin2o)

%.lang.o %_lang.h : %.lang
	@echo $(notdir $<)
	$(bin2o)

%.png.o %_png.h : %.png
	@echo $(notdir $<)
	$(bin2o)

%.ogg.o %_ogg.h : %.ogg
	@echo $(notdir $<)
	$(bin2o)

%.pcm.o %_pcm.h : %.pcm
	@echo $(notdir $<)
	$(bin2o)

-include $(DEPENDS)

endif
