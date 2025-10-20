---
url: "https://developer.axis.com/analytics/axis-scene-metadata/reference/protocols/rtsp/"
title: "RTSP | Axis developer documentation"
---

[Skip to main content](https://developer.axis.com/analytics/axis-scene-metadata/reference/protocols/rtsp/#__docusaurus_skipToContent_fallback)

[![Axis Communications Logo](https://developer.axis.com/img/axis-logo.svg)\\
**Axis developer documentation**](https://developer.axis.com/)

```

```

[Blog](https://developer.axis.com/blog/)

- [Metadata & Analytics](https://developer.axis.com/analytics/)
- [AXIS Object Analytics](https://developer.axis.com/analytics/axis-scene-metadata/reference/protocols/rtsp/#)

- [AXIS Scene Metadata](https://developer.axis.com/analytics/axis-scene-metadata/)

  - [Getting started](https://developer.axis.com/analytics/axis-scene-metadata/getting-started/)
  - [How-to guides](https://developer.axis.com/analytics/axis-scene-metadata/reference/protocols/rtsp/#)

  - [Reference](https://developer.axis.com/analytics/axis-scene-metadata/reference/protocols/rtsp/#)

    - [Concepts](https://developer.axis.com/analytics/axis-scene-metadata/reference/concepts/)

    - [Definitions](https://developer.axis.com/analytics/axis-scene-metadata/reference/protocols/rtsp/#)

    - [Modules](https://developer.axis.com/analytics/axis-scene-metadata/reference/modules/)

    - [Data Formats](https://developer.axis.com/analytics/axis-scene-metadata/reference/protocols/rtsp/#)

    - [Protocols](https://developer.axis.com/analytics/axis-scene-metadata/reference/protocols/rtsp/#)

      - [RTSP](https://developer.axis.com/analytics/axis-scene-metadata/reference/protocols/rtsp/)
      - [MQTT](https://developer.axis.com/analytics/axis-scene-metadata/reference/protocols/mqtt/)
      - [Message broker](https://developer.axis.com/analytics/axis-scene-metadata/reference/protocols/message-broker/)
    - [Related](https://developer.axis.com/analytics/axis-scene-metadata/reference/protocols/rtsp/#)
- [Cloud Integrations](https://developer.axis.com/analytics/axis-scene-metadata/reference/protocols/rtsp/#)

- [Radar](https://developer.axis.com/analytics/axis-scene-metadata/reference/protocols/rtsp/#)


- [Home page](https://developer.axis.com/)
- [AXIS Scene Metadata](https://developer.axis.com/analytics/axis-scene-metadata/)
- Reference
- Protocols
- RTSP

On this page

# RTSP

Axis devices deliver scene metadata via a RTSP stream using the [ONVIF Scene Description](https://developer.axis.com/analytics/axis-scene-metadata/reference/data-formats/onvif/) XML based data structure.
This RTSP endpoint is most often used by Video Management Systems (VMS) to obtain data just like a video stream.

## Accessing ONVIF scene description scene metadata data via RTSP [​](https://developer.axis.com/analytics/axis-scene-metadata/reference/protocols/rtsp/\#accessing-onvif-scene-description-scene-metadata-data-via-rtsp "Direct link to Accessing ONVIF scene description scene metadata data via RTSP")

To connect to to a RTSP stream that includes scene metadata from enabled analytics metadata producers the following RTSP URL can be used,

```codeBlockLines_e6Vv
rtsp://<device-ip>/axis-media/media.amp?camera=1&audio=0&video=0&analytics=polygon

```

For more information, see [Video Streaming over RTSP API](https://developer.axis.com/vapix/network-video/video-streaming/#video-streaming-over-rtsp).

To configure enabled analytics metadata producers the [Analytics Metadata Producer Configuration API](https://developer.axis.com/vapix/network-video/analytics-metadata-producer-configuration/) is used.

For a detailed guide on how to configure and start a RTSP stream with scene metadata, see this [how-to](https://developer.axis.com/analytics/axis-scene-metadata/how-to-guides/scene-metadata-over-rtsp/).

Last updated on **Aug 18, 2025**

[Previous\\
\\
ONVIF](https://developer.axis.com/analytics/axis-scene-metadata/reference/data-formats/onvif/) [Next\\
\\
MQTT](https://developer.axis.com/analytics/axis-scene-metadata/reference/protocols/mqtt/)

- [Accessing ONVIF scene description scene metadata data via RTSP](https://developer.axis.com/analytics/axis-scene-metadata/reference/protocols/rtsp/#accessing-onvif-scene-description-scene-metadata-data-via-rtsp)

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