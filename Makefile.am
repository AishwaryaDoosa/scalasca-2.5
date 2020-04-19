## -*- mode: makefile -*-
##*************************************************************************##
##  SCALASCA    http://www.scalasca.org/                                   ##
##*************************************************************************##
##  Copyright (c) 1998-2019                                                ##
##  Forschungszentrum Juelich GmbH, Juelich Supercomputing Centre          ##
##                                                                         ##
##  Copyright (c) 2009-2013                                                ##
##  German Research School for Simulation Sciences GmbH,                   ##
##  Laboratory for Parallel Programming                                    ##
##                                                                         ##
##  This software may be modified and distributed under the terms of       ##
##  a BSD-style license.  See the COPYING file in the package base         ##
##  directory for details.                                                 ##
##*************************************************************************##


## Subdirectories to be built recursively
SUBDIRS = $(subdirs) $(ax_config_subdirs)

## Options passed to aclocal to specify local macro directories. Although
## ACLOCAL_AMFLAGS should be replaced by AC_CONFIG_MACRO_DIRS, aclocal-1.13.4
## doesn't honor the paths provided to AC_CONFIG_MACRO_DIRS. See
## https://debbugs.gnu.org/cgi/bugreport.cgi?bug=21352
ACLOCAL_AMFLAGS = -I build-config/m4 -I vendor/common/build-config/m4


## Initialize some variables to be able to use '+=' in Makefile.am snippets
EXTRA_DIST    =
CLEANFILES    =
PHONY_TARGETS =
bin_SCRIPTS   =
BUILT_SOURCES =

## To install configuration summary file(s)
pkgdata_DATA = \
    @PACKAGE@.summary

if PLATFORM_MIC

# On MIC, install config.summary also in $pkgdatadir/mic
micpkgdatadir   = $(pkgdatadir)/mic
micpkgdata_DATA = \
    @PACKAGE@.summary

else !PLATFORM_MIC

TARGETS = mic

endif !PLATFORM_MIC

## To install extra documentation files
dist_doc_DATA = \
    README \
    OPEN_ISSUES

## To clean up configuration summary files
DISTCLEANFILES = \
    $(builddir)/config.summary \
    $(builddir)/@PACKAGE@.summary


##--- Hooks -----------------------------------------------------------------

dist-hook: \
    copy-revision-file-to-dist \
    copy-generator-parser-files-to-dist \
    doxygen-user-pack \
    doxygen-man


install-data-local: \
    install-pattern-docs \
    install-pearl-example \
    doxygen-user-unpack


install-data-hook: \
    doxygen-man-install
	for target in $(TARGETS)""; do \
	    if test -n "$$target" && test -f $(DESTDIR)$(pkgdatadir)/$$target/@PACKAGE@.summary; then \
	        echo >> $(DESTDIR)$(pkgdatadir)/@PACKAGE@.summary ; \
	        echo >> $(DESTDIR)$(pkgdatadir)/@PACKAGE@.summary ; \
	        echo "=====[ Target: $$target ]====================" >> $(DESTDIR)$(pkgdatadir)/@PACKAGE@.summary ; \
	        echo >> $(DESTDIR)$(pkgdatadir)/@PACKAGE@.summary ; \
	        cat $(DESTDIR)$(pkgdatadir)/$$target/@PACKAGE@.summary >> $(DESTDIR)$(pkgdatadir)/@PACKAGE@.summary ; \
	    fi \
	done


install-exec-hook: \
    install-mic-wrapper

uninstall-hook: \
    uninstall-mic-wrapper \
    doxygen-man-uninstall
	rm -rf $(DESTDIR)$(docdir)


clean-local: \
    doxygen-dev-clean \
    doxygen-user-clean


##--- Extra distribution files ----------------------------------------------

include build-config/platforms/platforms.am
include vendor/common/build-config/platforms.am

EXTRA_DIST += \
    $(srcdir)/src/config.h \
    $(srcdir)/share/scalasca.errors \
    $(srcdir)/build-config/tap-driver.sh \
    $(srcdir)/vendor/common/COPYING \
    $(srcdir)/vendor/common/build-config/generate-library-version.sh \
    $(srcdir)/vendor/common/build-config/generate-package-version.sh \
    $(srcdir)/vendor/common/build-config/test-runner-mpi.sh \
    $(srcdir)/vendor/common/build-config/test-runner-plain.sh

if !CROSS_BUILD

EXTRA_DIST += \
    $(srcdir)/build-frontend/configure.ac \
    $(srcdir)/build-frontend/aclocal.m4 \
    $(srcdir)/build-frontend/Makefile.am \
    $(srcdir)/build-frontend/Makefile.in \
    $(srcdir)/build-frontend/configure \
    $(srcdir)/src/config-frontend.h.in

endif !CROSS_BUILD


##--- REVISION file related targets -----------------------------------------

PHONY_TARGETS += \
    copy-revision-file-to-dist \
    update-revision-file

copy-revision-file-to-dist: update-revision-file
	$(MKDIR_P) "$(distdir)/build-config/"
	for i in REVISION REVISION_COMMON; do                                                   \
	    cp -p "$(srcdir)/build-config/$$i" "$(distdir)/build-config/";                      \
	    if grep -E [A-Z] "$(srcdir)/build-config/$$i" ||                                    \
                grep "-" "$(srcdir)/build-config/$$i"; then                                     \
                echo "WARNING: distribution does not match a single, unmodified revision." >&2; \
            fi                                                                                  \
        done


if SVN_CONTROLLED

update-revision-file:
	@svnversion -c $(srcdir) | cut -d ':' -f 2 \
	    > "$(srcdir)/build-config/REVISION"
	@svnversion -c $(srcdir)/vendor/common | cut -d ':' -f 2 \
	    > "$(srcdir)/build-config/REVISION_COMMON"

else !SVN_CONTROLLED

update-revision-file:

endif !SVN_CONTROLLED


##--- Targets related to error code headers ---------------------------------

DISTCLEANFILES += \
    src/ScalascaErrorCodes.h \
    src/scalasca_error_decls.gen.h

BUILT_SOURCES += \
    src/ScalascaErrorCodes.h \
    src/scalasca_error_decls.gen.h

src/ScalascaErrorCodes.h: \
    $(srcdir)/share/scalasca.errors
src/scalasca_error_decls.gen.h: \
    $(srcdir)/share/scalasca.errors


##--- Doxygen-related targets -----------------------------------------------

PHONY_TARGETS += \
    doxygen-user-unpack

if SVN_CONTROLLED

## Documentation tarballs will not be generated in SVN mode,
## so there is nothing to unpack here...
doxygen-user-unpack:

else !SVN_CONTROLLED

doxygen-user-unpack:
	$(MKDIR_P) $(DESTDIR)$(docdir)
	cd $(srcdir)/doc; \
	    files=`ls *-html.tar.gz 2>/dev/null`; \
	    for i in $$files; do \
	        $(INSTALL_DATA) $$i $(DESTDIR)$(docdir)/; \
	        cd $(DESTDIR)$(docdir); \
	        tar xzvf $$i; \
	        rm -f $$i; \
	        cd -; \
	    done
	$(INSTALL_DATA) $(srcdir)/doc/UserGuide.pdf $(DESTDIR)$(docdir)/manual
	$(INSTALL_DATA) $(srcdir)/doc/QuickReference.pdf $(DESTDIR)$(docdir)/manual

endif !SVN_CONTROLLED


##--- Include Makefile.am snippets ------------------------------------------

include doc/Makefile.top.inc.am
include doc/doxygen-common/Makefile.top.inc.am
include doc/doxygen-dev/pearl/Makefile.top.inc.am
include doc/doxygen-dev/scout/Makefile.top.inc.am
include doc/doxygen-dev/silas/Makefile.top.inc.am
include doc/doxygen-user/manual/Makefile.top.inc.am
include doc/doxygen-user/manpages/Makefile.top.inc.am
include doc/doxygen-user/pearl/Makefile.top.inc.am
include doc/patterns/Makefile.top.inc.am
include doc/quick-reference/Makefile.top.inc.am
include share/Makefile.top.inc.am
include src/pearl/example/Makefile.top.inc.am
include src/scout/generator/Makefile.top.inc.am
include src/utils/Makefile.top.inc.am


.PHONY: $(PHONY_TARGETS)
