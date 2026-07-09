#!/usr/bin/env bash
set -euo pipefail

SERVICE_USER="${MOCHAD_SERVICE_USER:-mochad}"
SERVICE_GROUP="${MOCHAD_SERVICE_GROUP:-mochad}"
USB_GROUP="${MOCHAD_USB_GROUP:-x10}"
INSTALL_SERVICE=true
DRY_RUN=false

SCRIPT_DIR="$(CDPATH= cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(CDPATH= cd -- "$SCRIPT_DIR/.." && pwd)"
SYSTEMD_SERVICE_SRC="$REPO_ROOT/systemd/mochad.service"
UDEV_SYSTEMD_SRC="$REPO_ROOT/udev/91-usb-x10-controllers.rules-systemd"
UDEV_GENERIC_SRC="$REPO_ROOT/udev/91-usb-x10-controllers.rules"
UDEV_DEST="/etc/udev/rules.d/91-usb-x10-controllers.rules"
SYSTEMD_SERVICE_DEST="/etc/systemd/system/mochad.service"

usage() {
    cat <<EOF
Usage: $0 [--dry-run] [--no-systemd] [--help]

Creates the native Linux permission model used by mochad-redux:

  user:      ${SERVICE_USER}
  group:     ${SERVICE_GROUP}
  USB group: ${USB_GROUP}
  udev:      root:${USB_GROUP} mode 0660 for CM15A/CM19A USB nodes

Environment overrides:
  MOCHAD_SERVICE_USER   default: mochad
  MOCHAD_SERVICE_GROUP  default: mochad
  MOCHAD_USB_GROUP      default: x10

Options:
  --dry-run     Show the operations without changing the host.
  --no-systemd  Install only the generic udev permission rule.
  --help        Show this help.
EOF
}

while [ "$#" -gt 0 ]; do
    case "$1" in
        --dry-run)
            DRY_RUN=true
            ;;
        --no-systemd)
            INSTALL_SERVICE=false
            ;;
        --help|-h)
            usage
            exit 0
            ;;
        *)
            echo "Unknown option: $1" >&2
            usage >&2
            exit 64
            ;;
    esac
    shift
done

log() {
    printf '%s\n' "$*"
}

run() {
    log "+ $*"
    if [ "$DRY_RUN" = false ]; then
        "$@"
    fi
}

require_root() {
    if [ "$DRY_RUN" = true ]; then
        return
    fi
    if [ "${EUID:-$(id -u)}" -ne 0 ]; then
        echo "This script must be run as root. Try: sudo $0" >&2
        exit 77
    fi
}

require_command() {
    if ! command -v "$1" >/dev/null 2>&1; then
        if [ "$DRY_RUN" = true ]; then
            log "[DRY-RUN] command not found locally, but would be required on target host: $1"
            return
        fi
        echo "Required command not found: $1" >&2
        exit 69
    fi
}

have() {
    command -v "$1" >/dev/null 2>&1
}

require_one_of() {
    label="$1"
    shift

    for candidate in "$@"; do
        if have "$candidate"; then
            return
        fi
    done

    if [ "$DRY_RUN" = true ]; then
        log "[DRY-RUN] none of these commands were found locally, but one would be required on target host for ${label}: $*"
        return
    fi

    echo "Required command group not found for ${label}: $*" >&2
    exit 69
}

systemd_available() {
    [ "$INSTALL_SERVICE" = true ] \
        && have systemctl \
        && [ -d /etc/systemd/system ]
}

group_exists() {
    group="$1"
    if have getent; then
        getent group "$group" >/dev/null 2>&1
        return
    fi
    grep -q "^${group}:" /etc/group 2>/dev/null
}

user_exists() {
    id -u "$1" >/dev/null 2>&1
}

create_system_group() {
    group="$1"
    if have groupadd; then
        run groupadd --system "$group"
    elif have addgroup; then
        run addgroup -S "$group"
    elif [ "$DRY_RUN" = true ]; then
        run groupadd --system "$group"
    else
        echo "No group creation command found. Expected groupadd or addgroup." >&2
        exit 69
    fi
}

add_user_to_usb_group() {
    if have usermod; then
        run usermod -g "$SERVICE_GROUP" -a -G "$USB_GROUP" "$SERVICE_USER"
    elif have addgroup; then
        run addgroup "$SERVICE_USER" "$USB_GROUP"
        log "[WARN] addgroup cannot change an existing user's primary group; verify ${SERVICE_USER} uses primary group ${SERVICE_GROUP}."
    elif [ "$DRY_RUN" = true ]; then
        run usermod -g "$SERVICE_GROUP" -a -G "$USB_GROUP" "$SERVICE_USER"
    else
        echo "No user group modification command found. Expected usermod or addgroup." >&2
        exit 69
    fi
}

create_system_user() {
    if have useradd; then
        run useradd \
            --system \
            --no-create-home \
            --home-dir /nonexistent \
            --shell /usr/sbin/nologin \
            --gid "$SERVICE_GROUP" \
            --groups "$USB_GROUP" \
            "$SERVICE_USER"
    elif have adduser; then
        run adduser \
            -S \
            -D \
            -H \
            -h /nonexistent \
            -s /sbin/nologin \
            -G "$SERVICE_GROUP" \
            "$SERVICE_USER"
        if have addgroup; then
            run addgroup "$SERVICE_USER" "$USB_GROUP"
        fi
    elif [ "$DRY_RUN" = true ]; then
        run useradd \
            --system \
            --no-create-home \
            --home-dir /nonexistent \
            --shell /usr/sbin/nologin \
            --gid "$SERVICE_GROUP" \
            --groups "$USB_GROUP" \
            "$SERVICE_USER"
    else
        echo "No user creation command found. Expected useradd or adduser." >&2
        exit 69
    fi
}

ensure_group() {
    group="$1"
    if group_exists "$group"; then
        log "[OK] group exists: $group"
        return
    fi
    create_system_group "$group"
}

ensure_user() {
    if user_exists "$SERVICE_USER"; then
        log "[OK] user exists: $SERVICE_USER"
        add_user_to_usb_group
        return
    fi

    create_system_user
}

install_udev_rule() {
    run install -d -m 0755 /etc/udev/rules.d

    if systemd_available; then
        run install -m 0644 "$UDEV_SYSTEMD_SRC" "$UDEV_DEST"
    else
        run install -m 0644 "$UDEV_GENERIC_SRC" "$UDEV_DEST"
    fi

    if have udevadm; then
        run udevadm control --reload-rules
        run udevadm trigger --subsystem-match=usb --attr-match=idVendor=0bc7
    else
        log "[WARN] udevadm not found; reload udev rules manually or replug the controller."
    fi
}

install_systemd_service() {
    if ! systemd_available; then
        log "[INFO] systemd service install skipped."
        return
    fi

    run install -m 0644 "$SYSTEMD_SERVICE_SRC" "$SYSTEMD_SERVICE_DEST"
    run systemctl daemon-reload
    log "[INFO] mochad.service installed. It will be activated by udev when a supported controller is connected."
}

report_usb_nodes() {
    if [ ! -d /dev/bus/usb ]; then
        log "[INFO] /dev/bus/usb is not present on this host."
        return
    fi

    log "[INFO] Current X10 USB nodes, if any:"
    found=false
    for sysdev in /sys/bus/usb/devices/*; do
        [ -r "$sysdev/idVendor" ] || continue
        [ -r "$sysdev/idProduct" ] || continue
        vendor="$(cat "$sysdev/idVendor")"
        product="$(cat "$sysdev/idProduct")"
        case "${vendor}:${product}" in
            0bc7:0001|0bc7:0002)
                found=true
                busnum="$(cat "$sysdev/busnum")"
                devnum="$(cat "$sysdev/devnum")"
                node="$(printf '/dev/bus/usb/%03d/%03d' "$busnum" "$devnum")"
                if [ -e "$node" ]; then
                    ls -l "$node"
                else
                    log "[WARN] expected USB node missing: $node"
                fi
                ;;
        esac
    done

    if [ "$found" = false ]; then
        log "[INFO] No CM15A/CM19A controller detected right now. Replug the controller after setup."
    fi
}

main() {
    require_root
    require_command id
    require_command install
    require_one_of "group creation" groupadd addgroup
    require_one_of "user creation" useradd adduser
    require_one_of "user group membership" usermod addgroup

    log "[STARTUP] configuring native mochad permissions"
    log "[INFO] service user=${SERVICE_USER} service group=${SERVICE_GROUP} usb group=${USB_GROUP}"

    ensure_group "$SERVICE_GROUP"
    ensure_group "$USB_GROUP"
    ensure_user
    install_udev_rule
    install_systemd_service
    report_usb_nodes

    log "[DONE] native mochad permissions are configured"
}

main "$@"
