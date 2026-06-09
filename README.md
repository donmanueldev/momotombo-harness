# RedisForge

RedisForge is a cross-platform Redis Desktop Manager built with C++20, Qt 6, CMake, hiredis, and Clean Architecture.

## Current Status

The repository now includes:

- Layered project structure under `src/app`, `src/domain`, `src/infrastructure`, `src/presentation`, and `src/shared`
- Connection Management module with:
  - connection profile validation
  - profile persistence abstraction
  - async Redis connection test flow
  - hiredis-backed infrastructure adapter
  - Qt ViewModel and widget UI
- GoogleTest unit tests and an opt-in Redis integration test
- CMake dependency bootstrapping for Qt 6, hiredis, and GoogleTest

## Folder Structure

```text
src/
├── app/
├── domain/
├── infrastructure/
├── presentation/
├── shared/
└── main.cpp
```

## Requirements

- CMake 3.24+
- C++20 compiler
- Qt 6.5+ with `Core`, `Widgets`, and `Concurrent`
- Ninja recommended

If `hiredis` or `GoogleTest` are not installed, CMake can fetch them automatically when `REDISFORGE_FETCH_MISSING_DEPENDENCIES=ON`.

## Build

Using presets:

```bash
cmake --preset debug
cmake --build --preset debug
ctest --preset debug
```

Quick local build on this machine:

```bash
./build.sh
```

Without presets:

```bash
cmake -S . -B build/debug -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build/debug
ctest --test-dir build/debug --output-on-failure
```

Run the desktop app:

```bash
./build/debug/src/RedisForge
```

## Important

Do not compile `src/main.cpp` directly with raw `c++`.

This is a Qt 6 + CMake application, so `main.cpp` depends on:

- Qt include paths
- Qt framework link flags
- generated Qt MOC files
- the other RedisForge targets and source files

Use `./build.sh` or the CMake commands above instead.

## Redis Integration Test

The hiredis integration test is disabled by default. Enable it with:

```bash
export REDISFORGE_RUN_REDIS_INTEGRATION_TESTS=1
export REDISFORGE_TEST_REDIS_HOST=127.0.0.1
export REDISFORGE_TEST_REDIS_PORT=6379
ctest --test-dir build/debug --output-on-failure
```

Optional:

```bash
export REDISFORGE_TEST_REDIS_USERNAME=default
export REDISFORGE_TEST_REDIS_PASSWORD=secret
```

## Architecture Rules

- UI depends on ViewModels only
- Application layer orchestrates use cases
- Domain contains business rules and abstractions
- Infrastructure implements domain contracts
- Redis requests must stay off the UI thread
- Pagination is the default interaction model
