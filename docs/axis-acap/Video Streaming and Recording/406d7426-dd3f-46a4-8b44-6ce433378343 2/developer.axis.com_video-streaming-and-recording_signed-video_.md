---
url: "https://developer.axis.com/video-streaming-and-recording/signed-video/"
title: "Signed video | Axis developer documentation"
---

[Skip to main content](https://developer.axis.com/video-streaming-and-recording/signed-video/#__docusaurus_skipToContent_fallback)

[![Axis Communications Logo](https://developer.axis.com/img/axis-logo.svg)\\
**Axis developer documentation**](https://developer.axis.com/)

```

```

[Blog](https://developer.axis.com/blog/)

- [Video streaming & recording](https://developer.axis.com/video-streaming-and-recording/)
- [Zipstream](https://developer.axis.com/video-streaming-and-recording/zipstream/)
- [Signed video](https://developer.axis.com/video-streaming-and-recording/signed-video/)
- [How-to guides](https://developer.axis.com/video-streaming-and-recording/signed-video/#)


- [Home page](https://developer.axis.com/)
- Signed video

On this page

# Signed video

By the end of 2021, Axis launched an open-source project for video authentication. This article will show you how signed video works and what steps should be taken during integration.

info

For more context on the topic, please see [Developer Community](https://www.axis.com/developer-community/signed-video).

## Benefits of signed video integration [​](https://developer.axis.com/video-streaming-and-recording/signed-video/\#benefits-of-signed-video-integration "Direct link to Benefits of signed video integration")

- Implementation of signed video by Axis provides end-to-end integrity of the data. Most video management systems are currently providing signed information when they receive the video. However, the video might be tampered with or modified before it is saved by video management system.
- Axis wants to bring a standard into the video industry and increase the reputation of video as evidence and forensic matter. Our open-source framework simplifies for clients that want to access the code, do the integration, and get support.
- Implementation of signed video by Axis will provide trust and assurance to Axis customers that the video has not been tampered with since leaving an Axis network camera. Axis view is that signed video represents the most effective way to authenticate video: by adding a signature at the point of capture and tying that to the camera’s unique ID through Axis Edge Vault, any subsequent tampering will become obvious. Sharing the video authentication software as open-source with the surveillance industry will allow similar approaches from other manufacturers to become a standard for video authentication.

## How signed video works [​](https://developer.axis.com/video-streaming-and-recording/signed-video/\#how-signed-video-works "Direct link to How signed video works")

Signed video adds cryptographic signatures to a captured video as part of the video codec format (H264 and H265) using SEI frames.

### Briefly described [​](https://developer.axis.com/video-streaming-and-recording/signed-video/\#briefly-described "Direct link to Briefly described")

- The feature collects information from previous frames and signs the information using a private encryption key. Then, packetize the produced signature together with some additional information.
- For validation, the user can then verify the information by using the signature and the corresponding public key.
- The public key is already present in the SEI frames and secured through attestation, hence the video is self-validating.

For detailed information visit the [GitHub repository](https://github.com/AxisCommunications/signed-video-framework/blob/master/feature-description.md).

## What the integration should look like [​](https://developer.axis.com/video-streaming-and-recording/signed-video/\#what-the-integration-should-look-like "Direct link to What the integration should look like")

### Essential part [​](https://developer.axis.com/video-streaming-and-recording/signed-video/\#essential-part "Direct link to Essential part")

- After recording the video SEI frames must be kept. This comprehends the exporting part too. SEI frames must be present after exporting the recordings. In order to integrate and work with signed video, you have to verify that you are not discarding the SEI frames during recording and exporting.
- After exporting the video users can verify their videos with [AXIS File Player](https://www.axis.com/ftp/pub_soft/cam_srv/file_player/latest/AxisFilePlayer.exe). The AXIS File Player supports both h264/h265 in asf and MKV and only h264 in MP4 format.
- Signed video is disabled by default. You have to enable it on the device level. For that, we have updated [VAPIX®](https://developer.axis.com/vapix/network-video/signed-video/). You can do it on RTPS level ( `videosigned=1`) per stream and `param.cgi` level ( `Image.I#.MPEG.SignedVideo.Enabled`) as a global enabler.

## How do I know signed video is supported on the device? [​](https://developer.axis.com/video-streaming-and-recording/signed-video/\#how-do-i-know-signed-video-is-supported-on-the-device "Direct link to How do I know signed video is supported on the device?")

Signed video is currently supported on devices with [Axis Edge Vault](https://www.axis.com/solutions/built-in-cybersecurity-features). You can use the [product selector](https://www.axis.com/support/tools/product-selector/shared/%5B%7B%22index%22:%5B12,3%5D,%22value%22:%22Yes%22%7D%5D) to view supported products. In terms of integration, you can use the [API discovery](https://developer.axis.com/vapix/network-video/api-discovery-service/) CGI to determine if the device is supporting signed video or not.

Here is a sample response:

Response

```codeBlockLines_e6Vv
{
    "id": "signed-video",
    "version": "1.0",
    "name": "Signed Video",
    "docLink": "https://www.axis.com/vapix_library/",
    "status": "official"
}

```

You can also check `param.cgi` to understand if it is supported and enabled. In this case `Image.I#.MPEG.SignedVideo.Enabled` is the parameter that you are looking for.

### Full integration [​](https://developer.axis.com/video-streaming-and-recording/signed-video/\#full-integration "Direct link to Full integration")

It is ideal if users can verify the signed video in playback view. If your exporting contains a proprietary video player, it would be best if you verify the video with your native client.

## How your video client can support the implementation of signed video by Axis [​](https://developer.axis.com/video-streaming-and-recording/signed-video/\#how-your-video-client-can-support-the-implementation-of-signed-video-by-axis "Direct link to How your video client can support the implementation of signed video by Axis")

We suggest two different ways:

### 1\. If you are using [AXIS Media Control](https://www.axis.com/support/tools/axis-media-control) [​](https://developer.axis.com/video-streaming-and-recording/signed-video/\#1-if-you-are-using-axis-media-control "Direct link to 1-if-you-are-using-axis-media-control")

You have already reached halfway and can use the following functions to verify your video:

- First set `EnableSignedVideoVerification` to true.

- Then call `SignatureState`, (it could be done for every image in `OnNewImageEvent`) to get the state of the signed video.



SignatureState





```codeBlockLines_e6Vv
typedef enum {
      unknown = 0,
      missing = 1,
      valid = 2,
      invalid = 3,
} SignatureState;

```

- SignatureDescription can be used to get a string with vendor information.


### 2\. If you are not using [AXIS Media Control](https://www.axis.com/support/tools/axis-media-control) [​](https://developer.axis.com/video-streaming-and-recording/signed-video/\#2-if-you-are-not-using-axis-media-control "Direct link to 2-if-you-are-not-using-axis-media-control")

You must merge the C code from the signed video framework into your code.

- The framework works in Linux and Windows.
- You can find the sample codes in the [GitHub repository](https://github.com/AxisCommunications/signed-video-framework-examples).

## Implementation tips [​](https://developer.axis.com/video-streaming-and-recording/signed-video/\#implementation-tips "Direct link to Implementation tips")

All present tips are related to how we solve things inside AXIS File Player, where we want to display frames that have been validated.

- While starting to play the video, you start showing the I-frame. Until the first SEI frame you will not know if the video is signed or not. So, for the playback function you should consider buffering the video before showing whether it is signed or not.
- The same concept is valid for the end of the video. It may finish before receiving an SEI frame, and you should be prepared to show the end of the video without signed verification.
- The easiest approach is to have a validation feature that can scan a whole file and say if it is fully valid or not. In this case, you do not need to prebuffer. The problem you would encounter is what to say about the end of the file where there are remaining frames without the sign information. You may omit or say the last remaining X number of frames could not be verified. The signed video framework will soon provide an accumulated report for all validated GOPs in one chunk.
- If you have a timeline bar you can mark it as 'green' when validation is confirmed. If you watch new material it will not be validated upon view, but you can scroll back if a GOP becomes invalid depending on your implementation.
- SEI frames increase the bandwidth consumption. It can be neglectable if you are running on premise, but you should consider this if you are paying ingress costs on a cloud system. The bitrate increase depends on FPS and GOP-length. Some specific numbers can be found in the table below for reference.

![Bitrate increase](https://www.axis.com/sites/axis/files/styles/standard_1360_x_auto/public/2022-07/gop_lenght_2600x1950.jpg?itok=ThEf-ecW)

| Framerate | GOP 1 sec | GOP2 sec |
| --- | --- | --- |
| 8 fps | 21 kbits/s | 12 kbits/s |
| 15 fps | 23 kbits/s | 14 kbits/s |
| 30 fps | 27 kbits/s | 17 kbits/s |
| 60 fps | 35 kbits/s | 25 kbits/s |

## More information [​](https://developer.axis.com/video-streaming-and-recording/signed-video/\#more-information "Direct link to More information")

- [VAPIX®](https://developer.axis.com/vapix/network-video/signed-video/)
- [GitHub repository](https://github.com/AxisCommunications/signed-video-framework)

Last updated on **Aug 18, 2025**

[Previous\\
\\
Zipstream](https://developer.axis.com/video-streaming-and-recording/zipstream/) [Next\\
\\
AV1™ integration](https://developer.axis.com/video-streaming-and-recording/how-to-guides/integration-guide-av1/)

- [Benefits of signed video integration](https://developer.axis.com/video-streaming-and-recording/signed-video/#benefits-of-signed-video-integration)
- [How signed video works](https://developer.axis.com/video-streaming-and-recording/signed-video/#how-signed-video-works)
  - [Briefly described](https://developer.axis.com/video-streaming-and-recording/signed-video/#briefly-described)
- [What the integration should look like](https://developer.axis.com/video-streaming-and-recording/signed-video/#what-the-integration-should-look-like)
  - [Essential part](https://developer.axis.com/video-streaming-and-recording/signed-video/#essential-part)
- [How do I know signed video is supported on the device?](https://developer.axis.com/video-streaming-and-recording/signed-video/#how-do-i-know-signed-video-is-supported-on-the-device)
  - [Full integration](https://developer.axis.com/video-streaming-and-recording/signed-video/#full-integration)
- [How your video client can support the implementation of signed video by Axis](https://developer.axis.com/video-streaming-and-recording/signed-video/#how-your-video-client-can-support-the-implementation-of-signed-video-by-axis)
  - [1\. If you are using AXIS Media Control](https://developer.axis.com/video-streaming-and-recording/signed-video/#1-if-you-are-using-axis-media-control)
  - [2\. If you are not using AXIS Media Control](https://developer.axis.com/video-streaming-and-recording/signed-video/#2-if-you-are-not-using-axis-media-control)
- [Implementation tips](https://developer.axis.com/video-streaming-and-recording/signed-video/#implementation-tips)
- [More information](https://developer.axis.com/video-streaming-and-recording/signed-video/#more-information)

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