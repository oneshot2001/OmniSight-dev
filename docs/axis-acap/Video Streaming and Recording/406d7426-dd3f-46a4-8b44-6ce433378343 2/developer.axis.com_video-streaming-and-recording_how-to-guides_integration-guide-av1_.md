---
url: "https://developer.axis.com/video-streaming-and-recording/how-to-guides/integration-guide-av1/"
title: "AV1™ integration | Axis developer documentation"
---

[Skip to main content](https://developer.axis.com/video-streaming-and-recording/how-to-guides/integration-guide-av1/#__docusaurus_skipToContent_fallback)

[![Axis Communications Logo](https://developer.axis.com/img/axis-logo.svg)\\
**Axis developer documentation**](https://developer.axis.com/)

```

```

[Blog](https://developer.axis.com/blog/)

- [Video streaming & recording](https://developer.axis.com/video-streaming-and-recording/)
- [Zipstream](https://developer.axis.com/video-streaming-and-recording/zipstream/)
- [Signed video](https://developer.axis.com/video-streaming-and-recording/signed-video/)
- [How-to guides](https://developer.axis.com/video-streaming-and-recording/how-to-guides/integration-guide-av1/#)

  - [AV1™ integration](https://developer.axis.com/video-streaming-and-recording/how-to-guides/integration-guide-av1/)

- [Home page](https://developer.axis.com/)
- How-to guides
- AV1™ integration

On this page

# AV1™ integration

## How to integrate AV1 video with Axis ARTPEC-9 devices [​](https://developer.axis.com/video-streaming-and-recording/how-to-guides/integration-guide-av1/\#how-to-integrate-av1-video-with-axis-artpec-9-devices "Direct link to How to integrate AV1 video with Axis ARTPEC-9 devices")

Axis supports the AV1 video encoding standard on the [ARTPEC-9 system-on-chip (SoC)](https://www.axis.com/solutions/system-on-chip), and this addition promises easier video integration, new features, improved compression efficiency, and reduced bandwidth requirements.

This document outlines our approach to implementing AV1 video in our devices and solutions. It also offers guidance on how you can integrate AV1 encoded video from our devices into your integrations and solutions.

## AV1 video implementation in Axis products [​](https://developer.axis.com/video-streaming-and-recording/how-to-guides/integration-guide-av1/\#av1-video-implementation-in-axis-products "Direct link to AV1 video implementation in Axis products")

AV1 video is available only on Axis devices that have ARTPEC-9 or later SoC. Implementation of [AV1 video in our ARTPEC-9 Axis devices](https://www.axis.com/solutions/av1-codec) is compliant with [Alliance for Open Media (AOMedia) AV1 specifications](https://aomedia.org/specifications/av1).

The implementation will support:

- **Main Profile:** Ensures compatibility with a wide range of platforms.
- **8-bit color depth:** Allows flexible configuration to suit various use cases.

## How to set up integration with Axis AV1-enabled devices [​](https://developer.axis.com/video-streaming-and-recording/how-to-guides/integration-guide-av1/\#how-to-set-up-integration-with-axis-av1-enabled-devices "Direct link to How to set up integration with Axis AV1-enabled devices")

- **Familiarize yourself with the AXIS OS APIs (VAPIX):** [VAPIX](https://developer.axis.com/vapix/) APIs provide the necessary tools and documentation for customized integration with [AXIS OS](https://www.axis.com/support/axis-os).

- **Understand AV1 configuration settings:** Learn about configuring how to configure AV1-related parameters, such as bitrate, resolution, and frame rate, using [VAPIX](https://developer.axis.com/vapix/) APIs.

- **Prepare to retrieve AV1 video streams:** You need to retrieve AV1-encoded video streams from Axis devices using standard protocols such as RTSP/RTP or HTTP. For example, you can use the following URL to retrieve an AV1 stream:


`http://<servername>/axis-cgi/media.cgi?videocodec=av1&container=mp4`.

Axis devices will support AV1 video over WebRTC alongside [Axis Cloud Connect](https://www.axis.com/solutions/axis-cloud-connect) services. However, Axis Cloud Connect is currently in private preview and available only to selected partners. We are working on adding support for more partner integrations.

- **Decode and render AV1 video streams:** For example, you can utilize FFmpeg, a popular open-source multimedia framework, to decode AV1 video streams in your application. For more information, see [FFmpeg](https://ffmpeg.org/).

Most modern browsers, media players, video hardware, and many public frameworks support AV1 video rendering.

## Axis integration of AV1 video technology [​](https://developer.axis.com/video-streaming-and-recording/how-to-guides/integration-guide-av1/\#axis-integration-of-av1-video-technology "Direct link to Axis integration of AV1 video technology")

_The following information shows how we implemented AV1 encoded video in our video solutions._

- We added support for streaming AV1 video over RTSP/RTP in Axis streaming library implementation, written in C# and using VAPIX to communicate with Axis devices.

- For decoding AV1 video, we utilize FFmpeg with the [dav1d decoder library](https://github.com/videolan/dav1d) enabled. For mobile apps built for iOS and Android platforms, we use FFmpeg to retrieve frames and allow the platform to perform decoding and video players handle the rendering of frames.

- We're leveraging the built-in AV1 video support in web browsers for our web client. Due to Google's involvement in the Alliance for Open Media, many browsers already offer excellent AV1 video compatibility.


## Key considerations for future integration [​](https://developer.axis.com/video-streaming-and-recording/how-to-guides/integration-guide-av1/\#key-considerations-for-future-integration "Direct link to Key considerations for future integration")

Consider the following when you prepare for integration with AV1 video-enabled Axis devices:

- **Compatibility:** Ensure that your system's hardware and software can support AV1 video decoding. Remember that AV1 video may require more CPU resources than other codecs.
- **Performance optimization:** Optimize your application's performance by leveraging multi-threading, GPU acceleration, or other techniques.
- **Licensing:** Familiarize yourself with the licensing terms and conditions associated with using AV1 video technology. It is your sole responsibility to obtain any necessary licenses from third parties for any patents or other intellectual property that may be required for the use of AV1 video technology as described in this document.

By understanding Axis' approach to AV1 video implementation and planning for integration, you can ensure a seamless transition to AV1 video-enabled Axis devices and of the benefits of this advanced video codec.

Be advised that this document is provided “as is” without warranty of any kind and for information purposes only.

_AV1™ is a trademark of the Alliance for Open Media, a Joint Development Foundation project/The Linux Foundation._

Last updated on **Aug 28, 2025**

[Previous\\
\\
Signed video](https://developer.axis.com/video-streaming-and-recording/signed-video/)

- [How to integrate AV1 video with Axis ARTPEC-9 devices](https://developer.axis.com/video-streaming-and-recording/how-to-guides/integration-guide-av1/#how-to-integrate-av1-video-with-axis-artpec-9-devices)
- [AV1 video implementation in Axis products](https://developer.axis.com/video-streaming-and-recording/how-to-guides/integration-guide-av1/#av1-video-implementation-in-axis-products)
- [How to set up integration with Axis AV1-enabled devices](https://developer.axis.com/video-streaming-and-recording/how-to-guides/integration-guide-av1/#how-to-set-up-integration-with-axis-av1-enabled-devices)
- [Axis integration of AV1 video technology](https://developer.axis.com/video-streaming-and-recording/how-to-guides/integration-guide-av1/#axis-integration-of-av1-video-technology)
- [Key considerations for future integration](https://developer.axis.com/video-streaming-and-recording/how-to-guides/integration-guide-av1/#key-considerations-for-future-integration)

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