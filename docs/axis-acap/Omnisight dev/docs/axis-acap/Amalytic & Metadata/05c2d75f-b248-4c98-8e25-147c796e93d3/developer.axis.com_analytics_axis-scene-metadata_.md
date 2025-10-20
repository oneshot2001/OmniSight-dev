---
url: "https://developer.axis.com/analytics/axis-scene-metadata/"
title: "AXIS Scene Metadata | Axis developer documentation"
---

[Skip to main content](https://developer.axis.com/analytics/axis-scene-metadata/#__docusaurus_skipToContent_fallback)

[![Axis Communications Logo](https://developer.axis.com/img/axis-logo.svg)\\
**Axis developer documentation**](https://developer.axis.com/)

```

```

[Blog](https://developer.axis.com/blog/)

- [Metadata & Analytics](https://developer.axis.com/analytics/)
- [AXIS Object Analytics](https://developer.axis.com/analytics/axis-scene-metadata/#)

- [AXIS Scene Metadata](https://developer.axis.com/analytics/axis-scene-metadata/)

  - [Getting started](https://developer.axis.com/analytics/axis-scene-metadata/getting-started/)
  - [How-to guides](https://developer.axis.com/analytics/axis-scene-metadata/#)

  - [Reference](https://developer.axis.com/analytics/axis-scene-metadata/#)
- [Cloud Integrations](https://developer.axis.com/analytics/axis-scene-metadata/#)

- [Radar](https://developer.axis.com/analytics/axis-scene-metadata/#)


- [Home page](https://developer.axis.com/)
- AXIS Scene Metadata

# AXIS Scene Metadata

Axis products produce scene metadata — data about what the device sees. This documentation describes how to consume, interface with, and make use of this data when building applications. The provided data can be used in various applications for real-time situational awareness, efficient data searches, and identifying trends and patterns.

Axis devices such as cameras and radars track and collect information about detected objects, including humans, vehicles, and unclassified movement. Data is available under multiple topics to address different use cases. The table below provides an overview of the topics and their properties:

| Topic | Summary | Availability | Source |
| --- | --- | --- | --- |
| Analytics Scene Description | Real-time tracking of objects | [RTSP](https://developer.axis.com/analytics/axis-scene-metadata/reference/protocols/rtsp/), [ACAP SDK](https://developer.axis.com/analytics/axis-scene-metadata/reference/protocols/message-broker/), [MQTT](https://developer.axis.com/analytics/axis-scene-metadata/reference/protocols/mqtt/) ​ | [Fusion Tracker](https://developer.axis.com/analytics/axis-scene-metadata/reference/modules/fusion-tracker/) |
| Consolidated Track | Information about each object is summarized and delivered once the object leaves the scene. | [ACAP SDK](https://developer.axis.com/analytics/axis-scene-metadata/reference/protocols/message-broker/), [MQTT](https://developer.axis.com/analytics/axis-scene-metadata/reference/protocols/mqtt/) | [Track Consolidation](https://developer.axis.com/analytics/axis-scene-metadata/reference/modules/track-consolidation/) |

All topics are encoded in JSON following [Analytics Data Format](https://developer.axis.com/analytics/axis-scene-metadata/reference/data-formats/analytics-data-format/). Most data is also translated into [ONVIF conformant](https://www.onvif.org/profiles/specifications/) XML over RTSP.

AXIS Scene Metadata is supported across various devices, including products with multiple sensor types. For instance, multi-sensor devices like the [AXIS Q1686-DLE](https://www.axis.com/products/axis-q1686-dle) radar-video device combine sensor outputs to deliver a coherent and comprehensive scene understanding.

For a complete list of supported products, visit the [compatible products](https://www.axis.com/products/axis-scene-metadata#compatible-products) page. Click **View More** to see the full list.

To get started, see the [Getting Started guide](https://developer.axis.com/analytics/axis-scene-metadata/getting-started/).

To read more about central concepts to AXIS Scene Metadata, such as [object tracking](https://developer.axis.com/analytics/axis-scene-metadata/reference/concepts/object-tracking/), [processing modules](https://developer.axis.com/analytics/axis-scene-metadata/reference/concepts/processing-modules/), [data framing](https://developer.axis.com/analytics/axis-scene-metadata/reference/concepts/data-framing/), etc., see the [concepts section](https://developer.axis.com/analytics/axis-scene-metadata/reference/concepts/).

info

This documentation describe the AXIS Scene Metadata functionality as of the latest version of the [active track](https://help.axis.com/axis-os#active-track) for AXIS OS.

![metadata-homepage-stock-image](https://developer.axis.com/assets/images/index.metadata-homepage-stock-image-560065cef9d92f3d8c9ec18fb075049f.webp)

Last updated on **Sep 4, 2025**

[Previous\\
\\
AXIS Object Analytics counting data](https://developer.axis.com/analytics/axis-object-analytics/how-to-guides/axis-object-analytics-counting-data/) [Next\\
\\
Getting started](https://developer.axis.com/analytics/axis-scene-metadata/getting-started/)

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