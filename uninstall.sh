#!/bin/bash
# Smart File Organizer — Uninstall Script
# Removes the binary and systemd user service.

set -e

BINARY_NAME="organizer"
SERVICE_NAME="smart-file-organizer"
INSTALL_DIR="$HOME/.local/bin"
SERVICE_DIR="$HOME/.config/systemd/user"

# ── Colors ──
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo ""
echo -e "  ${YELLOW}Uninstalling Smart File Organizer...${NC}"
echo ""

# ── Stop and disable service ──
if systemctl --user is-active "$SERVICE_NAME.service" &>/dev/null; then
    systemctl --user stop "$SERVICE_NAME.service"
    echo -e "  ${GREEN}✓${NC} Stopped service"
fi

if systemctl --user is-enabled "$SERVICE_NAME.service" &>/dev/null; then
    systemctl --user disable "$SERVICE_NAME.service"
    echo -e "  ${GREEN}✓${NC} Disabled service"
fi

# ── Remove service file ──
if [ -f "$SERVICE_DIR/$SERVICE_NAME.service" ]; then
    rm "$SERVICE_DIR/$SERVICE_NAME.service"
    systemctl --user daemon-reload
    echo -e "  ${GREEN}✓${NC} Removed service file"
fi

# ── Remove binary ──
if [ -f "$INSTALL_DIR/$BINARY_NAME" ]; then
    rm "$INSTALL_DIR/$BINARY_NAME"
    echo -e "  ${GREEN}✓${NC} Removed binary"
fi

echo ""
echo -e "  ${GREEN}Uninstall complete.${NC}"
echo ""
