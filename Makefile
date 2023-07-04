# Makefile for MiniballSort
.PHONY: clean all

PWD			:= $(shell pwd)
BIN_DIR     := ./bin
SRC_DIR     := ./src
LIB_DIR     := ./lib
INC_DIR     := ./include
UTIL_DIR    := ./utils
AME_FILE	:= \"$(PWD)/data/mass_1.mas20\"
SRIM_DIR	:= \"$(PWD)/srim/\"
CUR_DIR		:= \"$(PWD)/\"

ROOTVER     := $(shell root-config --version | head -c1)
ifeq ($(ROOTVER),5)
	ROOTDICT  := rootcint
	DICTEXT   := .h
else
	ROOTDICT  := rootcling
	DICTEXT   := _rdict.pcm
endif

PLATFORM:=$(shell uname)
ifeq ($(PLATFORM),Darwin)
SHAREDSWITCH = -Qunused-arguments -shared -undefined dynamic_lookup -dynamiclib -Wl,-install_name,'@executable_path/../lib/'# NO ENDING SPACE
OSDEF = -DMACOSX
else
SHAREDSWITCH = -shared -Wl,-soname,# NO ENDING SPACE
OSDEF = -DLINUX
LIBEXTRA = -lrt
endif


ROOTCFLAGS   := $(shell root-config --cflags)
ROOTLDFLAGS  := $(shell root-config --ldflags)
ROOTLIBS     := $(shell root-config --glibs) -lRHTTP -lThread
LIBS         := $(ROOTLIBS) $(LIBEXTRA)

# Compiler.
CC          = $(shell root-config --cxx)
# Flags for compiler.
CFLAGS		= -c -Wall -Wextra $(ROOTCFLAGS) -g -fPIC
CFLAGS		+= -DUNIX -DPOSIX $(OSDEF)
INCLUDES	+= -I$(INC_DIR) -I.

# Pass in the data file locations
CFLAGS		+= -DAME_FILE=$(AME_FILE)
CFLAGS		+= -DSRIM_DIR=$(SRIM_DIR)
CFLAGS		+= -DCUR_DIR=$(CUR_DIR)

# Linker.
LD          = $(shell root-config --ld)
# Flags for linker.
LDFLAGS 	+= $(ROOTLDFLAGS)

# The object files.
OBJECTS =  		$(SRC_DIR)/Calibration.o \
				$(SRC_DIR)/CommandLineInterface.o \
				$(SRC_DIR)/Converter.o \
				$(SRC_DIR)/DataPackets.o \
				$(SRC_DIR)/DataSpy.o \
				$(SRC_DIR)/Settings.o \
				$(SRC_DIR)/EventBuilder.o \
				$(SRC_DIR)/MbsConverter.o \
				$(SRC_DIR)/MbsFormat.o \
				$(SRC_DIR)/MidasConverter.o \
				$(SRC_DIR)/MiniballEvts.o \
				$(SRC_DIR)/MiniballGeometry.o \
				$(SRC_DIR)/Reaction.o \
				$(SRC_DIR)/Histogrammer.o \
				$(SRC_DIR)/MiniballGUI.o

# The header files.
DEPENDENCIES =  $(INC_DIR)/Calibration.hh \
				$(INC_DIR)/CommandLineInterface.hh \
				$(INC_DIR)/Converter.hh \
				$(INC_DIR)/DataPackets.hh \
				$(INC_DIR)/DataSpy.hh \
				$(INC_DIR)/Settings.hh \
				$(INC_DIR)/EventBuilder.hh \
				$(INC_DIR)/MbsConverter.hh \
				$(INC_DIR)/MbsFormat.hh \
				$(INC_DIR)/MidasConverter.hh \
				$(INC_DIR)/MiniballEvts.hh \
				$(INC_DIR)/MiniballGeometry.hh \
				$(INC_DIR)/Reaction.hh \
				$(INC_DIR)/Histogrammer.hh \
				$(INC_DIR)/MiniballGUI.hh

 
.PHONY : all
all: $(BIN_DIR)/mb_sort $(LIB_DIR)/libmb_sort.so $(UTIL_DIR)/mb_angle_fit
 
$(LIB_DIR)/libmb_sort.so: mb_sort.o $(OBJECTS) mb_sortDict.o
	mkdir -p $(LIB_DIR)
	$(LD) mb_sort.o $(OBJECTS) mb_sortDict.o $(SHAREDSWITCH)$@ $(LIBS) -o $@

$(BIN_DIR)/mb_sort: mb_sort.o $(OBJECTS) mb_sortDict.o
	mkdir -p $(BIN_DIR)
	$(LD) -o $@ $^ $(LDFLAGS) $(LIBS)

mb_sort.o: mb_sort.cc
	$(CC) $(CFLAGS) $(INCLUDES) $^

$(UTIL_DIR)/%: $(UTIL_DIR)/%.o $(OBJECTS) $(UTIL_DIR)/%Dict.o
	mkdir -p $(BIN_DIR)
	$(LD) -o $@ $^ $(LDFLAGS) $(LIBS)

$(UTIL_DIR)/%.o: $(UTIL_DIR)/%.cc
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(SRC_DIR)/%.o: $(SRC_DIR)/%.cc $(INC_DIR)/%.hh
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

mb_sortDict.o: mb_sortDict.cc mb_sortDict$(DICTEXT) $(INC_DIR)/RootLinkDef.h
	mkdir -p $(BIN_DIR)
	mkdir -p $(LIB_DIR)
	$(CC) -fPIC $(CFLAGS) $(INCLUDES) -c $<
	cp $(basename $@)$(DICTEXT) $(LIB_DIR)/
	cp $(basename $@)$(DICTEXT) $(BIN_DIR)/

mb_sortDict.cc: $(DEPENDENCIES) $(INC_DIR)/RootLinkDef.h
	$(ROOTDICT) -f $@ -c $(INCLUDES) $(DEPENDENCIES) $(INC_DIR)/RootLinkDef.h

$(UTIL_DIR)/%Dict.o: $(UTIL_DIR)/%Dict.cc $(UTIL_DIR)/%Dict$(DICTEXT) $(INC_DIR)/RootLinkDef.h
	mkdir -p $(BIN_DIR)
	mkdir -p $(LIB_DIR)
	$(CC) -fPIC $(CFLAGS) $(INCLUDES) -c $<
	cp $(basename $@)$(DICTEXT) $(LIB_DIR)/
	cp $(basename $@)$(DICTEXT) $(BIN_DIR)/

$(UTIL_DIR)/%Dict.cc: $(DEPENDENCIES) $(INC_DIR)/RootLinkDef.h
	$(ROOTDICT) -f $@ -c $(INCLUDES) $(DEPENDENCIES) $(INC_DIR)/RootLinkDef.h

clean:
	rm -vf $(BIN_DIR)/mb_sort $(SRC_DIR)/*.o $(UTIL_DIR)/*.o $(SRC_DIR)/*~ \
	$(UTIL_DIR)/*~ $(INC_DIR)/*.gch *.o $(BIN_DIR)/*.pcm $(UTIL_DIR)/*.pcm \
	*.pcm $(BIN_DIR)/*Dict* $(UTIL_DIR)/*Dict* *Dict* $(LIB_DIR)/*
