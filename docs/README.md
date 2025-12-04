# Geocoding System Documentation

Welcome to the Geocoding System documentation! This directory contains comprehensive documentation organized by category.

## 📚 Documentation Structure

```
docs/
├── architecture/     # System design and architecture
├── development/      # Development guides and standards
├── deployment/       # Deployment and operations
├── api/             # API documentation
└── migrations/      # Project restructuring history
```

## 🚀 Quick Start

New to the project? Start here:

1. **[Architecture Overview](architecture/OVERVIEW.md)** - Understand the system design
2. **[REST API](api/REST_API.md)** - Learn the API endpoints
3. **[Docker Guide](deployment/DOCKER_BUILD_GUIDE.md)** - Deploy with Docker
4. **[Testing Guide](development/TESTING_GUIDE.md)** - Run and write tests

## 📖 Documentation by Category

### Architecture

System design, components, and data flow.

- **[Overview](architecture/OVERVIEW.md)** - High-level architecture and design principles
- **[Data Node Architecture](architecture/DATA_NODE.md)** - Data node internals *(coming soon)*
- **[Gateway Architecture](architecture/GATEWAY.md)** - Gateway node internals *(coming soon)*

### Development

Guides for developers working on the codebase.

- **[Style Guide](development/STYLE_GUIDE.md)** - Code style and formatting rules
- **[Quick Style Reference](development/QUICK_STYLE_REFERENCE.md)** - Quick style lookup
- **[Style Setup](development/STYLE_SETUP_SUMMARY.md)** - Setting up code formatting
- **[Testing Guide](development/TESTING_GUIDE.md)** - Writing and running tests
- **[Contributing](development/CONTRIBUTING.md)** - How to contribute *(coming soon)*

### Deployment

Deployment, operations, and infrastructure.

- **[Docker Build Guide](deployment/DOCKER_BUILD_GUIDE.md)** - Docker build process and optimization
- **[Docker Test Guide](deployment/DOCKER_TEST_GUIDE.md)** - Testing Docker deployments
- **[Docker Migration](deployment/DOCKER_MIGRATION_COMPLETE.md)** - Docker restructuring details
- **[Production Guide](deployment/PRODUCTION.md)** - Production deployment *(coming soon)*

### API

API reference and usage examples.

- **[REST API](api/REST_API.md)** - Complete REST API documentation
- **[gRPC API](api/GRPC_API.md)** - Internal gRPC API *(coming soon)*

### Migrations

Project restructuring and migration history.

- **[Apps Migration](migrations/MIGRATION_COMPLETE.md)** - Apps directory restructuring
- **[Restructuring Plan](migrations/RESTRUCTURING_PLAN.md)** - Overall restructuring plan

## 🎯 Common Tasks

### For Users

- **Search for addresses:** See [REST API](api/REST_API.md)
- **Deploy the system:** See [Docker Guide](deployment/DOCKER_BUILD_GUIDE.md)
- **Use the web interface:** Open http://localhost:18080/

### For Developers

- **Set up development environment:** See [Style Setup](development/STYLE_SETUP_SUMMARY.md)
- **Write tests:** See [Testing Guide](development/TESTING_GUIDE.md)
- **Follow code style:** See [Style Guide](development/STYLE_GUIDE.md)
- **Understand architecture:** See [Architecture Overview](architecture/OVERVIEW.md)

### For Operators

- **Deploy with Docker:** See [Docker Guide](deployment/DOCKER_BUILD_GUIDE.md)
- **Monitor health:** `curl http://localhost:18080/health`
- **View logs:** `docker-compose logs -f`
- **Troubleshoot:** See [Docker Test Guide](deployment/DOCKER_TEST_GUIDE.md)

## 🔍 Finding Documentation

### By Topic

- **Architecture & Design** → `architecture/`
- **Code Style & Standards** → `development/`
- **Docker & Deployment** → `deployment/`
- **API Reference** → `api/`

### By Role

- **New Developer** → Start with `architecture/OVERVIEW.md`
- **API Consumer** → Start with `api/REST_API.md`
- **DevOps Engineer** → Start with `deployment/DOCKER_BUILD_GUIDE.md`
- **Contributor** → Start with `development/STYLE_GUIDE.md`

## 📝 Documentation Standards

### Writing Guidelines

- Use clear, concise language
- Include code examples
- Add diagrams where helpful
- Keep docs up to date with code changes

### File Naming

- Use `SCREAMING_SNAKE_CASE.md` for documentation files
- Use descriptive names (e.g., `REST_API.md` not `api.md`)
- Group related docs in subdirectories

### Structure

Each document should include:
1. Title and introduction
2. Table of contents (for long docs)
3. Main content with examples
4. Related documentation links

## 🤝 Contributing to Documentation

Found an error or want to improve the docs?

1. Edit the relevant `.md` file
2. Follow the documentation standards above
3. Test any code examples
4. Submit your changes

## 📊 Documentation Status

| Category | Status | Coverage |
|----------|--------|----------|
| Architecture | 🟡 Partial | Core docs complete |
| Development | 🟢 Complete | All guides present |
| Deployment | 🟢 Complete | Docker fully documented |
| API | 🟡 Partial | REST complete, gRPC pending |
| Migrations | 🟢 Complete | All migrations documented |

Legend:
- 🟢 Complete - Comprehensive documentation
- 🟡 Partial - Core docs present, some gaps
- 🔴 Missing - Needs documentation

## 🔗 External Resources

- **Project Repository:** [GitHub](https://github.com/your-org/geocoding-system) *(update link)*
- **Issue Tracker:** [GitHub Issues](https://github.com/your-org/geocoding-system/issues) *(update link)*
- **C++ Style Guide:** [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html)
- **gRPC Documentation:** [grpc.io](https://grpc.io/)
- **Crow Framework:** [Crow Documentation](https://crowcpp.org/)

## 📮 Getting Help

- **Documentation Issues:** Open an issue or submit a PR
- **Usage Questions:** See [REST API](api/REST_API.md) or [Testing Guide](development/TESTING_GUIDE.md)
- **Bug Reports:** Check logs and see [Docker Test Guide](deployment/DOCKER_TEST_GUIDE.md)

---

**Last Updated:** December 2024
**Version:** 1.0.0
