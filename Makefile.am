bin_PROGRAMS = mochad
systemdsystemunitdir = $(prefix)/lib/systemd/system
systemdsystemunit_DATA = systemd/mochad.service
udevrulesdir = $(prefix)/lib/udev/rules.d
udevrules_DATA = udev/91-usb-x10-controllers.rules \
		udev/91-usb-x10-controllers.rules-systemd
hotplug2dir = $(pkgdatadir)/hotplug2
hotplug2_DATA = hotplug2/20-usb-x10 hotplug2/mochad

mochad_SOURCES = mochad.c config.c decode.c diagnostics.c encode.c global.c \
		mochad_event.c socket_io.c usb_endpoint_selection.c \
		x10state.c x10_write.c \
		config.h decode.h diagnostics.h encode.h global.h \
		mochad_event.h socket_io.h usb_endpoint_selection.h \
		version.h x10state.h x10_write.h
EXTRA_DIST = udev/91-usb-x10-controllers.rules udev/91-usb-x10-controllers.rules-systemd \
		udev/README.md \
		hotplug2/20-usb-x10 hotplug2/mochad systemd/mochad.service \
		cgi/x10.pl cgi/netcat.pl cgi/getsensors.pl cgi/cgi-lib.pl \
		apps/mochamon.pl apps/simplemon.pl apps/bash.sh \
		apps/rfsectopl3.pl apps/x10-tk.py apps/mochad.scr \
		scripts/setup-native-permissions.sh \
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
		docs/support-files.md docs/supported-platforms.md \
		docs/native-install-rollback.md \
		validation/README.md validation/compiler.md validation/hardware.md \
		validation/regression.md validation/release-evidence-template.md \
		validation/releases/README.md validation/releases/v0.4.0.md \
		ARCHITECTURE.md CHANGELOG.md CONTRIBUTING.md DESIGN.md \
		MAINTAINING.md PROJECT.md README.md RELEASE_CHECKLIST.md \
		ROADMAP.md SECURITY.md COPYING LICENSE.md NOTICE VERSION \
		tools/stubs/libusb-1.0/libusb.h
