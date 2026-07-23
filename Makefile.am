bin_PROGRAMS = mochad
sbin_SCRIPTS = scripts/mochad-redux-setup

AM_CPPFLAGS = -I$(top_srcdir)/src/core \
		-I$(top_srcdir)/src/config \
		-I$(top_srcdir)/src/net \
		-I$(top_srcdir)/src/usb \
		-I$(top_srcdir)/src/x10

# These files are deliberately installed as inactive templates.  A package
# installation must never alter the running host; mochad-redux-setup is the
# explicit administrator action that renders them into /etc.
templatedir = $(pkgdatadir)/templates
template_DATA = systemd/mochad.service.in \
		udev/91-usb-x10-controllers.rules.in \
		config/mochad.conf.example
legacydir = $(pkgdatadir)/legacy/hotplug2
legacy_DATA = hotplug2/20-usb-x10 hotplug2/mochad
docdir = $(datadir)/doc/mochad-redux
dist_doc_DATA = README.md docs/native-install-rollback.md \
		docs/support-files.md docs/supported-platforms.md
licensedir = $(datadir)/licenses/mochad-redux
license_DATA = COPYING NOTICE LICENSE.md
man_MANS = docs/mochad.1 docs/mochad-redux-setup.8

print-sbindir:
	@echo '$(sbindir)'

mochad_SOURCES = src/core/mochad.c src/core/global.c src/core/global.h \
		src/config/config.c src/config/config.h src/config/version.h \
		src/net/diagnostics.c src/net/diagnostics.h \
		src/net/socket_io.c src/net/socket_io.h \
		src/usb/usb_endpoint_selection.c src/usb/usb_endpoint_selection.h \
		src/usb/x10_write.c src/usb/x10_write.h \
		src/x10/decode.c src/x10/decode.h \
		src/x10/encode.c src/x10/encode.h \
		src/x10/mochad_event.c src/x10/mochad_event.h \
		src/x10/x10state.c src/x10/x10state.h
EXTRA_DIST = config/mochad.conf.example \
		systemd/mochad.service.in \
		udev/91-usb-x10-controllers.rules.in \
		udev/91-usb-x10-controllers.rules udev/91-usb-x10-controllers.rules-systemd \
		udev/README.md \
		hotplug2/20-usb-x10 hotplug2/mochad systemd/mochad.service \
		cgi/x10.pl cgi/netcat.pl cgi/getsensors.pl cgi/cgi-lib.pl \
		apps/mochamon.pl apps/simplemon.pl apps/bash.sh \
		apps/rfsectopl3.pl apps/x10-tk.py apps/mochad.scr \
		scripts/install-native.sh scripts/mochad-redux-setup.in \
		scripts/release/prepare-next-dev.sh \
		scripts/release/prepare-release.sh \
		scripts/release/update-version-files.sh \
		scripts/validate/cm19a-hardware-validation.sh \
		scripts/validate/clean-build-test.sh \
		scripts/validate/docker-build.sh \
		scripts/validate/docker-smoke-test.sh \
		scripts/validate/full-libusb-build.sh \
		scripts/validate/libusb-stub-syntax-check.sh \
		scripts/validate/native-smoke-test.sh \
		scripts/validate/native-setup-tool.sh \
		scripts/validate/staged-install-contract.sh \
		scripts/validate/release-evidence.sh \
		scripts/validate/repository-hygiene.sh \
		scripts/validate/source-archive-validation.sh \
		scripts/validate/strict-libusb-free-compile.sh \
		scripts/validate/tcp-diagnostics-smoke-test.sh \
		scripts/validate/unit-tests.sh \
		scripts/validate/version-consistency.sh \
		tests/test_config.c tests/test_diagnostics.c \
		tests/test_mochad_event.c \
		tests/test_encode_rf.c \
		tests/test_x10_write.c \
		tests/test_socket_io.c tests/test_tcp_diagnostics.c \
		tests/test_usb_endpoint_selection.c \
		docs/compatibility.md docs/generated-artifacts.md docs/hardware-validation.md \
		docs/json-api.md docs/milestone-1.md docs/source-lineage.md \
		docs/repository-layout-source-move-manifest.md \
		docs/support-files.md docs/supported-platforms.md \
		docs/native-install-rollback.md docs/mochad.1 \
		docs/mochad-redux-setup.8 \
		validation/README.md validation/compiler.md validation/hardware.md \
		validation/regression.md validation/release-evidence-template.md \
		validation/releases/README.md validation/releases/v0.4.0.md \
		ARCHITECTURE.md CHANGELOG.md CONTRIBUTING.md DESIGN.md \
		MAINTAINING.md PROJECT.md README.md RELEASE_CHECKLIST.md \
		ROADMAP.md SECURITY.md COPYING LICENSE.md NOTICE VERSION \
		tools/stubs/libusb-1.0/libusb.h
