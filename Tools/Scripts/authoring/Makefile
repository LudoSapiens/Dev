# ##
# A generic Makefile for various authoring automation.
#
# Help topics start with a double hash followed by a space ("## ").
#
# It can be used to convert SVGs to PNGs, etc.
# ##


#############
# Variables #
#######################################################################

CONVERT    := convert
GREP       := grep
GZIP       := gzip
INKSCAPE   := inkscape
NOTHING    := echo -n
RM         := rm

ifndef PLATFORM
   PLATFORM := $(shell uname)
endif

ifeq ($(PLATFORM), Darwin)
   INKSCAPE   := $(shell mdfind -onlyin /Applications -interpret "Inkscape kind:Application" | xargs -J % find % -name inkscape)
endif

ifndef INKSCAPE_OPTIONS
   # Ref: http://inkscape.org/doc/inkscape-man.html
   #INKSCAPE_OPTIONS := --export-area-drawing
   INKSCAPE_OPTIONS := --export-area-page
   #INKSCAPE_OPTIONS := --export-area-snap
endif



###################
# UTILITY TARGETS #
#######################################################################

default: help

# List all targets we don't want to automatically delete here.
#.SECONDARY: $(DVI_FILES) $(PDF_FILES)

.SUFFIXES:
.SUFFIXES: .dvi .eps .jpg .pdf .png .ps .tex .v .v_all .view .view_all

## A generic Makefile used to compile ANY LaTeX file in the current directory.
## 
## Usage:

help: always
	@$(GREP) "^## " Makefile | sed s/"^## "//g

#	@clear  # Add this line as the first line above for a nice clear screen before the help.

# Fake phony rule to allow extension rules to be phony.
# That allows, for example, things like 'make file.view' even if
# a file of that name exists.
.PHONY: always
always:
	@$(NOTHING)

# A note on some special variables:
#  $@: The name of the target.
#  $*: The root name of the target.
#  $<: The first prerequisite.
#  $^: All of the prerequisites. 

## 

#########
# UTILS #
#######################################################################

##   > make debug            Prints various debug information.
debug:
	@echo "Variables"
	@echo "---------"
	@echo "        PLATFORM: $(PLATFORM)"
	@echo "INKSCAPE_OPTIONS: $(INKSCAPE_OPTIONS)"
	@echo ""
	@echo "Tools"
	@echo "-----"
	@echo "      GREP: $(GREP)"
	@echo "        RM: $(RM)"
	@echo "  INKSCAPE: $(INKSCAPE)"

## 

###############
# CONVERSIONS #
#######################################################################
# Note:
#   Inkscape >=0.48 support an '--export-latex' option which allows equations in diagrams.
#   More details here:
#     http://wiki.inkscape.org/wiki/images/SVG_in_LaTeX.pdf
##

##   > make file.eps         Converts file.svg into file.eps using Inkscape.
%.eps: %.svg
	$(INKSCAPE) --without-gui $(INKSCAPE_OPTIONS) --file=$^ --export-eps=$@

##   > make file.png         Converts file.svg into file.png using Inkscape.
%.png: %.svg
	$(INKSCAPE) --without-gui $(INKSCAPE_OPTIONS) --file=$^ --export-png=$@

##   > make file.pdf         Converts file.svg into file.pdf using Inkscape.
%.pdf: %.svg
	$(INKSCAPE) --without-gui $(INKSCAPE_OPTIONS) --file=$^ --export-pdf=$@

##                           Note: All of the above honor INKSCAPE_OPTIONS environment.

## 

##   > make file.gz          Compresses a file using GZip.
%.gz:
	$(GZIP) --best -c $* > $@

## 
