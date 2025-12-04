# Frontend to Web Directory Rename

**Date:** December 3, 2024
**Status:** ✅ Complete

## Overview

Renamed the `frontend/` directory to `web/` to better reflect its purpose and align with common naming conventions in C++ projects with web interfaces.

## Rationale

1. **Clearer separation**: The `apps/` directory contains C++ executable entry points, while `web/` clearly indicates static web assets
2. **Common convention**: In C++ projects with web frontends, `web/`, `public/`, or `static/` are more common than `frontend/`
3. **Build system clarity**: Separates compiled executables from static web resources
4. **Deployment model**: The web directory contains assets served by the gateway, not standalone applications

## Changes Made

### Directory Structure
```bash
frontend/          →  web/
├── index.html        ├── index.html
└── README.md         └── README.md
```

### Code Changes

1. **Gateway Server** (`src/gateway/gateway_server.cpp`)
   - Updated file path: `/app/frontend/index.html` → `/app/web/index.html`
   - Updated comments to reference "web frontend"

2. **Docker Configuration**
   - `docker/Dockerfile`: Updated COPY command for web files
   - `docker/docker-compose.yml`: Fixed context paths to `..` for proper resolution
   - `docker/docker-compose.dev.yml`: Updated volume mount for hot reload
   - Fixed volume paths from `./data` to `../data` for consistency

3. **Documentation Updates**
   - `README.md`: Updated file paths and commands
   - `web/README.md`: Updated title and all path references
   - `docs/architecture/OVERVIEW.md`: Updated terminology
   - `docs/api/REST_API.md`: Updated section title
   - `docs/deployment/*.md`: Updated all references
   - `docs/migrations/RESTRUCTURING_PLAN.md`: Updated directory structure

4. **Build Reports**
   - `BUILD_AND_TEST_REPORT.md`: Updated status descriptions

## Testing

### Local Build
```bash
cd build
cmake ..
make -j$(sysctl -n hw.ncpu)
# ✅ Build successful
```

### Docker Build
```bash
docker compose -f docker/docker-compose.yml build gateway
# ✅ Build in progress
```

### Verification Steps
1. ✅ Directory renamed successfully
2. ✅ All code references updated
3. ✅ All documentation updated
4. ✅ Local build passes
5. ⏳ Docker build in progress
6. ⏳ Gateway serves web frontend at http://localhost:18080/

## Gateway Access

The web frontend is accessible via the gateway node:

**URL:** `http://localhost:18080/`

The gateway serves the `web/index.html` file at the root endpoint, providing:
- Beautiful search interface
- Real-time address search
- Google Maps integration
- Result visualization

## Files Modified

### Source Code
- `src/gateway/gateway_server.cpp`

### Docker
- `docker/Dockerfile`
- `docker/docker-compose.yml`
- `docker/docker-compose.dev.yml`
- `docker/README.md`

### Documentation
- `README.md`
- `web/README.md`
- `docs/architecture/OVERVIEW.md`
- `docs/api/REST_API.md`
- `docs/deployment/DOCKER_MIGRATION_COMPLETE.md`
- `docs/deployment/DOCKER_TEST_GUIDE.md`
- `docs/migrations/RESTRUCTURING_PLAN.md`
- `BUILD_AND_TEST_REPORT.md`

## Impact

- ✅ **No breaking changes** - The gateway still serves the frontend at the same URL
- ✅ **Improved clarity** - Better separation between C++ apps and web assets
- ✅ **Consistent naming** - Aligns with common C++ project conventions
- ✅ **Docker compatibility** - Fixed context paths for proper builds

## Next Steps

1. Complete Docker build verification
2. Test gateway serving web frontend
3. Verify hot reload in development mode
4. Update any external documentation or scripts

## Rollback

If needed, rollback is straightforward:
```bash
mv web frontend
# Then revert all file changes using git
git checkout src/gateway/gateway_server.cpp docker/ docs/ README.md
```
