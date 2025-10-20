# OMNISIGHT Documentation

This directory contains all technical documentation for the OMNISIGHT project development on the Axis ACAP platform.

## Directory Structure

### `/axis-acap/`
Complete Axis development documentation organized by topic:

#### Core Platform Documentation
- **ACAP/** - Core ACAP platform documentation
- **ACAP version 12/** - ACAP version 12 specific docs, API references, compatibility guides
- **Documentation/** - General Axis development guides and knowledge base articles

#### API & Integration
- **Vapix/** - VAPIX API documentation (device configuration, network video, applications)
- **AOA/** - Axis Object Analytics documentation
- **Amalytic & Metadata/** - Scene metadata and analytics data formats

#### Specialized Topics
- **Vision/** - Computer vision on-device, DLPU documentation, model optimization
- **Video Streaming and Recording/** - Video protocols, Zipstream, signed video
- **Scene Metadata/** - Scene metadata integration guides
- **Guidlines/** - Development and testing guidelines

#### Reference Materials
- **Axis White Papers/** - Technical white papers
- **Axis Files/** - Additional Axis resources
- **Research/** - Industry research and pain points
- **Markdown versions/** - PRD and strategy documents (omnisight-prd.md, neuraxis-prd.md, etc.)

## Key Documents for OMNISIGHT Development

### Must-Read for Getting Started
1. [ACAP version 12 Main](axis-acap/ACAP%20version%2012/) - Start here for ACAP 4.0+ development
2. [Computer Vision on Device](axis-acap/Vision/) - DLPU, model optimization, TensorFlow Lite
3. [Scene Metadata Integration](axis-acap/Scene%20Metadata/) - For swarm intelligence and data sharing
4. [VAPIX API](axis-acap/Vapix/) - Device control and configuration APIs

### Critical for OMNISIGHT Features
1. **Timeline Threading™ (Precognitive Engine)**
   - Vision/DLPU documentation for transformer models
   - Computer vision optimization guides

2. **Swarm Intelligence**
   - Amalytic & Metadata/Message Broker (MQTT)
   - Scene Metadata protocols

3. **Natural Language Interface**
   - ACAP API documentation
   - Integration patterns

4. **Performance Requirements**
   - Vision/optimization-tips
   - Vision/quantization guides
   - Recommended model architectures

## Product Requirements
- **omnisight-prd.md** - Main product requirements document (located in Markdown versions/)
- **neuraxis-prd.md** - Alternative PRD version
- **sentinel-core-strategy.md** - Core strategy document

## Usage Notes
- All documents are copied from the source iCloud Drive location
- Updated: October 19, 2025
- Organized for quick reference during development

## Next Steps
1. Review the OMNISIGHT PRD in Markdown versions/
2. Study ACAP version 12 documentation for platform capabilities
3. Deep dive into Vision documentation for ML model deployment
4. Review Scene Metadata for swarm intelligence implementation
