# syntax=docker/dockerfile:1
FROM mcr.microsoft.com/devcontainers/cpp:ubuntu-22.04

RUN apt-get update \
 && apt-get install -y --no-install-recommends \
      lcov \
      meson \
 && rm -rf /var/lib/apt/lists/*
