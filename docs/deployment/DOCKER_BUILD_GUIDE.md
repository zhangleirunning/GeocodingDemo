# Docker Build Guide

## Why is the first build slow?

The initial Docker build takes **5-10 minutes** because it needs to compile large C++ dependencies from source:

### Build Time Breakdown:

1. **vcpkg setup** (~2-3 minutes)
   - Clones vcpkg repository
   - Bootstraps vcpkg

2. **Dependency compilation** (~5-7 minutes) ⚠️ **Slowest part**
   - gRPC (large networking library)
   - Protobuf (serialization)
   - Crow (web framework)
   - Google Test + RapidCheck (testing)
   - Transitive dependencies (abseil, re2, openssl, etc.)

3. **Your code** (~30 seconds) ✅ **Fast!**
   - Compiles your C++ source files

**Total first build: ~8-10 minutes**

## Optimization: Docker Layer Caching

The Dockerfile is optimized to cache dependencies separately from your code:

```dockerfile
# Step 1: Copy only dependency files (vcpkg.json, CMakeLists.txt)
COPY vcpkg.json CMakeLists.txt ./

# Step 2: Install dependencies (CACHED - only runs if dependencies change)
RUN cmake ... && echo "Dependencies cached"

# Step 3: Copy your source code
COPY include ./include
COPY src ./src

# Step 4: Build your code (FAST - only your code compiles)
RUN make data_node_server gateway_server
```

### What This Means:

✅ **First build**: 8-10 minutes (compiles everything)
✅ **Subsequent builds**: 30-60 seconds (only your code)
✅ **After changing your code**: 30-60 seconds (dependencies cached)
⚠️ **After changing vcpkg.json**: 8-10 minutes (dependencies recompile)

## Build Commands

### Standard Build (Recommended)
```bash
docker-compose build
```

### Build with Progress Output
```bash
docker-compose build --progress=plain
```

### Force Clean Build (if you have issues)
```bash
docker-compose build --no-cache
```

### Build and Start Services
```bash
docker-compose up --build
```

## Speeding Up Builds

### 1. Use BuildKit (Modern Docker)
BuildKit enables parallel builds and better caching:

```bash
# Enable BuildKit (add to ~/.bashrc or ~/.zshrc)
export DOCKER_BUILDKIT=1
export COMPOSE_DOCKER_CLI_BUILD=1

# Then build normally
docker-compose build
```

### 2. Increase Docker Resources
In Docker Desktop settings:
- **CPUs**: Increase to 4-8 cores
- **Memory**: Increase to 8GB+
- **Disk**: Ensure sufficient space

### 3. Use Pre-built Base Image (Advanced)
For teams, you can:
1. Build once and push to a registry
2. Use that as a base image
3. Only compile your code on top

## Monitoring Build Progress

### Watch build in real-time:
```bash
docker-compose build --progress=plain 2>&1 | tee build.log
```

### Check what's being built:
```bash
docker-compose build --progress=plain 2>&1 | grep -E "Step|RUN|COPY"
```

## Troubleshooting

### Build fails with "out of memory"
Increase Docker memory allocation in Docker Desktop settings.

### Build is extremely slow (>20 minutes)
- Check Docker resource allocation
- Ensure you're not running other heavy processes
- Try enabling BuildKit (see above)

### "CMakeCache.txt directory mismatch" error
This is fixed by the `.dockerignore` file which excludes the local `build/` directory.

## After First Build

Once the first build completes:
- Dependencies are cached in Docker layers
- Rebuilds only compile your changed code (~30-60 seconds)
- You can iterate quickly on your code

## Quick Reference

| Scenario | Build Time | Command |
|----------|-----------|---------|
| First build | 8-10 min | `docker-compose build` |
| Code change | 30-60 sec | `docker-compose build` |
| Dependency change | 8-10 min | `docker-compose build` |
| Force rebuild | 8-10 min | `docker-compose build --no-cache` |

## Pro Tips

1. **Keep Docker running**: Don't quit Docker Desktop between builds
2. **Don't use `--no-cache`** unless necessary (it invalidates all caching)
3. **Use BuildKit**: Significantly faster parallel builds
4. **Allocate more resources**: More CPUs = faster compilation
5. **Be patient on first build**: Grab coffee ☕ - it's worth the wait!

## What Gets Cached?

✅ **Cached (fast rebuilds)**:
- vcpkg installation
- Dependency compilation (gRPC, Protobuf, etc.)
- Proto file generation

❌ **Not cached (triggers rebuild)**:
- Changes to `vcpkg.json`
- Changes to `CMakeLists.txt`
- Changes to your source code (but only your code recompiles)

---

**Bottom line**: First build is slow, but subsequent builds are fast thanks to Docker layer caching! 🚀
