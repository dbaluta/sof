if BUILD_BAYTRAIL
SUBDIRS = intel
else
SUBDIRS = imx
endif

noinst_LTLIBRARIES = libdrivers.la

libdrivers_la_CFLAGS = \
	$(AM_CFLAGS) \
	$(ARCH_CFLAGS) \
	$(PLATFORM_INCDIR) \
	$(SOF_INCDIR) \
	$(ARCH_INCDIR)

# This lib is just composition of convenience libraries
if BUILD_BAYTRAIL
libdrivers_la_SOURCES = dw-dma.c
endif

if BUILD_BAYTRAIL
libdrivers_la_LIBADD = intel/libdrivers.la
else
libdrivers_la_LIBADD = imx/libdrivers.la
endif
