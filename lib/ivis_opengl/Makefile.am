AM_CPPFLAGS = $(SDL_CFLAGS) $(PNG_CFLAGS) $(OPENGLC_CFLAGS) $(OPENGL_CFLAGS) $(WZ_CPPFLAGS) $(GLee_CFLAGS)
AM_CFLAGS = $(WZ_CFLAGS) -Wno-missing-declarations
AM_CXXFLAGS = $(WZ_CXXFLAGS)

noinst_LIBRARIES = libivis_opengl.a
noinst_HEADERS = \
	piematrix.h \
	screen.h \
        bitimage.h \
        imd.h \
        ivi.h \
        ivisdef.h \
        jpeg_encoder.h \
        pieblitfunc.h \
        pieclip.h \
        piedef.h \
        piefunc.h \
        piemode.h \
        piepalette.h \
        piestate.h \
        pietypes.h \
        png_util.h \
        rendmode.h \
        tex.h \
        textdraw.h

libivis_opengl_a_SOURCES = \
	ivi.cpp \
	pieblitfunc.cpp \
	piedraw.cpp \
	piefunc.cpp \
	piematrix.cpp \
	piemode.cpp \
	piepalette.cpp \
	piestate.cpp \
	rendmode.cpp \
	screen.cpp \
	tex.cpp \
	textdraw.cpp \
        bitimage.cpp \
        imd.cpp \
        imdload.cpp \
        jpeg_encoder.cpp \
        pieclip.cpp \
        png_util.cpp

if !SYSTEM_GLEE
libivis_opengl_a_SOURCES += GLee.c
noinst_HEADERS += GLee.h
endif

libivis_opengl_a_LIBADD = $(top_builddir)/lib/framework/libframework.a
