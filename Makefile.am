bin_PROGRAMS = mochad
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
		tests/test_usb_endpoint_selection.c \
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
	if test -n "$(DESTDIR)" ; then \
		echo "staged install: skipping host user/group setup" ; \
	elif test "$$(id -u)" = "0" ; then \
		group_exists() { if command -v getent >/dev/null 2>&1 ; then getent group "$$1" >/dev/null 2>&1 ; else grep -q "^$$1:" /etc/group ; fi ; } ; \
		user_exists() { id -u "$$1" >/dev/null 2>&1 ; } ; \
		create_group() { if command -v groupadd >/dev/null 2>&1 ; then groupadd --system "$$1" ; elif command -v addgroup >/dev/null 2>&1 ; then addgroup -S "$$1" ; else echo "warning: no groupadd/addgroup found; run scripts/setup-native-permissions.sh manually" ; fi ; } ; \
		if ! group_exists mochad ; then create_group mochad ; fi ; \
		if ! group_exists x10 ; then create_group x10 ; fi ; \
		if ! user_exists mochad ; then \
			if command -v useradd >/dev/null 2>&1 ; then \
				useradd --system --no-create-home --home-dir /nonexistent --shell /usr/sbin/nologin --gid mochad --groups x10 mochad ; \
			elif command -v adduser >/dev/null 2>&1 ; then \
				adduser -S -D -H -h /nonexistent -s /sbin/nologin -G mochad mochad ; \
				if command -v addgroup >/dev/null 2>&1 ; then addgroup mochad x10 >/dev/null 2>&1 || true ; fi ; \
			else \
				echo "warning: no useradd/adduser found; run scripts/setup-native-permissions.sh manually" ; \
			fi ; \
		else \
			if command -v usermod >/dev/null 2>&1 ; then \
				usermod -g mochad -a -G x10 mochad ; \
			elif command -v addgroup >/dev/null 2>&1 ; then \
				addgroup mochad x10 >/dev/null 2>&1 || true ; \
			fi ; \
		fi ; \
	fi ;
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
