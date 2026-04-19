# syntax=docker/dockerfile:1

FROM gcc:14-bookworm AS build
WORKDIR /src

COPY Makefile ./
COPY game.h ./
COPY main.c map.c enemy.c config.c save.c ./

RUN make clean && make

FROM debian:bookworm-slim AS runtime
WORKDIR /game

COPY --from=build /src/abyss_walker ./abyss_walker
COPY config.txt ./config.txt

# Save files and leaderboard are redirected to /data so they can be persisted
# with a volume mount while keeping the app code immutable in /game.
RUN mkdir -p /data \
    && ln -sf /data/maze.sav /game/maze.sav \
    && ln -sf /data/leaderboard.csv /game/leaderboard.csv

VOLUME ["/data"]

ENTRYPOINT ["./abyss_walker"]
