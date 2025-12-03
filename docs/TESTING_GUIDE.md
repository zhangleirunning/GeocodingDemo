# Testing Your Geocoding System

## ✅ Your System is Working!

The system is running correctly. The issue was searching for addresses that don't exist in your demo data.

## Understanding Your Data

Your demo data contains addresses from **Salinas and Seaside, California** (not Seattle or generic "Main Street" addresses).

### Check what's in your data:

```bash
# View first few addresses
head -10 data/shard_0_data_demo.csv

# Count total records
wc -l data/shard_0_data_demo.csv
```

## Example Searches That Work

### 1. Search by Street Name:

```bash
curl -X POST http://localhost:18080/api/findAddress \
  -H "Content-Type: application/json" \
  -d '{"address": "MCKINNON"}'
```

### 2. Search by City:

```bash
curl -X POST http://localhost:18080/api/findAddress \
  -H "Content-Type: application/json" \
  -d '{"address": "Salinas"}'
```

### 3. Search by Street + City:

```bash
curl -X POST http://localhost:18080/api/findAddress \
  -H "Content-Type: application/json" \
  -d '{"address": "MCKINNON Salinas"}'
```

### 4. Search by Postal Code:

```bash
curl -X POST http://localhost:18080/api/findAddress \
  -H "Content-Type: application/json" \
  -d '{"address": "93906"}'
```

### 5. Search by Road:

```bash
curl -X POST http://localhost:18080/api/findAddress \
  -H "Content-Type: application/json" \
  -d '{"address": "ARNHEM ROAD"}'
```

## Pretty Print Results with jq

If you have `jq` installed:

```bash
curl -X POST http://localhost:18080/api/findAddress \
  -H "Content-Type: application/json" \
  -d '{"address": "Salinas"}' | jq .
```

Install jq:
```bash
# macOS
brew install jq

# Linux
sudo apt-get install jq
```

## Understanding the Response

```json
{
  "query": "Salinas",                    // Your search query
  "normalized_terms": ["SALINAS"],       // Terms after normalization
  "results": [                           // Top 5 results
    {
      "hash": "...",
      "longitude": -121.646,
      "latitude": 36.708,
      "number": "1531",
      "street": "MCKINNON STREET",
      "unit": "C",
      "city": "Salinas",
      "postcode": "93906",
      "shard_id": 0,                     // Which data node returned this
      "relevance_score": 125             // Higher = more relevant
    }
  ],
  "result_count": 5,                     // Number of results returned
  "successful_nodes": 2,                 // Both data nodes responded
  "failed_nodes": 0                      // No failures
}
```

## Find Streets in Your Data

```bash
# List unique street names
cut -d',' -f4 data/shard_0_data_demo.csv | sort -u | head -20

# Search for specific pattern
grep -i "STREET" data/shard_0_data_demo.csv | cut -d',' -f4 | sort -u

# List cities
cut -d',' -f6 data/shard_0_data_demo.csv | sort -u
```

## Testing Different Scenarios

### Test with Multiple Terms:
```bash
curl -X POST http://localhost:18080/api/findAddress \
  -H "Content-Type: application/json" \
  -d '{"address": "1531 MCKINNON Salinas"}'
```

### Test with Partial Match:
```bash
curl -X POST http://localhost:18080/api/findAddress \
  -H "Content-Type: application/json" \
  -d '{"address": "MCK"}'
```

### Test with No Results:
```bash
curl -X POST http://localhost:18080/api/findAddress \
  -H "Content-Type: application/json" \
  -d '{"address": "NonExistentStreet"}'
```

Expected response:
```json
{
  "query": "NonExistentStreet",
  "normalized_terms": ["NONEXISTENTSTREET"],
  "results": [],
  "result_count": 0,
  "successful_nodes": 2,
  "failed_nodes": 0
}
```

## Using Your Own Data

To use your own address data:

1. **Prepare CSV file** with these columns:
   ```
   LON,LAT,NUMBER,STREET,UNIT,CITY,DISTRICT,REGION,POSTCODE,ID,HASH
   ```

2. **Replace demo files**:
   ```bash
   cp your_data_shard_0.csv data/shard_0_data_demo.csv
   cp your_data_shard_1.csv data/shard_1_data_demo.csv
   ```

3. **Restart services**:
   ```bash
   docker-compose restart
   ```

## Monitoring

### View Logs:
```bash
# All services
docker-compose logs -f

# Specific service
docker-compose logs -f gateway
docker-compose logs -f data_node_0
```

### Check Health:
```bash
curl http://localhost:18080/health
```

Expected: `{"status":"healthy","data_nodes":2}`

### Check Service Info:
```bash
curl http://localhost:18080/
```

## Troubleshooting

### No Results?
- ✅ Check if the search term exists in your data
- ✅ Try searching for city names (Salinas, Seaside)
- ✅ Try searching for postal codes (93906, 93955)

### Gateway Not Responding?
```bash
# Check if services are running
docker-compose ps

# Restart services
docker-compose restart
```

### Data Nodes Not Loading Data?
```bash
# Check data node logs
docker-compose logs data_node_0 | grep "indexed"

# Should see: "Total records indexed: 10000"
```

## Performance Testing

### Test Response Time:
```bash
time curl -X POST http://localhost:18080/api/findAddress \
  -H "Content-Type: application/json" \
  -d '{"address": "Salinas"}' -o /dev/null -s
```

### Load Testing (with Apache Bench):
```bash
# Install ab (Apache Bench)
# macOS: brew install httpd
# Linux: sudo apt-get install apache2-utils

# Run 100 requests
ab -n 100 -c 10 -p request.json -T application/json \
  http://localhost:18080/api/findAddress
```

Where `request.json` contains:
```json
{"address": "Salinas"}
```

## Quick Reference

| Endpoint | Method | Purpose |
|----------|--------|---------|
| `/health` | GET | Check system health |
| `/` | GET | Service information |
| `/api/findAddress` | POST | Search for addresses |

---

**Your system is working perfectly! Just search for addresses that exist in your data.** 🎉
