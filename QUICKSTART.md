# OMNISIGHT Quick Start Guide

Get up and running with OMNISIGHT development in 5 minutes.

## Prerequisites Check
```bash
docker --version        # Docker 20.0+
docker-compose --version # Docker Compose 1.29+
make --version          # GNU Make 4.0+
git --version           # Git 2.30+
```

## 1. Initial Setup (First Time Only)

```bash
# Clone repository
git clone https://github.com/oneshot2001/OmniSight-dev.git
cd OmniSight-dev

# Configure environment
cp .env.example .env
# Edit .env with your camera IP and credentials

# Build development environment (takes 5-10 minutes)
make docker-build
```

## 2. Start Development

```bash
# Start all services
make dev

# Open development shell
make shell
```

You're now inside the ACAP SDK development container!

## 3. Build OMNISIGHT

```bash
# Inside the container or from host:
make build
```

## 4. Run Tests

```bash
# Run all tests
make test

# Run specific component tests
make test-perception
make test-timeline
make test-swarm
```

## 5. Deploy to Camera

```bash
# Package the application
make package

# Deploy to camera (edit CAMERA_IP in .env first)
make deploy

# Or specify camera inline
make deploy CAMERA_IP=192.168.1.100 CAMERA_USER=root CAMERA_PASS=yourpass
```

## 6. Monitor Application

```bash
# View application logs
make logs

# Check status
make status

# Access web interface
open https://192.168.1.100/local/omnisight/
```

## Common Commands Cheat Sheet

| Command | Description |
|---------|-------------|
| `make help` | Show all available commands |
| `make dev` | Start development environment |
| `make shell` | Open shell in dev container |
| `make build` | Build the application |
| `make package` | Create .eap package |
| `make deploy` | Build + package + deploy to camera |
| `make clean` | Clean build artifacts |
| `make test` | Run all tests |
| `make logs` | View application logs from camera |
| `make status` | Check app status on camera |
| `make info` | Display build information |

## Project Structure

```
omnisight/
├── src/                    # Your code goes here
│   ├── core/              # Application framework
│   ├── perception/        # Vision & object detection
│   ├── timeline/          # Timeline Threading™
│   ├── swarm/             # Swarm Intelligence
│   ├── nlp/               # Natural Language
│   └── platform/          # ACAP integration
├── models/                # ML models (TFLite)
├── tests/                 # Test files
├── Dockerfile             # Dev container config
├── Makefile               # Build commands
├── manifest.json          # ACAP app manifest
└── docker-compose.yml     # Services config
```

## Development Workflow

1. **Make code changes** in `src/` directory
2. **Build**: `make build`
3. **Test**: `make test`
4. **Deploy**: `make deploy`
5. **Monitor**: `make logs`
6. **Iterate**: Repeat!

## Troubleshooting

### Docker Issues
```bash
# Rebuild clean
make clean-all
make docker-build

# Check Docker is running
docker ps
```

### Build Fails
```bash
# Enter container and debug
make shell
cd /opt/app/build
cmake .. -DCMAKE_VERBOSE_MAKEFILE=ON
make
```

### Cannot Connect to Camera
```bash
# Test connection
ping 192.168.1.100
curl -k https://192.168.1.100

# Verify credentials in .env
cat .env | grep CAMERA
```

### Need Help?
```bash
# Show all make targets with descriptions
make help

# View detailed documentation
cat DEVELOPMENT.md
```

## Next Steps

Ready to start coding? Check out:
- [DEVELOPMENT.md](DEVELOPMENT.md) - Full development guide
- [README.md](README.md) - Project overview
- [PRD](docs/axis-acap/Markdown%20versions/omnisight-prd.md) - Product requirements
- [Axis ACAP Docs](docs/axis-acap/) - Platform documentation

## Week 1 Goals

- [ ] Implement basic perception engine
- [ ] Set up VDO stream capture
- [ ] Integrate object detection model
- [ ] Create unit tests for perception pipeline

Let's build the future! 🚀
