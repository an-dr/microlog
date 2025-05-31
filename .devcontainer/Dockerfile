# VS Code Dev Container with Ubuntu, CMake, and lcov
FROM mcr.microsoft.com/devcontainers/base:ubuntu-22.04

# Install CMake, Ninja, lcov, and other build essentials
RUN apt-get update && \
    apt-get install -y \
        cmake \
        ninja-build \
        build-essential \
        lcov \
        git \
        curl \
        wget \
        gdb \
    && apt-get clean && rm -rf /var/lib/apt/lists/*

# Optional: Install PowerShell (for running .ps1 scripts)
RUN wget -q https://packages.microsoft.com/config/ubuntu/22.04/packages-microsoft-prod.deb && \
    dpkg -i packages-microsoft-prod.deb && \
    apt-get update && \
    apt-get install -y powershell && \
    rm packages-microsoft-prod.deb
