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
template_DATA = packaging/linux/systemd/mochad.service.in \
		packaging/linux/udev/91-usb-x10-controllers.rules.in \
		packaging/linux/config/mochad.conf.example
legacydir = $(pkgdatadir)/legacy/hotplug2
legacy_DATA = packaging/openwrt/hotplug2/20-usb-x10 \
		packaging/openwrt/init.d/mochad
docdir = $(datadir)/doc/mochad-redux
dist_doc_DATA = README.md docs/installation/native-install-rollback.md \
		docs/development/legacy-support-files.md \
		docs/installation/supported-platforms.md
licensedir = $(datadir)/licenses/mochad-redux
license_DATA = COPYING NOTICE LICENSE.md
man_MANS = packaging/linux/man/mochad.1 \
		packaging/linux/man/mochad-redux-setup.8

print-sbindir:
	@echo '$(sbindir)'

mochad_SOURCES = src/core/mochad.c src/core/global.c src/core/global.h \
		src/config/config.c src/config/config.h src/config/version.h \
		src/net/diagnostics.c src/net/diagnostics.h \
		src/net/socket_io.c src/net/socket_io.h \
		src/net/transport_evidence.c src/net/transport_evidence.h \
		src/usb/usb_endpoint_selection.c src/usb/usb_endpoint_selection.h \
		src/usb/x10_write.c src/usb/x10_write.h \
		src/x10/decode.c src/x10/decode.h \
		src/x10/encode.c src/x10/encode.h \
		src/x10/mochad_event.c src/x10/mochad_event.h \
		src/x10/x10state.c src/x10/x10state.h
EXTRA_DIST = packaging/linux/config/mochad.conf.example \
		packaging/linux/systemd/mochad.service.in \
		packaging/linux/udev/91-usb-x10-controllers.rules.in \
		docs/research/legacy-packaging/udev/91-usb-x10-controllers.rules \
		docs/research/legacy-packaging/udev/91-usb-x10-controllers.rules-systemd \
		docs/research/legacy-packaging/udev/README.md \
		packaging/openwrt/hotplug2/20-usb-x10 \
		packaging/openwrt/init.d/mochad \
		docs/research/legacy-packaging/systemd/mochad.service \
		cgi/x10.pl cgi/netcat.pl cgi/getsensors.pl cgi/cgi-lib.pl \
		contrib/apps/mochamon.pl contrib/apps/simplemon.pl \
		contrib/apps/bash.sh contrib/apps/rfsectopl3.pl \
		docs/research/legacy-apps/x10-tk.py \
		docs/research/legacy-apps/mochad.scr \
		.clang-format .clang-format-ignore \
		scripts/build/compile-without-libusb.sh \
		scripts/format/c.sh \
		scripts/setup/install-native.sh scripts/setup/mochad-redux-setup.in \
		scripts/release/prepare-next-dev.sh \
		scripts/release/prepare-release.sh \
		scripts/release/update-version-files.sh \
		scripts/support/collect_support_bundle.py \
		scripts/hardware/cm19a-validation.sh \
		scripts/hardware/lab-preflight.sh \
		scripts/hardware/record-result.sh \
		scripts/hardware/run-cm19a-validation.sh \
		scripts/validate/clang-format.sh \
		scripts/validate/clean-build-test.sh \
		scripts/validate/docker-build.sh \
		scripts/validate/docker-smoke-test.sh \
		scripts/validate/full-libusb-build.sh \
		scripts/validate/libusb-stub-syntax-check.sh \
		scripts/validate/native-smoke-test.sh \
		scripts/validate/native-setup-tool.sh \
		scripts/validate/staged-install-contract.sh \
		scripts/validate/release-evidence.sh \
		scripts/validate/release-evidence-runner.sh \
		scripts/validate/repository-hygiene.sh \
		scripts/validate/shellcheck.sh \
		scripts/validate/source-archive-validation.sh \
		scripts/validate/strict-libusb-free-compile.sh \
		scripts/validate/tcp-diagnostics-smoke-test.sh \
		scripts/validate/unit-tests.sh \
		scripts/validate/version-consistency.sh \
		tests/unit/test_config.c tests/unit/test_diagnostics.c \
		tests/unit/test_transport_evidence.c \
		tests/golden/test_mochad_event.c \
		tests/unit/test_encode_rf.c \
		tests/unit/test_x10_write.c \
		tests/unit/test_socket_io.c tests/integration/test_tcp_diagnostics.c \
		tests/unit/test_usb_endpoint_selection.c \
		tests/test_support_bundle.py \
		docs/architecture/compatibility.md \
		docs/development/generated-artifacts.md \
		docs/development/branch-closeout-inventory.md \
		docs/development/branch-closeout-plan.md \
		docs/development/branch-closeout-results.md \
		docs/development/hardware-lab-setup.md \
		docs/development/hardware-validation.md \
		docs/development/test-strategy.md \
		docs/protocol/json-api.md \
		docs/research/milestones/milestone-1.md \
		docs/research/audits/static-analysis.md \
		docs/research/README.md \
		docs/research/legacy-apps/README.md \
		docs/research/legacy-packaging/README.md \
		docs/research/plans/static-analysis-cleanup.md \
		docs/research/source-lineage.md \
		docs/development/repository-layout-source-move-manifest.md \
		docs/development/legacy-support-files.md \
		docs/development/duplicate-and-obsolete-files.md \
		docs/development/repository-file-inventory.md \
		docs/development/repository-layout-proposal.md \
		docs/README.md \
		docs/installation/supported-platforms.md \
		docs/installation/native-install-rollback.md \
		docs/operations/support-bundles.md \
		packaging/linux/man/mochad.1 \
		packaging/linux/man/mochad-redux-setup.8 \
		validation/README.md validation/compiler.md validation/hardware.md \
		validation/regression.md validation/release-evidence-template.md \
		validation/releases/README.md validation/releases/v0.4.0.md \
		docs/architecture/architecture.md CHANGELOG.md CONTRIBUTING.md \
		docs/architecture/design.md docs/development/maintaining.md \
		docs/architecture/project-status.md README.md \
		docs/operations/transport-evidence.md \
		docs/release/beta-status.md docs/release/checklist.md \
		docs/release/roadmap.md \
		AGENTS.md SECURITY.md COPYING LICENSE.md NOTICE VERSION \
		tests/support/libusb-1.0/libusb.h
