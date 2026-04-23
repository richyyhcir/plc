#!/usr/bin/env bash
# Build and run Abyss Walker in Docker.
# Saves (maze.sav) and leaderboard persist in the abyss_walker_data volume.
#
# Usage:
#   docker/run.sh              # build (if needed) and run
#   docker/run.sh --rebuild    # force a no-cache rebuild before running
#   docker/run.sh --reset      # wipe the save/leaderboard volume first

set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$REPO_ROOT"

COMPOSE=(docker compose -f docker/docker-compose.yml)

for arg in "$@"; do
  case "$arg" in
    --reset)
      "${COMPOSE[@]}" down -v
      ;;
    --rebuild)
      "${COMPOSE[@]}" build --no-cache abyss-walker
      ;;
    -h|--help)
      sed -n '2,8p' "$0" | sed 's/^# \{0,1\}//'
      exit 0
      ;;
    *)
      echo "Unknown option: $arg" >&2
      exit 1
      ;;
  esac
done

exec "${COMPOSE[@]}" run --rm --build abyss-walker
