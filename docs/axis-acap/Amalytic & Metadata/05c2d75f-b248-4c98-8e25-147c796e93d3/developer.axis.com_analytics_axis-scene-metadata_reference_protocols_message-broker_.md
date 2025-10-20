---
url: "https://developer.axis.com/analytics/axis-scene-metadata/reference/protocols/message-broker/"
title: "Message broker | Axis developer documentation"
---

[Skip to main content](https://developer.axis.com/analytics/axis-scene-metadata/reference/protocols/message-broker/#__docusaurus_skipToContent_fallback)

[![Axis Communications Logo](https://developer.axis.com/img/axis-logo.svg)\\
**Axis developer documentation**](https://developer.axis.com/)

```

```

[Blog](https://developer.axis.com/blog/)

- [Metadata & Analytics](https://developer.axis.com/analytics/)
- [AXIS Object Analytics](https://developer.axis.com/analytics/axis-scene-metadata/reference/protocols/message-broker/#)

- [AXIS Scene Metadata](https://developer.axis.com/analytics/axis-scene-metadata/)

  - [Getting started](https://developer.axis.com/analytics/axis-scene-metadata/getting-started/)
  - [How-to guides](https://developer.axis.com/analytics/axis-scene-metadata/reference/protocols/message-broker/#)

  - [Reference](https://developer.axis.com/analytics/axis-scene-metadata/reference/protocols/message-broker/#)

    - [Concepts](https://developer.axis.com/analytics/axis-scene-metadata/reference/concepts/)

    - [Definitions](https://developer.axis.com/analytics/axis-scene-metadata/reference/protocols/message-broker/#)

    - [Modules](https://developer.axis.com/analytics/axis-scene-metadata/reference/modules/)

    - [Data Formats](https://developer.axis.com/analytics/axis-scene-metadata/reference/protocols/message-broker/#)

    - [Protocols](https://developer.axis.com/analytics/axis-scene-metadata/reference/protocols/message-broker/#)

      - [RTSP](https://developer.axis.com/analytics/axis-scene-metadata/reference/protocols/rtsp/)
      - [MQTT](https://developer.axis.com/analytics/axis-scene-metadata/reference/protocols/mqtt/)
      - [Message broker](https://developer.axis.com/analytics/axis-scene-metadata/reference/protocols/message-broker/)
    - [Related](https://developer.axis.com/analytics/axis-scene-metadata/reference/protocols/message-broker/#)
- [Cloud Integrations](https://developer.axis.com/analytics/axis-scene-metadata/reference/protocols/message-broker/#)

- [Radar](https://developer.axis.com/analytics/axis-scene-metadata/reference/protocols/message-broker/#)


- [Home page](https://developer.axis.com/)
- [AXIS Scene Metadata](https://developer.axis.com/analytics/axis-scene-metadata/)
- Reference
- Protocols
- Message broker

On this page

# Message broker

warning

This feature is to be deprecated and replaced by new functionality.

The Message Broker is an internal component (via the [ACAP Native SDK](https://developer.axis.com/acap/api/)) that enables efficient communication between applications on the same device. It uses a publish/subscribe pattern to eliminate direct coupling between different processes, making it easier to exchange data without introducing build-time dependencies. Although it may soon be deprecated, it remains a valuable tool for applications built on Axis devices.

## Core concepts [​](https://developer.axis.com/analytics/axis-scene-metadata/reference/protocols/message-broker/\#core-concepts "Direct link to Core concepts")

- **Producers**: Applications that create one or more “channels” to publish messages.
- **Subscribers**: Applications that listen for messages on specified topics and sources.
- **Topics**: Textual identifiers that group messages (e.g., “com.axis.analytics\_scene\_description.v0.beta”).
- **Sources**: Subdivisions of a topic that help organize messages (for example, each camera's video stream can be a different source).

## How it works [​](https://developer.axis.com/analytics/axis-scene-metadata/reference/protocols/message-broker/\#how-it-works "Direct link to How it works")

1. **Channel Creation**: A producer defines a channel by specifying a topic and an optional source. The combination of topic + source uniquely identifies the channel.
2. **Message Publication**: Producers publish data (payload) to the channel.
3. **Subscription**: A subscriber “listens” to a topic (and optionally a source) and receives messages published on matching channels.
4. **On-Demand Production**: Producers can enable or disable message generation based on whether consumers are actively subscribed, optimizing resource usage.

## Interacting via libmdb [​](https://developer.axis.com/analytics/axis-scene-metadata/reference/protocols/message-broker/\#interacting-via-libmdb "Direct link to Interacting via libmdb")

Applications communicate with the Message Broker through a C API called **libmdb**. This library provides functions for creating channels, publishing messages, subscribing to topics and sources, and managing message flows.

For more information, see [Message Broker documentation](https://developer.axis.com/acap/api/src/api/message-broker/html/index.html).

## Available topic [​](https://developer.axis.com/analytics/axis-scene-metadata/reference/protocols/message-broker/\#available-topic "Direct link to Available topic")

- **`com.axis.analytics_scene_description.v0.beta`**: Provides data produced by the [Fusion Tracker](https://developer.axis.com/analytics/axis-scene-metadata/reference/modules/fusion-tracker/) module.
- **`com.axis.consolidated_track.v1.beta`**: Provides data produced by the [Track Consolidation](https://developer.axis.com/analytics/axis-scene-metadata/reference/modules/fusion-tracker/) module.

_Remember to also set the `source` parameter in addition to `topic` when subscribing!_

Last updated on **Oct 6, 2025**

[Previous\\
\\
MQTT](https://developer.axis.com/analytics/axis-scene-metadata/reference/protocols/mqtt/) [Next\\
\\
Feature flags](https://developer.axis.com/analytics/axis-scene-metadata/reference/related/feature-flags/)

- [Core concepts](https://developer.axis.com/analytics/axis-scene-metadata/reference/protocols/message-broker/#core-concepts)
- [How it works](https://developer.axis.com/analytics/axis-scene-metadata/reference/protocols/message-broker/#how-it-works)
- [Interacting via libmdb](https://developer.axis.com/analytics/axis-scene-metadata/reference/protocols/message-broker/#interacting-via-libmdb)
- [Available topic](https://developer.axis.com/analytics/axis-scene-metadata/reference/protocols/message-broker/#available-topic)

COMMUNITY

- [Axis Developer Community](https://axis.com/developer-community)
- [Axis on GitHub](https://github.com/AxisCommunications)

SOCIAL

- [LinkedIn](https://www.linkedin.com/company/axis-communications)
- [YouTube](https://www.youtube.com/@AxisCommunications)

LEGAL AND COMPLIANCE

- [Legal](https://www.axis.com/legal)
- [Privacy](https://www.axis.com/privacy)

© 2025 Axis Communications AB. All rights reserved.