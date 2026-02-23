#!/usr/bin/env bash
set -euo pipefail

BIN_DIR="$HOME/bin"

mkdir -p "$BIN_DIR"
cp cc "$BIN_DIR/cc"
cp _cc "$BIN_DIR/_cc"
chmod +x "$BIN_DIR/cc"

echo "Installed cc and _cc to $BIN_DIR"
echo ""
echo "Ensure your shell config has:"
echo '  export PATH="$HOME/bin:$PATH"'
echo '  fpath=($HOME/bin $fpath)'
echo ""
echo "Then restart your shell or run: source ~/.zshrc"
