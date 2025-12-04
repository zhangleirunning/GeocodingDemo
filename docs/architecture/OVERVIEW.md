# System Architecture Overview

## Introduction

The Geocoding System is a distributed address search service built with C++17, designed for high performance and horizontal scalability. The system uses a sharded architecture where data is partitioned across multiple data nodes, with a gateway node providing a unified API interface.

## High-Level Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                         Client Layer                         │
│  (Web Browser, Mobile App, API Clients)                     │
└────────────────────────┬────────────────────────────────────┘
                         │ HTTP/REST
                         ▼
┌─────────────────────────────────────────────────────────────┐
│                      Gateway Node                            │
│  - HTTP Server (Crow Framework)                             │
│  - Request Routing & Load Balancing                         │
│  - Result Aggregation & Ranking                             │
│  - Web Frontend Serving                                      │
│  Port: 18080                                                 │
└────────────────────────┬────────────────────────────────────┘
                         │ gRPC (Parallel)
          ┌──────────────┴──────────────┐
          │                             │
          ▼                             ▼
┌──────────────────────┐      ┌──────────────────────┐
│   Data Node 0        │      │   Data Node 1        │
│   (Shard 0)          │      │   (Shard 1)          │
│                      │      │                      │
│  - RadixTreeIndex    │      │  - RadixTreeIndex    │
│  - ForwardIndex      │      │  - ForwardIndex      │
│  - CSV Parser        │      │  - CSV Parser        │
│  - gRPC Server       │      │  - gRPC Server       │
│  Port: 50051         │      │  Port: 50052         │
└──────────────────────┘      └──────────────────────┘
          │                             │
          ▼                             ▼
┌──────────────────────┐      ┌──────────────────────┐
│  Shard 0 Data        │      │  Shard 1 Data        │
│  (CSV File)          │      │  (CSV File)          │
└──────────────────────┘      └──────────────────────┘
```

## Core Components

### 1. Gateway Node
**Purpose:** API gateway and request orchestrator

**Responsibilities:**
- Accept HTTP REST requests
- Serve web frontend interface
- Distribute queries to data nodes via gRPC
- Aggregate results from multiple shards
- Rank results by relevance
- Handle partial failures gracefully

**Technology:**
- Crow (C++ web framework)
- gRPC client
- Async/parallel request handling

### 2. Data Nodes
**Purpose:** Distributed data storage and search

**Responsibilities:**
- Load and parse CSV address data
- Build and maintain search indexes
- Process search queries via gRPC
- Return matching address records

**Technology:**
- gRPC server
- Custom RadixTreeIndex (prefix search)
- Custom ForwardIndex (record storage)
- CSV parser

### 3. Indexes

#### RadixTreeIndex
- **Purpose:** Fast prefix-based text search
- **Structure:** Space-optimized trie (radix tree)
- **Indexed Fields:** Street, City, District, Region, Postcode
- **Performance:** O(k) search where k = prefix length

#### ForwardIndex
- **Purpose:** Fast record retrieval by ID
- **Structure:** Hash map (unordered_map)
- **Storage:** Complete address records
- **Performance:** O(1) lookup

## Data Flow

### Search Request Flow

```
1. Client → Gateway
   POST /api/findAddress {"address": "Main Street"}

2. Gateway Processing
   - Parse and validate request
   - Normalize search terms
   - Create gRPC requests

3. Gateway → Data Nodes (Parallel)
   - Async gRPC calls to all shards
   - Timeout: 5 seconds
   - Handle partial failures

4. Data Node Processing
   - Search RadixTreeIndex for matching IDs
   - Retrieve records from ForwardIndex
   - Return results via gRPC

5. Gateway Aggregation
   - Collect results from all nodes
   - Calculate relevance scores
   - Remove duplicates
   - Sort by relevance
   - Return top 5 results

6. Gateway → Client
   JSON response with ranked results
```

## Scalability

### Horizontal Scaling
- **Data Nodes:** Add more shards to distribute data
- **Gateway Nodes:** Add more gateways behind load balancer
- **Independent Scaling:** Scale data and gateway layers separately

### Sharding Strategy
- Data partitioned by hash or range
- Each shard operates independently
- No cross-shard dependencies
- Parallel query execution

## Performance Characteristics

### Data Node
- **Index Build:** O(n × m) where n = records, m = avg fields
- **Search:** O(k + r) where k = prefix length, r = results
- **Memory:** ~250MB per 1M records

### Gateway
- **Query Latency:** Max(shard latencies) + aggregation time
- **Typical:** < 50ms for queries with < 100 results
- **Throughput:** Limited by data node capacity

## Fault Tolerance

### Partial Failures
- Gateway continues with available shards
- Returns partial results with status indicator
- Logs failures for monitoring

### Complete Failures
- Gateway returns 503 Service Unavailable
- Clear error messages
- Graceful degradation

## Technology Stack

### Core
- **Language:** C++17
- **Build System:** CMake
- **Package Manager:** vcpkg

### Networking
- **HTTP Server:** Crow
- **RPC:** gRPC + Protocol Buffers
- **Async:** std::async, std::future

### Testing
- **Unit Tests:** Google Test
- **Property Tests:** RapidCheck
- **Integration Tests:** Mock gRPC servers

### Deployment
- **Containerization:** Docker
- **Orchestration:** Docker Compose
- **OS:** Debian (container), macOS/Linux (development)

## Design Principles

1. **Separation of Concerns**
   - Clear boundaries between components
   - Independent data and gateway layers

2. **Horizontal Scalability**
   - Stateless gateway nodes
   - Independent data shards

3. **Performance**
   - Custom indexes for fast search
   - Parallel query execution
   - Efficient C++ implementation

4. **Reliability**
   - Graceful failure handling
   - Health checks
   - Comprehensive logging

5. **Maintainability**
   - Clean code structure
   - Comprehensive tests
   - Clear documentation

## Future Enhancements

### Short Term
- [ ] Index persistence (save/load)
- [ ] Fuzzy matching
- [ ] Geospatial queries

### Medium Term
- [ ] Replication for high availability
- [ ] Query caching
- [ ] Metrics and monitoring

### Long Term
- [ ] Auto-scaling
- [ ] Multi-region deployment
- [ ] Real-time index updates

## Related Documentation

- [Data Node Architecture](DATA_NODE.md)
- [Gateway Architecture](GATEWAY.md)
- [API Documentation](../api/REST_API.md)
- [Deployment Guide](../deployment/DOCKER_BUILD_GUIDE.md)
