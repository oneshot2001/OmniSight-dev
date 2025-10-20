---
url: "https://developer.axis.com/vapix/applications/"
title: "Applications | Axis developer documentation"
---

[Skip to main content](https://developer.axis.com/vapix/applications/#__docusaurus_skipToContent_fallback)

[![Axis Communications Logo](https://developer.axis.com/img/axis-logo.svg)\\
**Axis developer documentation**](https://developer.axis.com/)

```

```

[Blog](https://developer.axis.com/blog/)

- [VAPIX®](https://developer.axis.com/vapix/)
- [Applications](https://developer.axis.com/vapix/applications/)

  - [Application API](https://developer.axis.com/vapix/applications/application-api/)
  - [Application configuration API](https://developer.axis.com/vapix/applications/application-configuration-api/)
  - [AXIS Object analytics API](https://developer.axis.com/vapix/applications/axis-object-analytics-api/)
  - [Cross line detection 1.1 API](https://developer.axis.com/vapix/applications/cross-line-detection-1.1-api/)
  - [Demographic identifier API](https://developer.axis.com/vapix/applications/demographic-identifier-api/)
  - [Digital autotracking API](https://developer.axis.com/vapix/applications/digital-autotracking-api/)
  - [Fence guard](https://developer.axis.com/vapix/applications/fence-guard/)
  - [License plate verifier API](https://developer.axis.com/vapix/applications/license-plate-verifier-api/)
  - [Loitering guard](https://developer.axis.com/vapix/applications/loitering-guard/)
  - [Motion guard](https://developer.axis.com/vapix/applications/motion-guard/)
  - [P8815-2 3D people counter API](https://developer.axis.com/vapix/applications/p8815-2-3d-people-counter-api/)
  - [People counter API](https://developer.axis.com/vapix/applications/people-counter-api/)
  - [Queue monitor API](https://developer.axis.com/vapix/applications/queue-monitor-api/)
  - [Video motion detection 2.1 API](https://developer.axis.com/vapix/applications/video-motion-detection-2.1-api/)
  - [Video motion detection 3 API](https://developer.axis.com/vapix/applications/video-motion-detection-3-api/)
  - [Video motion detection 4 API](https://developer.axis.com/vapix/applications/video-motion-detection-4-api/)
- [Audio systems](https://developer.axis.com/vapix/audio-systems/)

- [Body worn systems](https://developer.axis.com/vapix/body-worn-systems/)
- [Device configuration](https://developer.axis.com/vapix/device-configuration/)

- [Intercom](https://developer.axis.com/vapix/intercom/)

- [Network video](https://developer.axis.com/vapix/network-video/)

- [Physical access control](https://developer.axis.com/vapix/physical-access-control/)

- [Radar](https://developer.axis.com/vapix/radar/)


- [Home page](https://developer.axis.com/)
- Applications

On this page

# Applications

This section describes how to upload and configure AXIS Camera Application Platform (ACAP) applications.

Use **Application API** upload, control and manage applications and license keys.

Use **AXIS Application Configuration API** to configure applications developed by Axis.

Read more about AXIS Camera Application Platform at [www.axis.com/products/acap](https://www.axis.com/products/acap).

## Version history [​](https://developer.axis.com/vapix/applications/\#version-history "Direct link to Version history")

| Date | Updates |
| --- | --- |
| 2024–08–23 | [License plate verifier API](https://developer.axis.com/vapix/applications/license-plate-verifier-api/): Major updates. |
| 2024–05–03 | [Application API](https://developer.axis.com/vapix/applications/application-api/): New error codes added to **Upload application**. |
| 2024–02–06 | [People counter API](https://developer.axis.com/vapix/applications/people-counter-api/): Updated request URL. |
| 2023–12–21 | [AXIS Object analytics API](https://developer.axis.com/vapix/applications/axis-object-analytics-api/): Added methods `getAccumulatedCounts`, `resetAccumulatedCounts`, `resetPassthrough` and `getOccupancy`. |
| 2023–06–01 | [Application API](https://developer.axis.com/vapix/applications/application-api/): `AllowRoot` and `AllowUnsigned` added. |
| 2023–01–10 | [P8815-2 3D people counter API](https://developer.axis.com/vapix/applications/p8815-2-3d-people-counter-api/): `/export_occupancy` added. |
| 2022–11–03 | [Application API](https://developer.axis.com/vapix/applications/application-api/): `config.cgi` added. |
| 2022–06–27 | [P8815-2 3D people counter API](https://developer.axis.com/vapix/applications/p8815-2-3d-people-counter-api/): adjustOccupancy and restartPassthroughCountdown added. |
| 2022–01–04 | [People counter API](https://developer.axis.com/vapix/applications/people-counter-api/): Merged product parameters to a single command and clarified the documentation. |
| 2021–08–30 | [P8815-2 3D people counter API](https://developer.axis.com/vapix/applications/p8815-2-3d-people-counter-api/): New API. |
| 2021–06–14 | [License plate verifier API](https://developer.axis.com/vapix/applications/license-plate-verifier-api/): New API. |
| 2021–06–03 | [Application API](https://developer.axis.com/vapix/applications/application-api/): Minor updates. |
| 2021–06–02 | [AXIS Object analytics API](https://developer.axis.com/vapix/applications/axis-object-analytics-api/): Transferred from Network video. |
| 2019–01–14 | [Occupancy data](https://developer.axis.com/vapix/applications/people-counter-api/#occupancy-data): Updated API requests. |
| 2018–07–20 | **Deprecated:** [Video motion detection 3 API](https://developer.axis.com/vapix/applications/video-motion-detection-3-api/) [Video motion detection 2.1 API](https://developer.axis.com/vapix/applications/video-motion-detection-2.1-api/) |
| 2018–06–01 | **Updates:** [People counter API](https://developer.axis.com/vapix/applications/people-counter-api/): New API, merged with People counting apps API |
| 2018–04–20 | Updates: [Fence guard](https://developer.axis.com/vapix/applications/fence-guard/): New firmware-version [Loitering guard](https://developer.axis.com/vapix/applications/loitering-guard/): New firmware-version [Motion guard](https://developer.axis.com/vapix/applications/motion-guard/): New firmware-version [Video motion detection 4 API](https://developer.axis.com/vapix/applications/video-motion-detection-4-api/): New firmware-version Application API: Updated license examples Demographic Identifier API: Updated Get Ended tracks People Counter: Added information for later firmware-versions. |
| 2018–02–27 | [Loitering guard](https://developer.axis.com/vapix/applications/loitering-guard/): New API. [Video motion detection 4 API](https://developer.axis.com/vapix/applications/video-motion-detection-4-api/): API updated |
| 2017–11–29 | Corrected the Event Stream URL for the AXIS Video Motion Detection 4 API. |
| 2017–09–14 | [Fence guard](https://developer.axis.com/vapix/applications/fence-guard/), and [Motion guard](https://developer.axis.com/vapix/applications/motion-guard/): New API:s. |
| 2017–09–01 | Clarified that users with operator rights can access restart and reboot services. [Restart service](https://developer.axis.com/vapix/applications/demographic-identifier-api/#restart-service) and [Reboot the camera](https://developer.axis.com/vapix/applications/demographic-identifier-api/#reboot-the-camera) |
| 2017-05-03 | , [P8815-2 3D people counter API](https://developer.axis.com/vapix/applications/p8815-2-3d-people-counter-api/), [Queue monitor API](https://developer.axis.com/vapix/applications/queue-monitor-api/), and [Demographic identifier API](https://developer.axis.com/vapix/applications/demographic-identifier-api/): New API:s. |
| 2017-03-28 | Section Applications moved from VAPIX® Network Video to VAPIX® Applications. |

Last updated on **Oct 15, 2025**

[Previous\\
\\
VAPIX®](https://developer.axis.com/vapix/) [Next\\
\\
Application API](https://developer.axis.com/vapix/applications/application-api/)

- [Version history](https://developer.axis.com/vapix/applications/#version-history)

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