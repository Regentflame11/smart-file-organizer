#!/bin/bash
# Smart File Organizer — Install Script
# Sets up the binary and systemd user service for autostart.

set -e

BINARY_NAME="organizer"
SERVICE_NAME="smart-file-organizer"
INSTALL_DIR="$HOME/.local/bin"
SERVICE_DIR="$HOME/.config/systemd/user"

# ── Colors ──
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo ""
echo -e "  ${GREEN}╔══════════════════════════════════════╗${NC}"
echo -e "  ${GREEN}║   Smart File Organizer — Installer   ║${NC}"
echo -e "  ${GREEN}╚══════════════════════════════════════╝${NC}"
echo ""

# ── Check if binary exists ──
if [ ! -f "$BINARY_NAME" ]; then
    echo -e "  ${RED}Error:${NC} Binary '$BINARY_NAME' not found."
    echo "  Run 'make' first to build the project."
    exit 1
fi

# ── Determine watch directory ──
WATCH_DIR="${1:-$HOME/Downloads}"

if [ ! -d "$WATCH_DIR" ]; then
    echo -e "  ${RED}Error:${NC} Directory '$WATCH_DIR' does not exist."
    exit 1
fi

echo -e "  Watch directory: ${YELLOW}$WATCH_DIR${NC}"

# ── Install binary ──
mkdir -p "$INSTALL_DIR"
cp "$BINARY_NAME" "$INSTALL_DIR/$BINARY_NAME"
chmod +x "$INSTALL_DIR/$BINARY_NAME"
echo -e "  ${GREEN}✓${NC} Installed binary to $INSTALL_DIR/$BINARY_NAME"

# ── Check if ~/.local/bin is in PATH ──
if [[ ":$PATH:" != *":$INSTALL_DIR:"* ]]; then
    echo ""
    echo -e "  ${YELLOW}Warning:${NC} $INSTALL_DIR is not in your PATH."
    echo "  Add this to your ~/.bashrc or ~/.zshrc:"
    echo ""
    echo "    export PATH=\"\$HOME/.local/bin:\$PATH\""
    echo ""
fi

# ── Create systemd user service ──
mkdir -p "$SERVICE_DIR"

cat > "$SERVICE_DIR/$SERVICE_NAME.service" << EOF
[Unit]
Description=Smart File Organizer — Automatic file organizer
Documentation=https://github.com/Regentflame11/smart-file-organizer
After=default.target

[Service]
Type=simple
ExecStart=$INSTALL_DIR/$BINARY_NAME $WATCH_DIR
Restart=on-failure
RestartSec=5
StandardOutput=journal
StandardError=journal

[Install]
WantedBy=default.target
EOF

echo -e "  ${GREEN}✓${NC} Created systemd service: $SERVICE_NAME"

# ── Enable and start the service ──
systemctl --user daemon-reload
systemctl --user enable "$SERVICE_NAME.service"
systemctl --user start  "$SERVICE_NAME.service"

echo -e "  ${GREEN}✓${NC} Service enabled and started"

echo ""
echo -e "  ${GREEN}Installation complete!${NC}"
echo ""
echo "  Useful commands:"
echo "    systemctl --user status  $SERVICE_NAME   # Check status"
echo "    systemctl --user stop    $SERVICE_NAME   # Stop the service"
echo "    systemctl --user restart $SERVICE_NAME   # Restart"
echo "    journalctl --user -u     $SERVICE_NAME   # View logs"
echo ""
