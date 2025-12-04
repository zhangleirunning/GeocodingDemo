# Docker Migration Testing Guide

## ✅ Automated Tests Completed

1. ✅ **Configuration Validation** - All configs are valid
2. ✅ **Development Config** - Valid
3. ✅ **Production Config** - Valid
4. ✅ **Symlink** - Working correctly
5. ✅ **Path Resolution** - Fixed and validated

## 🧪 Manual Testing Steps

### Test 1: Build Services

```bash
# From project root (using symlink)
docker-compose build

# Expected: Build completes successfully
# Time: 5-10 minutes (first time)
```

### Test 2: Start Services

```bash
# Start all services
docker-compose up -d

# Check status
docker-compose ps

# Expected: All 3 services running
# - geocoding_data_node_0 (healthy)
# - geocoding_data_node_1 (healthy)
# - geocoding_gateway (healthy)
```

### Test 3: Verify Services

```bash
# Test gateway health
curl http://localhost:18080/health

# Expected: {"status":"healthy","data_nodes":2}

# Test web frontend
open http://localhost:18080/
# Or: curl http://localhost:18080/ | head -20

# Expected: HTML web frontend loads

# Test search
curl -X POST http://localhost:18080/api/findAddress \
  -H "Content-Type: application/json" \
  -d '{"address": "Salinas"}'

# Expected: JSON with search results
```

### Test 4: View Logs

```bash
# All services
docker-compose logs

# Specific service
docker-compose logs gateway
docker-compose logs data_node_0

# Follow logs
docker-compose logs -f
```

### Test 5: Test Development Mode

```bash
# Stop current services
docker-compose down

# Start in development mode
docker-compose -f docker/docker-compose.yml -f docker/docker-compose.dev.yml up

# Expected: Services start with DEBUG logging
```

### Test 6: Test Production Mode

```bash
# Stop current services
docker-compose down

# Start in production mode
docker-compose -f docker/docker-compose.yml -f docker/docker-compose.prod.yml up -d

# Check resource limits
docker stats

# Expected: Services running with resource limits
```

### Test 7: Rebuild and Restart

```bash
# Rebuild and restart
docker-compose up -d --build

# Expected: Services rebuild and restart successfully
```

### Test 8: Clean Shutdown

```bash
# Stop services
docker-compose down

# Verify stopped
docker-compose ps

# Expected: No services running
```

## 🔍 What to Check

### ✅ Build Phase
- [ ] Build completes without errors
- [ ] All dependencies install correctly
- [ ] Executables compile successfully
- [ ] Images are created

### ✅ Runtime Phase
- [ ] All 3 services start
- [ ] Health checks pass
- [ ] Services can communicate (gateway → data nodes)
- [ ] Ports are accessible

### ✅ Functionality
- [ ] Gateway health endpoint works
- [ ] Web frontend loads at http://localhost:18080/
- [ ] Search API returns results
- [ ] Data nodes respond to gRPC calls

### ✅ Configuration
- [ ] Environment variables are set correctly
- [ ] Volume mounts work (data files accessible)
- [ ] Network communication works
- [ ] Logs are visible

## 🐛 Troubleshooting

### Build Fails

```bash
# Clean everything
docker-compose down
docker system prune -f

# Rebuild from scratch
docker-compose build --no-cache
```

### Services Won't Start

```bash
# Check logs
docker-compose logs gateway
docker-compose logs data_node_0

# Check Docker resources
docker system df

# Increase Docker memory/CPU in Docker Desktop settings
```

### Port Conflicts

```bash
# Check what's using ports
lsof -i :18080
lsof -i :50051
lsof -i :50052

# Kill conflicting processes or change ports in docker-compose.yml
```

### Volume Mount Issues

```bash
# Verify data files exist
ls -la data/

# Check volume mounts
docker-compose config | grep volumes -A 2
```

## ✅ Success Criteria

Your Docker migration is successful if:

1. ✅ `docker-compose build` completes without errors
2. ✅ `docker-compose up -d` starts all services
3. ✅ `docker-compose ps` shows all services healthy
4. ✅ `curl http://localhost:18080/health` returns healthy status
5. ✅ Web frontend loads in browser at http://localhost:18080/
6. ✅ Search API returns results
7. ✅ Logs show no errors

## 📊 Expected Results

### docker-compose ps
```
NAME                    STATUS
geocoding_data_node_0   Up (healthy)
geocoding_data_node_1   Up (healthy)
geocoding_gateway       Up (healthy)
```

### Health Check
```json
{
  "status": "healthy",
  "data_nodes": 2
}
```

### Search API
```json
{
  "query": "Salinas",
  "result_count": 5,
  "successful_nodes": 2,
  "failed_nodes": 0,
  "results": [...]
}
```

## 🎯 Quick Test Script

Save this as `test-docker.sh`:

```bash
#!/bin/bash

echo "🧪 Testing Docker Migration..."
echo ""

echo "1️⃣ Validating configuration..."
docker-compose config --quiet && echo "✓ Config valid" || echo "✗ Config invalid"

echo ""
echo "2️⃣ Building services..."
docker-compose build --quiet && echo "✓ Build successful" || echo "✗ Build failed"

echo ""
echo "3️⃣ Starting services..."
docker-compose up -d && echo "✓ Services started" || echo "✗ Services failed to start"

echo ""
echo "4️⃣ Waiting for services to be ready..."
sleep 10

echo ""
echo "5️⃣ Testing health endpoint..."
curl -s http://localhost:18080/health | grep -q "healthy" && echo "✓ Health check passed" || echo "✗ Health check failed"

echo ""
echo "6️⃣ Testing search API..."
curl -s -X POST http://localhost:18080/api/findAddress \
  -H "Content-Type: application/json" \
  -d '{"address": "Salinas"}' | grep -q "result_count" && echo "✓ Search API works" || echo "✗ Search API failed"

echo ""
echo "7️⃣ Checking service status..."
docker-compose ps

echo ""
echo "✅ Testing complete!"
echo ""
echo "To view logs: docker-compose logs -f"
echo "To stop: docker-compose down"
```

Run with:
```bash
chmod +x test-docker.sh
./test-docker.sh
```

## 📝 Notes

- First build takes 5-10 minutes
- Subsequent builds are much faster (cached layers)
- Services need ~10 seconds to fully start
- Gateway health check may take 15 seconds to pass

---

**Ready to test?** Run the commands above and verify everything works!
