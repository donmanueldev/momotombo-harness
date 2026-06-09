# RedisForge

## Product Vision

RedisForge es una aplicación desktop moderna para desarrolladores, DevOps Engineers y Platform Engineers que permite inspeccionar, administrar y monitorear instancias Redis de forma visual.

El objetivo es proporcionar una experiencia rápida, intuitiva y orientada a productividad para trabajar con Redis en entornos de desarrollo, staging y producción.

---

# Problem Statement

Redis es ampliamente utilizado como:

- Cache distribuido
- Session Store
- Queue Backend
- Pub/Sub Broker
- Stream Processing Engine
- Vector Store
- Rate Limiter

Sin embargo, las herramientas existentes suelen presentar uno o más problemas:

- Interfaces complejas
- Alto consumo de memoria
- Mala experiencia para inspeccionar datos
- Escasa visibilidad de Streams
- Poca productividad para debugging

RedisForge busca resolver estos problemas mediante una experiencia desktop nativa, rápida y enfocada en workflows reales de ingeniería.

---

# Target Users

## Primary

- Backend Engineers
- Platform Engineers
- DevOps Engineers
- SREs
- Solution Architects

## Secondary

- QA Engineers
- Mobile Developers
- Full Stack Developers

---

# Core Features

## Connection Management

- Multiple Redis Connections
- Connection Profiles
- SSL/TLS Support
- Redis Sentinel
- Redis Cluster
- SSH Tunnel Support
- Connection Testing
- Favorites

---

## Keys Explorer

### Browse Keys

- SCAN support
- Infinite scrolling
- Namespace navigation
- Key grouping

### Key Information

- Key Name
- Key Type
- Memory Usage
- TTL
- Creation Metadata

### Search

- Search by pattern
- Wildcard search
- Namespace filtering

Examples:

user:*
cache:*
session:*

---

## Value Inspector

### String Viewer

- Plain text
- Syntax highlighting

### JSON Viewer

- Pretty print
- Tree mode
- Raw mode
- Validation

### Hash Viewer

- Field table
- Edit fields

### List Viewer

- Pagination
- Push/Pop operations

### Set Viewer

- Members explorer

### Sorted Set Viewer

- Scores
- Ranking

---

## TTL Manager

- TTL visualization
- Expiration countdown
- Extend expiration
- Remove expiration
- Bulk TTL updates

---

## Key Editor

- Create key
- Update key
- Delete key
- Duplicate key
- Rename key

---

## Memory Analysis

- Key memory usage
- Largest keys
- Namespace analysis
- Growth detection

---

# Pub/Sub Console

## Subscribe

- Multiple channels
- Wildcard subscriptions

## Publish

- Send messages
- JSON messages
- Templates

## Monitor

- Real-time events
- Search messages
- Export logs

---

# Redis Streams

## Streams Explorer

- Stream listing
- Message browsing
- Infinite pagination

## Message Inspector

- ID
- Fields
- Metadata

## Producer

- Add messages

## Consumer Groups

- Groups
- Pending messages
- Consumers
- Acknowledgements

---

# Monitoring

## Metrics

- Connected clients
- Memory usage
- CPU usage
- Hit ratio
- Evicted keys

## Dashboard

- Real-time charts
- Throughput
- Commands/sec

---

# Command Console

Integrated Redis CLI.

Examples:

GET user:1

TTL user:1

INFO

XRANGE notifications - +

---

# Data Export

- JSON
- CSV
- TXT

---

# Security

- Read-only mode
- Production protection
- Dangerous command confirmation

Protected commands:

FLUSHALL

FLUSHDB

DEL *

---

# Roadmap

## V1

- Connections
- Keys Explorer
- JSON Viewer
- TTL Viewer

## V2

- Pub/Sub
- Monitoring
- CLI

## V3

- Streams
- Consumer Groups

## V4

- Cluster
- Sentinel
- SSH Tunnels

## V5

- AI Assistant
- Query Suggestions
- Performance Analyzer

---

# Tech Stack

Frontend:
- Qt 6
- C++20

Backend Integration:
- hiredis

Storage:
- SQLite

Build:
- CMake

Testing:
- GoogleTest

CI/CD:
- GitHub Actions

Packaging:
- Windows MSI
- macOS DMG
- Linux AppImage

---

# Success Metrics

- Startup < 2s
- Search < 200ms
- 100k+ keys supported
- Memory footprint < 150MB
- Native experience on Windows, macOS and Linux