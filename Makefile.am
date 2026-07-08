bin_PROGRAMS = mochad
mochad_SOURCES = mochad.c config.c diagnostics.c encode.c global.c \
		mochad_event.c socket_io.c x10state.c x10_write.c \
		config.h decode.h diagnostics.h encode.h global.h \
		mochad_event.h socket_io.h version.h x10state.h x10_write.h
EXTRA_DIST = udev/91-usb-x10-controllers.rules udev/91-usb-x10-controllers.rules-systemd \
		udev/README.md \
		hotplug2/20-usb-x10 hotplug2/mochad systemd/mochad.service \
		cgi/x10.pl cgi/netcat.pl cgi/getsensors.pl cgi/cgi-lib.pl \
		apps/mochamon.pl apps/simplemon.pl apps/bash.sh \
		apps/rfsectopl3.pl apps/x10-tk.py apps/mochad.scr \
		scripts/validate/cm19a-hardware-validation.sh \
		scripts/validate/clean-build-test.sh \
		scripts/validate/docker-build.sh \
		scripts/validate/docker-smoke-test.sh \
		scripts/validate/full-libusb-build.sh \
		scripts/validate/libusb-stub-syntax-check.sh \
		scripts/validate/native-smoke-test.sh \
		scripts/validate/strict-libusb-free-compile.sh \
		scripts/validate/tcp-diagnostics-smoke-test.sh \
		scripts/validate/unit-tests.sh \
		tests/test_config.c tests/test_diagnostics.c \
		tests/test_mochad_event.c \
		tests/test_x10_write.c \
		tests/test_socket_io.c tests/test_tcp_diagnostics.c \
		docs/generated-artifacts.md docs/hardware-validation.md \
		docs/json-api.md docs/milestone-1.md docs/support-files.md \
		docs/supported-platforms.md \
		validation/README.md validation/compiler.md validation/hardware.md \
		validation/regression.md validation/release-evidence-template.md \
		validation/releases/README.md \
		ARCHITECTURE.md CHANGELOG.md CONTRIBUTING.md DESIGN.md \
		MAINTAINING.md PROJECT.md README.md RELEASE_CHECKLIST.md \
		ROADMAP.md SECURITY.md LICENSE.md \
		tools/stubs/libusb-1.0/libusb.h

install-exec-hook:
	if test -d /lib/systemd/system -a -x /bin/systemctl ; then \
				cp $(abs_top_srcdir)/systemd/mochad.service /etc/systemd/system ; \
				cp $(abs_top_srcdir)/udev/91-usb-x10-controllers.rules-systemd /etc/udev/rules.d/91-usb-x10-controllers.rules ; \
	else \
		if test -d /etc/udev/rules.d ; then \
			cp $(abs_top_srcdir)/udev/91-usb-x10-controllers.rules /etc/udev/rules.d ; \
		fi ; \
	fi ;
	if test -d /etc/hotplug.d/usb ; then \
		cp $(abs_top_srcdir)/hotplug2/20-usb-x10 /etc/hotplug.d/usb ; \
		cp $(abs_top_srcdir)/hotplug2/mochad /etc/init.d ; \
	fi ;
