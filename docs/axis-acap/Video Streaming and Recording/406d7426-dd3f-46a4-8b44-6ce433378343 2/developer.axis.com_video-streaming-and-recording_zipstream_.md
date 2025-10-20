---
url: "https://developer.axis.com/video-streaming-and-recording/zipstream/"
title: "Zipstream | Axis developer documentation"
---

[Skip to main content](https://developer.axis.com/video-streaming-and-recording/zipstream/#__docusaurus_skipToContent_fallback)

[![Axis Communications Logo](https://developer.axis.com/img/axis-logo.svg)\\
**Axis developer documentation**](https://developer.axis.com/)

```

```

[Blog](https://developer.axis.com/blog/)

- [Video streaming & recording](https://developer.axis.com/video-streaming-and-recording/)
- [Zipstream](https://developer.axis.com/video-streaming-and-recording/zipstream/)
- [Signed video](https://developer.axis.com/video-streaming-and-recording/signed-video/)
- [How-to guides](https://developer.axis.com/video-streaming-and-recording/zipstream/#)


- [Home page](https://developer.axis.com/)
- Zipstream

On this page

# Zipstream

Axis Zipstream technology preserves all the important forensic details you need, while lowering bandwidth and storage requirements by an average of 50% or more.

info

For more context on the topic, please see [Developer Community](https://www.axis.com/developer-community/zipstream-integration).

Unlike most compression technologies, Zipstream doesn’t just limit the bitrate - instead, three kinds of intelligent algorithms ensure that relevant forensic information is identified, recorded and preserved at full frame rate for later use.

Zipstream algorithms that analyze the video stream in real time:

- Dynamic ROI (regions of interest) — this identifies regions of interest based on objects, people, or motion in the scene, and applies the correct level of compression from a forensic perspective.
- Dynamic GOP (group of pictures) — this algorithm makes the camera send bandwidth-intense I-frames less frequently when there is no motion in the scene.
- Dynamic FPS (frames per second) — this reduces the bitrate when there is little or no motion in the scene. The camera captures and analyzes video at full frame rate, but unnecessary frames are not encoded.

With the latest Zipstream update there is a new and easy way to enable Zipstream support in your video management system (VMS) or similar application. While support for the classic way to configure Zipstream, using a few VAPIX parameters will be kept, there is now also a Zipstream storage profile parameter that automatically selects the best parameters and includes the latest encoding tools for optimal configuration. This new storage profile is requested for each video stream by the VMS by adding a parameter to the request. By doing so the camera knows that this stream is intended for storage and the best bitrate reductions will be applied. Other video streams from the camera, not optimized for storage, will be produced in parallel with their own configuration. If many video streams are requested, more than the camera can deliver, the video streams using storage profile will be prioritized, e.g. drop frames last.

Zipstream delivers video using H.264 or H.265 international video encoding standards, in both cases Zipstream is fully compatible with standard video players. Note that long GOP sizes increase the random access delay when the recorded video clip is decoded and starting just before an I-frame since the player needs to seek back to the last I-frame to start decoding. Playing from start of a recorded video clip introduces no additional delay.

- Stream URL with storage profile enabled using default values for all other settings:
`rtsp://root:pass@192.168.0.90/axis-media/media.amp? videocodec=h264&videozprofile=storage`
- Stream URL with storage profile with typical parameters asking for a resolution and framerate:
`rtsp://root:pass@192.168.0.90/axis-media/media.amp? resolution=1920x1080&fps=30&videocodec=h264&videozprofile=storage`

The new parameter `videozprofile` choose between `storage` or `classic`. If `storage` is selected the stream will use so called B-Frames (Bidirectional predicted inter frames) to reduce the bitrate even further but with additional 2 frames latency. If `videozprofile` is not given, `classic` will be used as the default (if not overridden by the new global configuration parameter).

- Stream URL with classic detailed Zipstream parameter control not using any profile:
`rtsp://root:pass@192.168.0.90/axis-media/media.amp? resolution=1920x1080&fps=30&compression=30&videocodec=h264&videozfpsmode=dynamic &videozgopmode=dynamic&videozstrenght=50&videozmaxgoplength=300`

The rest of the stream parameters and the one used above are in detail described and explained in the [Zipstream section in VAPIX](https://developer.axis.com/vapix/network-video/zipstream-technology/).

All modern cameras from Axis support Zipstream, which can be verified with the [Product selector](https://www.axis.com/products/product-selector#!/), or by using VAPIX and checking for the parameter `Properties.Zipstream.Zipstream=yes`.

Different products implement different Zipstream features. PTZ cameras have special pan/tilt/zoom optimizations. Thermal cameras are tuned for optimal storage of heat map images and encoders are optimized for smaller resolutions. To learn more about the different Zipstream features see the Zipstream [white paper](https://whitepapers.axis.com/).

You can use [AXIS Site Designer](https://www.axis.com/support/tools/axis-site-designer) to predict the storage need for video recordings from Axis cameras.

## More information [​](https://developer.axis.com/video-streaming-and-recording/zipstream/\#more-information "Direct link to More information")

- [Cut the storage, not the quality - an introduction to Zipstream](https://www.axis.com/solutions/zipstream)
- [Zipstream section in VAPIX - full integration guidelines, including examples](https://developer.axis.com/vapix/network-video/zipstream-technology/)
- [Axis Zipstream - whitepaper](https://whitepapers.axis.com/axis-zipstream-technology)
- [Download an app to compare standard H.264 to Axis Zipstream technology](https://www.axis.com/learning/educational-apps/compression-h264-vs-zipstream#/)

Last updated on **Aug 18, 2025**

[Previous\\
\\
Video streaming & recording](https://developer.axis.com/video-streaming-and-recording/) [Next\\
\\
Signed video](https://developer.axis.com/video-streaming-and-recording/signed-video/)

- [More information](https://developer.axis.com/video-streaming-and-recording/zipstream/#more-information)

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