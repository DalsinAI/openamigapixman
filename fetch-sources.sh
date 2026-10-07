#!/bin/sh
set -eu
HERE=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
DIR="$HERE/tarballs"
FILE="$DIR/pixman-0.46.4.tar.gz"
URL="https://cairographics.org/releases/pixman-0.46.4.tar.gz"
SHA="d09c44ebc3bd5bee7021c79f922fe8fb2fb57f7320f55e97ff9914d2346a591c"
mkdir -p "$DIR"
if [ ! -f "$FILE" ]; then
  if command -v curl >/dev/null 2>&1; then
    curl -fL "$URL" -o "$FILE.tmp"
  elif command -v wget >/dev/null 2>&1; then
    wget -O "$FILE.tmp" "$URL"
  else
    echo "need curl or wget to fetch $URL" >&2; exit 2
  fi
  mv "$FILE.tmp" "$FILE"
fi
echo "$SHA  $FILE" | sha256sum -c -
