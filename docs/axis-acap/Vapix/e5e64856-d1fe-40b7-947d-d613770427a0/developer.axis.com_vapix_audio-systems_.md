---
url: "https://developer.axis.com/vapix/audio-systems/"
title: "Audio systems | Axis developer documentation"
---

[Skip to main content](https://developer.axis.com/vapix/audio-systems/#__docusaurus_skipToContent_fallback)

[![Axis Communications Logo](https://developer.axis.com/img/axis-logo.svg)\\
**Axis developer documentation**](https://developer.axis.com/)

```

```

[Blog](https://developer.axis.com/blog/)

- [VAPIX®](https://developer.axis.com/vapix/)
- [Applications](https://developer.axis.com/vapix/applications/)

- [Audio systems](https://developer.axis.com/vapix/audio-systems/)

  - [Audio Analytics](https://developer.axis.com/vapix/audio-systems/audio-analytics/)
  - [Audio API](https://developer.axis.com/vapix/audio-systems/audio-api/)
  - [Audio control service API](https://developer.axis.com/vapix/audio-systems/audio-control-service-api/)
  - [Audio Device Control](https://developer.axis.com/vapix/audio-systems/audio-device-control/)
  - [Audio mixer API](https://developer.axis.com/vapix/audio-systems/audio-mixer-api/)
  - [Audio Multicast Controller](https://developer.axis.com/vapix/audio-systems/audio-multicast-controller/)
  - [Audio relay service API](https://developer.axis.com/vapix/audio-systems/audio-relay-service-api/)
  - [Auto speaker test service API](https://developer.axis.com/vapix/audio-systems/auto-speaker-test-service-api/)
  - [AXIS Audio Manager Edge API](https://developer.axis.com/vapix/audio-systems/axis-audio-manager-edge-api/)
  - [AXIS Audio Manager Pro API](https://developer.axis.com/vapix/audio-systems/axis-audio-manager-pro-api/)
  - [Media clip API](https://developer.axis.com/vapix/audio-systems/media-clip-api/)
- [Body worn systems](https://developer.axis.com/vapix/body-worn-systems/)
- [Device configuration](https://developer.axis.com/vapix/device-configuration/)

- [Intercom](https://developer.axis.com/vapix/intercom/)

- [Network video](https://developer.axis.com/vapix/network-video/)

- [Physical access control](https://developer.axis.com/vapix/physical-access-control/)

- [Radar](https://developer.axis.com/vapix/radar/)


- [Home page](https://developer.axis.com/)
- Audio systems

On this page

# Audio systems

VAPIX® Audio system APIs are a set of application programming interfaces (APIs) for Axis audio system products. The APIs make it easy to integrate Axis network speakers and other audio products in third-party systems.

## Version history [​](https://developer.axis.com/vapix/audio-systems/\#version-history "Direct link to Version history")

| Date | Updates |
| --- | --- |
| 2025-09-18 | Moved [Call service API](https://developer.axis.com/vapix/intercom/call-service-api/) to the Intercom section. |
| 2024–11–04 | [AXIS Audio Manager Edge API](https://developer.axis.com/vapix/audio-systems/axis-audio-manager-edge-api/): New API |
| 2024–07–02 | [AXIS Audio Manager Pro API](https://developer.axis.com/vapix/audio-systems/axis-audio-manager-pro-api/): Updated to API version 1.1. Added new methods, parameters and examples. |
| 2024–05–13 | [Audio Analytics](https://developer.axis.com/vapix/audio-systems/audio-analytics/): New API [Audio Device Control](https://developer.axis.com/vapix/audio-systems/audio-device-control/): New API |
| 2024–03–06 | [Audio API](https://developer.axis.com/vapix/audio-systems/audio-api/): Added arguments `audiobitrate`, `audiosamplerate` and `audiocodec` [Audio Multicast Controller](https://developer.axis.com/vapix/audio-systems/audio-multicast-controller/): New event |
| 2024–01–30 | [Audio Multicast Controller](https://developer.axis.com/vapix/audio-systems/audio-multicast-controller/): New API |
| 2023–11–03 | [Media clip API](https://developer.axis.com/vapix/audio-systems/media-clip-api/): Deprecated the `audiooutput` parameter and added the `audiodeviceid` and `audiooutputid` parameters. |
| 2023–06–13 | [AXIS Audio Manager Pro API](https://developer.axis.com/vapix/audio-systems/axis-audio-manager-pro-api/): New API |
| 2022–12–05 | [Auto speaker test service API](https://developer.axis.com/vapix/audio-systems/auto-speaker-test-service-api/): Added template on how to perform a speaker test |
| 2022–06–23 | [Media clip API](https://developer.axis.com/vapix/audio-systems/media-clip-api/): Deprecated parameters, minor updates [Audio relay service API](https://developer.axis.com/vapix/audio-systems/audio-relay-service-api/): Deprecated parameters [Audio control service API](https://developer.axis.com/vapix/audio-systems/audio-control-service-api/): Deprecated parameters |
| 2021–12–10 | [Audio mixer API](https://developer.axis.com/vapix/audio-systems/audio-mixer-api/): New API |
| 2021–07–19 | [Media clip API](https://developer.axis.com/vapix/audio-systems/media-clip-api/): Minor updates |
| 2021–04–29 | [Audio API](https://developer.axis.com/vapix/audio-systems/audio-api/): Minor updates [Media clip API](https://developer.axis.com/vapix/audio-systems/media-clip-api/): Minor updates |
| 2020–08–05 | [Media clip API](https://developer.axis.com/vapix/audio-systems/media-clip-api/): Minor updates |
| 2020–06–11 | [Media clip API](https://developer.axis.com/vapix/audio-systems/media-clip-api/): Minor updates |
| 2019–10–02 | [Call service API](https://developer.axis.com/vapix/intercom/call-service-api/): Minor updates. |
| 2018–09–04 | Audio API update: Added new Audio compression formats & Audio source parameters. |
| 2018–04–26 | Audio API update: Updated the information in the Transmit audio data-section.Audio Control Service API, Media Clip API update: Harmonized content across sections. |
| 2018–02–20 | Updated Curl command and a new Example added to the Audio Control Service API [Adjust InputConfiguration](https://developer.axis.com/vapix/audio-systems/audio-control-service-api/#adjust-inputconfiguration). |
| 2018–01–22 | Harmonized content across sections. |
| 2017–10–20 | Audio Relay Service API update: Added new Examples and Specifications |
| 2017-05-04 | Media Clip API update: Added stopclip.cgi and support for MP3.Call Service API update: Added support for IPv6.Audio Relay Service API update: Added support for multicast group speakers. |
| 2017–03–09 | VAPIX® Audio Systems released.Sections Audio Control Service API, Audio Relay Service API, and Auto Speaker Test Service API moved from VAPIX® Network Video. Sections Audio API, Media Clip API, and Call Service API are shared with VAPIX® Network Video. |

Last updated on **Sep 19, 2025**

[Previous\\
\\
Video motion detection 4 API](https://developer.axis.com/vapix/applications/video-motion-detection-4-api/) [Next\\
\\
Audio Analytics](https://developer.axis.com/vapix/audio-systems/audio-analytics/)

- [Version history](https://developer.axis.com/vapix/audio-systems/#version-history)

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