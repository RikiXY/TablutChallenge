#!/usr/bin/env bash
# Plays a full game using the official referee Server.jar.
# usage: tools/smoke.sh [TIMEOUT_S=5] [WHITE_CMD] [BLACK_CMD]
# Player commands get "ROLE TIMEOUT 127.0.0.1" appended. Default player is PROJ/build/player. Use absolute paths.
# Env: TABLUT_REFEREE = referee checkout (default ~/tablut/TablutCompetition)
# Exit 0 only for a normal result with no timeout or illegal-move ruling.

# tools/smoke.sh 5 "$PWD/build/player" "java -jar $HOME/tablut/TablutCompetition/Tablut/Executables/RandomPlayer.jar"

set -euo pipefail

scriptDir=$(dirname "$(realpath "$0")")
projectDir=$(realpath "$scriptDir/..")
timeout=${1:-5}

white=${2:-$projectDir/build/player}
black=${3:-$projectDir/build/player}
server=${TABLUT_REFEREE:-$HOME/tablut/TablutCompetition}/Tablut/Executables/Server.jar

logDir=$projectDir/logs/$(date +%Y%m%d-%H%M%S)
mkdir -p "$logDir"

pids=()
cleanup() { kill "${pids[@]}" 2>/dev/null || true; } # Kills all background processes on exit
trap cleanup EXIT # Kills even if the script is interrupted

# Check that the server ports are free
for p in 5800 5801; do
    if (exec 3<>/dev/tcp/127.0.0.1/$p) 2>/dev/null; then echo "port $p busy"; exit 1; fi
done

# Start the server and both players in the background, inside the log directory
(cd "$logDir" && exec java -jar "$server" -t "$timeout" > server.log 2>&1) & pids+=($!)
sleep 1
(cd "$logDir" && exec $white WHITE "$timeout" 127.0.0.1 > white.log 2>&1) & pids+=($!); wp=$!
sleep 0.5
(cd "$logDir" && exec $black BLACK "$timeout" 127.0.0.1 > black.log 2>&1) & pids+=($!); bp=$!
wait "$wp" || true; wait "$bp" || true # Wait for both players to finish
sleep 0.5

result=$(grep -a -o 'RESULT: .*' "$logDir/server.log" || true)
moves=$(grep -a -c 'Suggested move' "$logDir/server.log" || true)
bad=$(grep -a -E -c 'Timeout|ERRORS|Error for' "$logDir/server.log" || true)
echo "result=${result:-none} moves=$moves adjudications=$bad logs=$logDir"
[ -n "$result" ] && [ "$bad" = 0 ]
