# nwm - New Window Manager

VERSION = 1.0

# Paths
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man

X11INC = /usr/include/X11
X11LIB = /usr/lib/X11

# Xinerama, comment if you don't want it
XINERAMALIBS  = -lXinerama
XINERAMAFLAGS = -DXINERAMA

# Freetype
FREETYPELIBS = -lfontconfig -lXft
FREETYPEINC = /usr/include/freetype2

# Includes and libs
INCS = -I${X11INC} -I${FREETYPEINC}
LIBS = -L${X11LIB} -lX11 ${XINERAMALIBS} ${FREETYPELIBS} -lXext

# Flags
CPPFLAGS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_POSIX_C_SOURCE=200809L -DVERSION=\"${VERSION}\" ${XINERAMAFLAGS}
CXXFLAGS = -std=c++17 -Wall -Wextra -Wpedantic -O2 ${INCS} ${CPPFLAGS}
LDFLAGS = ${LIBS}

# Compiler and linker
CXX = g++

# Source files
SRC = nwm.cpp window.cpp layout.cpp
OBJ = ${SRC:.cpp=.o}

# Target
all: options nwm

options:
	@echo nwm build options:
	@echo "CXXFLAGS = ${CXXFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CXX      = ${CXX}"

.cpp.o:
	${CXX} -c ${CXXFLAGS} $<

${OBJ}: config.h nwm.h window.h layout.h

nwm: ${OBJ}
	${CXX} -o $@ ${OBJ} ${LDFLAGS}

clean:
	rm -f nwm ${OBJ}

install: all
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp -f nwm ${DESTDIR}${PREFIX}/bin
	chmod 755 ${DESTDIR}${PREFIX}/bin/nwm

uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/nwm

.PHONY: all options clean install uninstall
