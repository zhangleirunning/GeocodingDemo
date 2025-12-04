# REST API Documentation

## Base URL

```
http://localhost:18080
```

## Endpoints

### 1. Root Endpoint

Get service information.

**Endpoint:** `GET /`

**Response:**
```json
{
  "service": "Geocoding Gateway",
  "version": "1.0.0",
  "endpoints": ["/health", "/api/findAddress"]
}
```

**Status Codes:**
- `200 OK` - Success

---

### 2. Health Check

Check service health and data node connectivity.

**Endpoint:** `GET /health`

**Response:**
```json
{
  "status": "healthy",
  "data_nodes": 2
}
```

**Fields:**
- `status` (string) - Service status: "healthy" or "unhealthy"
- `data_nodes` (integer) - Number of connected data nodes

**Status Codes:**
- `200 OK` - Service is healthy

**Example:**
```bash
curl http://localhost:18080/health
```

---

### 3. Find Address

Search for addresses across all data nodes.

**Endpoint:** `POST /api/findAddress`

**Request Body:**
```json
{
  "address": "string"
}
```

**Parameters:**
- `address` (string, required) - Search query (street name, city, postal code, etc.)

**Response:**
```json
{
  "query": "Main Street",
  "normalized_terms": ["MAIN", "STREET"],
  "results": [
    {
      "hash": "abc123...",
      "longitude": -122.608996,
      "latitude": 47.166377,
      "number": "123",
      "street": "MAIN STREET",
      "unit": "APT 1",
      "city": "Seattle",
      "postcode": "98101",
      "shard_id": 0,
      "relevance_score": 125.5
    }
  ],
  "result_count": 5,
  "successful_nodes": 2,
  "failed_nodes": 0
}
```

**Response Fields:**
- `query` (string) - Original search query
- `normalized_terms` (array) - Normalized search terms used
- `results` (array) - Array of address records (max 5)
  - `hash` (string) - Unique record hash
  - `longitude` (number) - Longitude coordinate
  - `latitude` (number) - Latitude coordinate
  - `number` (string) - Street number
  - `street` (string) - Street name (normalized)
  - `unit` (string) - Unit/apartment number
  - `city` (string) - City name
  - `postcode` (string) - Postal code
  - `shard_id` (integer) - Data node that returned this result
  - `relevance_score` (number) - Relevance score (higher = more relevant)
- `result_count` (integer) - Number of results returned
- `successful_nodes` (integer) - Number of data nodes that responded successfully
- `failed_nodes` (integer) - Number of data nodes that failed

**Status Codes:**
- `200 OK` - Success, results found (may be empty)
- `207 Multi-Status` - Partial success (some nodes failed but results available)
- `400 Bad Request` - Invalid request (missing/empty address)
- `500 Internal Server Error` - Server error
- `503 Service Unavailable` - All data nodes failed

**Examples:**

Search by city:
```bash
curl -X POST http://localhost:18080/api/findAddress \
  -H "Content-Type: application/json" \
  -d '{"address": "Salinas"}'
```

Search by street:
```bash
curl -X POST http://localhost:18080/api/findAddress \
  -H "Content-Type: application/json" \
  -d '{"address": "MCKINNON STREET"}'
```

Search by postal code:
```bash
curl -X POST http://localhost:18080/api/findAddress \
  -H "Content-Type: application/json" \
  -d '{"address": "93906"}'
```

Search with multiple terms:
```bash
curl -X POST http://localhost:18080/api/findAddress \
  -H "Content-Type: application/json" \
  -d '{"address": "123 Main Street Seattle"}'
```

---

## Error Responses

### 400 Bad Request

**Causes:**
- Missing `address` field
- Empty `address` value
- Invalid JSON

**Example:**
```json
{
  "error": "Missing 'address' field in request body"
}
```

### 500 Internal Server Error

**Causes:**
- Unexpected server error
- Exception during processing

**Example:**
```json
{
  "error": "Internal server error",
  "details": "Exception message here"
}
```

### 503 Service Unavailable

**Causes:**
- All data nodes failed to respond
- No data nodes available

**Example:**
```json
{
  "error": "All data nodes failed to respond",
  "query": "Main Street",
  "normalized_terms": ["MAIN", "STREET"],
  "results": [],
  "result_count": 0,
  "successful_nodes": 0,
  "failed_nodes": 2
}
```

---

## Search Behavior

### Normalization

All search terms are normalized before querying:
- Converted to uppercase
- Whitespace trimmed and collapsed
- Street suffixes expanded (ST → STREET, AVE → AVENUE, etc.)

### Matching

- Searches are performed on normalized address fields
- All query terms must match (AND logic)
- Prefix matching is supported
- Case-insensitive

### Ranking

Results are ranked by relevance score based on:
1. **Match percentage** - How many query terms matched
2. **Position bonus** - Matches at start of field score higher
3. **Field priority** - Street > City > Postcode
4. **Completeness** - More complete addresses score higher

### Result Limits

- Maximum 5 results returned
- Results sorted by relevance (descending)
- Duplicates removed (same number + street + city + postcode)

---

## Rate Limiting

Currently no rate limiting is implemented. For production use, consider:
- Request rate limits per IP
- Concurrent request limits
- Query complexity limits

---

## CORS

CORS is enabled for all origins (`Access-Control-Allow-Origin: *`).

For production, configure specific allowed origins.

---

## Web Frontend

The gateway also serves a web frontend at the root URL:

**URL:** `http://localhost:18080/`

**Features:**
- Interactive search interface
- Real-time results
- Result ranking display
- Google Maps integration
- Mobile responsive

---

## Examples

### Successful Search

**Request:**
```bash
curl -X POST http://localhost:18080/api/findAddress \
  -H "Content-Type: application/json" \
  -d '{"address": "Salinas"}'
```

**Response:**
```json
{
  "query": "Salinas",
  "normalized_terms": ["SALINAS"],
  "results": [
    {
      "hash": "46a6ea62641c0d1c",
      "longitude": -121.6461331,
      "latitude": 36.7082169,
      "number": "1531",
      "street": "MCKINNON STREET",
      "unit": "C",
      "city": "Salinas",
      "postcode": "93906",
      "shard_id": 0,
      "relevance_score": 125.0
    }
  ],
  "result_count": 5,
  "successful_nodes": 2,
  "failed_nodes": 0
}
```

### No Results

**Request:**
```bash
curl -X POST http://localhost:18080/api/findAddress \
  -H "Content-Type: application/json" \
  -d '{"address": "NonexistentStreet"}'
```

**Response:**
```json
{
  "query": "NonexistentStreet",
  "normalized_terms": ["NONEXISTENTSTREET"],
  "results": [],
  "result_count": 0,
  "successful_nodes": 2,
  "failed_nodes": 0
}
```

### Partial Failure

**Response:**
```json
{
  "query": "Main Street",
  "normalized_terms": ["MAIN", "STREET"],
  "results": [...],
  "result_count": 3,
  "successful_nodes": 1,
  "failed_nodes": 1
}
```

**Status:** `207 Multi-Status`

---

## Client Libraries

### JavaScript/TypeScript

```javascript
const API_BASE_URL = 'http://localhost:18080';

async function searchAddress(query) {
  const response = await fetch(`${API_BASE_URL}/api/findAddress`, {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
    },
    body: JSON.stringify({ address: query })
  });

  return await response.json();
}

// Usage
const results = await searchAddress('Salinas');
console.log(results);
```

### Python

```python
import requests

API_BASE_URL = 'http://localhost:18080'

def search_address(query):
    response = requests.post(
        f'{API_BASE_URL}/api/findAddress',
        json={'address': query}
    )
    return response.json()

# Usage
results = search_address('Salinas')
print(results)
```

### cURL

```bash
#!/bin/bash

API_BASE_URL="http://localhost:18080"

search_address() {
  curl -s -X POST "$API_BASE_URL/api/findAddress" \
    -H "Content-Type: application/json" \
    -d "{\"address\": \"$1\"}"
}

# Usage
search_address "Salinas"
```

---

## Monitoring

### Health Check Monitoring

```bash
# Simple health check
curl -f http://localhost:18080/health || echo "Service unhealthy"

# With jq for parsing
curl -s http://localhost:18080/health | jq '.status'
```

### Performance Monitoring

Monitor these metrics:
- Response time (target: < 100ms)
- Success rate (target: > 99%)
- Data node availability
- Result count distribution

---

## Changelog

### v1.0.0 (Current)
- Initial API release
- Basic search functionality
- Health check endpoint
- Web frontend serving

### Future
- Autocomplete endpoint
- Batch search endpoint
- Advanced filtering
- Pagination support
