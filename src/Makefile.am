localedir = $(datadir)/locale
DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@
AUTOMAKE_OPTIONS = nostdinc

bin_PROGRAMS = tdu
dut_LDFLAGS  = $(LTLIBINTL)

noinst_HEADERS = gettext.h
dist_man_MANS = tdu.1
