localedir = $(datadir)/locale
DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@
AUTOMAKE_OPTIONS = nostdinc

bin_PROGRAMS = tdu
tdu_LDFLAGS  = $(LTLIBINTL)
tdu_SOURCES  = defs.h            extern.h       \
               main.c                           \
               mem.h             mem.c          \
               walk.h            walk.c

noinst_HEADERS = gettext.h
dist_man_MANS = tdu.1
