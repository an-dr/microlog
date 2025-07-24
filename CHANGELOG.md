# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [6.4.10] - 2025-07-14

### Changed
- Rename internal macros and expose feature flag defines to users ([#93](https://github.com/an-dr/microlog/pull/93))
- Add `ULOG_` prefix to all public macros for consistency
- Move internal macros not intended for public use to source file
- Add public feature flags to README for user access

## [6.4.9] - 2025-06-28

### Fixed
- Fix dynamic topic printing on Windows ([#89](https://github.com/an-dr/microlog/issues/89))

### Changed
- Refactor topics implementation for better maintainability
- Add primitive performance test

## [6.4.8] - 2025-06-26

### Fixed
- Fix not unlocked mutex after skipping logs under the current level

## [6.4.7] - 2025-06-25

### Changed
- Refactor time, callbacks, log and topics ([#88](https://github.com/an-dr/microlog/pull/88))
- Update design documentation
- Refactor and unify callback processing
- Refactor time and event printing logic
- Improve topic_process, create topic private structure

## [6.4.6] - 2025-06-22

### Changed
- Refactor code for maintainability and further feature development ([#79](https://github.com/an-dr/microlog/issues/79), [#86](https://github.com/an-dr/microlog/issues/86))
- Add CONTRIBUTING.md file
- Reorganize documentation structure
- Add code style and organization guidelines

## [6.4.5] - 2025-06-21

### Changed
- Update version to v6.4.5

## [6.4.4] - 2025-06-18

### Fixed
- Fix ulog_add_topic allowing adding of the same topic twice for static allocation ([#77](https://github.com/an-dr/microlog/pull/77))
- Fix ulog_add_topic implementation to be consistent across different topic allocation modes (static and dynamic)
- Closes [#76](https://github.com/an-dr/microlog/issues/76)

## [6.4.3] - 2025-06-15

### Changed
- Update meson and cmake packages so they build a static target ([#72](https://github.com/an-dr/microlog/pull/72))
- Improve CMake and Meson package configuration
- Add proper static library targets

## [6.4.2] - 2025-06-14

### Changed
- Add input validation to level-related functions, minor refactoring ([#70](https://github.com/an-dr/microlog/pull/70))
- Add script to set version after installation
- Rename `write_formatted_message` to `print_formatted_message`
- Improve input validation for level functions

## [6.4.1] - 2025-06-12

### Changed
- Remove replacement of disabled functions to no-op ([#61](https://github.com/an-dr/microlog/pull/61))
- Remove unexpected behavior where disabled functions were replaced with no-op macros
- Improve example code formatting and documentation

## [6.4.0] - 2025-06-04

### Added
- Add compilation flag `ULOG_DEFAULT_LOG_LEVEL` to overwrite default log verbosity ([#40](https://github.com/an-dr/microlog/pull/40))
- Support for compile-time configuration of default log level (debug vs release builds)

### Changed
- Update documentation with examples for `ULOG_DEFAULT_LOG_LEVEL` usage
- Default topic level now uses `ULOG_DEFAULT_LOG_LEVEL` instead of hardcoded `LOG_TRACE`

### Fixed
- Improve log level handling for topics with new default level configuration