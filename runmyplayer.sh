#!/bin/sh
here=$(dirname "$(readlink -f "$0")")
player="$here/player"
[ -x "$player" ] || player="$here/build/player"
exec "$player" "$@"
