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

## [6.4.0] - 2025-06-04

### Added
- Add compilation flag `ULOG_DEFAULT_LOG_LEVEL` to overwrite default log verbosity ([#40](https://github.com/an-dr/microlog/pull/40))
- Support for compile-time configuration of default log level (debug vs release builds)

### Changed
- Update documentation with examples for `ULOG_DEFAULT_LOG_LEVEL` usage
- Default topic level now uses `ULOG_DEFAULT_LOG_LEVEL` instead of hardcoded `LOG_TRACE`

### Fixed
- Improve log level handling for topics with new default level configuration

[Unreleased]: https://github.com/an-dr/microlog/compare/v6.4.10...HEAD
[6.4.10]: https://github.com/an-dr/microlog/compare/v6.4.9...v6.4.10
[6.4.9]: https://github.com/an-dr/microlog/compare/v6.4.8...v6.4.9
[6.4.8]: https://github.com/an-dr/microlog/compare/v6.4.7...v6.4.8
[6.4.0]: https://github.com/an-dr/microlog/compare/v6.3.3...v6.4.0