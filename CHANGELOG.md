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

## [6.3.3] - 2025-06-01

### Changed
- Remove unit tests from release workflow to improve release process

## [6.3.1] - 2025-05-31

### Fixed
- Fix va_args bug causing segmentation fault on Linux ([#55](https://github.com/an-dr/microlog/pull/55))
- Add proper va_list copying to avoid platform-specific issues
- Create event copy to avoid va_list issues in callback processing

### Added
- Add devcontainer for local builds and tests
- Add cross-platform development environment support

## [6.3.0] - 2025-05-22

### Added
- Add logging level configuration to topics ([#36](https://github.com/an-dr/microlog/pull/36))
- Add `ulog_set_topic_level()` function to set debug level for individual topics
- Support for per-topic logging level control

### Changed
- By default, topic logging level is set to `LOG_TRACE`
- Topics now respect both global logging level and topic-specific level
- Update README and examples with topic logging level configuration

## [6.2.1] - 2025-04-26

### Added
- Add minimalistic source package for easier distribution
- Add source package generation to build workflow

### Changed
- Update installation documentation to include source package option

## [6.2.0] - 2025-04-26

### Added  
- Add CMake package support with `find_package` integration ([#28](https://github.com/an-dr/microlog/pull/28))
- Add CI workflows for PR, merge and new version tag
- Add build tests and package testing
- Add Meson package support

### Changed
- Improve project packaging and distribution
- Add comprehensive CI/CD pipeline
- Update README with CMake and Meson package installation instructions

## [6.1.0] - 2025-04-18

### Changed
- Refactoring to remove non-standard `fmemopen` ([#25](https://github.com/an-dr/microlog/pull/25))
- Implement internal `vprint` and `print` functions accepting streams and buffers
- Add support for MSVC and other non-POSIX compilers
- Improve cross-platform compatibility

## [6.0.1] - 2024-12-10

### Fixed
- Fix missing space between log level and message ([#21](https://github.com/an-dr/microlog/pull/21))
- Improve log message formatting consistency
- Fix time formatting for custom prefix feature

## [6.0.0] - 2024-09-18

### Added
- Add Log Topics Feature ([#18](https://github.com/an-dr/microlog/pull/18))
- Support for filtering log messages by subsystems (e.g., "network", "storage")
- Add topic-specific logging macros (`logt_trace`, `logt_debug`, etc.)
- Support for both static and dynamic topic allocation
- Add topic management functions (`ulog_add_topic`, `ulog_enable_topic`, etc.)

### Changed
- Rename Extra Destinations to Extra Outputs for clarity
- Update log message format to include topic information
- Major version bump due to API changes

### Fixed
- Various bug fixes and improvements