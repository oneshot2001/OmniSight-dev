---
url: "https://developer.axis.com/acap/axis-devices-and-compatibility/"
title: "Axis devices & compatibility | Axis developer documentation"
---

[Skip to main content](https://developer.axis.com/acap/axis-devices-and-compatibility/#__docusaurus_skipToContent_fallback)

[![Axis Communications Logo](https://developer.axis.com/img/axis-logo.svg)\\
**Axis developer documentation**](https://developer.axis.com/)

[ACAP version 12](https://developer.axis.com/acap/axis-devices-and-compatibility/)

- [ACAP version 12](https://developer.axis.com/acap/axis-devices-and-compatibility/)
- [ACAP version 4](https://developer.axis.com/acap/4/axis-devices-and-compatibility/)
- [ACAP version 3](https://developer.axis.com/acap/3/)

```

```

[Blog](https://developer.axis.com/blog/)

- [ACAP version 12](https://developer.axis.com/acap/)
- [Get started](https://developer.axis.com/acap/axis-devices-and-compatibility/#)

- [Develop ACAP applications](https://developer.axis.com/acap/develop/)

- [Supported APIs](https://developer.axis.com/acap/api/)

- [Version history](https://developer.axis.com/acap/version-history/)
- [Axis devices & compatibility](https://developer.axis.com/acap/axis-devices-and-compatibility/)
- [Computer vision on device](https://developer.axis.com/acap/computer-vision-on-device/)
- [Cloud integrations](https://developer.axis.com/acap/cloud-integrations/)
- [Service for partners](https://developer.axis.com/acap/axis-devices-and-compatibility/#)

- [FAQ](https://developer.axis.com/acap/faq/)

- [Get help & give feedback](https://developer.axis.com/acap/get-help-and-give-feedback/)
- [Develop container applications](https://developer.axis.com/acap/develop-container-applications/)

- [Release notes](https://developer.axis.com/acap/release-notes/)


- [Home page](https://developer.axis.com/)
- Axis devices & compatibility

Version: ACAP version 12

On this page

# Axis devices & compatibility

Compatibility means that if an ACAP application can be installed and run on a
specific device, then the ACAP application is compatible with the device.
Compatibility depends on both hardware and software.

## Hardware compatibility [​](https://developer.axis.com/acap/axis-devices-and-compatibility/\#hardware-compatibility "Direct link to Hardware compatibility")

ACAP applications are supported on a large portion of all Axis devices. For an
ACAP application to be hardware compatible with a specific device, it must be
compiled using the SDK corresponding to the chip architecture in the device.

The ACAP Native SDK is hardware compatible with products using the chips listed
in the table below:

| Chip | Architecture |
| --- | --- |
| ARTPEC-6 | armv7hf |
| ARTPEC-7 | armv7hf |
| ARTPEC-8 | aarch64 |
| ARTPEC-9 | aarch64 |
| CV25 | aarch64 |
| i.MX 6SoloX | armv7hf |

See detailed information in the [product interface\\
guide](https://www.axis.com/developer-community/product-interface-guide).

## Software compatibility [​](https://developer.axis.com/acap/axis-devices-and-compatibility/\#software-compatibility "Direct link to Software compatibility")

An ACAP application is software compatible with AXIS OS if the APIs and other
runtime features are available in the specific AXIS OS release. The
availability of APIs in AXIS OS depends on both the AXIS OS version and the
device itself since some APIs are only relevant for certain devices. For
example, the Video capture API is only available on devices with an image
sensor

Choose the appropriate SDK version based on what AXIS OS version you want
supporting your ACAP application.

How do I know if my device supports a specific AXIS OS version? Please check
the web page for download of [device software](https://www.axis.com/support/device-software).

info

**Device software** has previously been named **firmware**.

The **Image version** referred to in the table below is the Docker image
release tag available from Docker Hub:

- [ACAP Native SDK](https://hub.docker.com/r/axisecp/acap-native-sdk/tags)

The tag format is `<image-version>-<architecture>-<container-distribution>`.
An example of a tag is `axisecp/acap-native-sdk:12.0.0-aarch64-ubuntu24.04`
where `12.0.0` maps to the image version.

### ACAP version 12 releases [​](https://developer.axis.com/acap/axis-devices-and-compatibility/\#acap-version-12-releases "Direct link to ACAP version 12 releases")

| ACAP Release | ACAP Native SDK Image version | Compatible with AXIS OS version |
| --- | --- | --- |
| 12.0 | 12.0.0 | 12.0 and later until LTS |
| 12.1 | 12.1.0 | 12.1 and later until LTS |
| 12.2 | 12.2.0 | 12.2 and later until LTS |
| 12.3 | 12.3.0 | 12.3 and later until LTS |
| 12.4 | 12.4.0 | 12.4 and later until LTS |
| 12.5 | 12.5.0 | 12.5 and later until LTS |
| 12.6 | 12.6.0 | 12.6 and later until LTS |

## Forward compatibility [​](https://developer.axis.com/acap/axis-devices-and-compatibility/\#forward-compatibility "Direct link to Forward compatibility")

An ACAP application is forward compatible for the AXIS OS related to a specific
SDK version. This means that the ACAP application is compatible for the listed
AXIS OS version and future versions until the next AXIS OS LTS (Long Term
Support) version. After an LTS, there may be changes that break compatibility,
for example when a deprecated API is removed. Breaking changes are always
announced in advance.

An ACAP application built with an SDK that is based on an older AXIS OS version
should always work on a newer AXIS OS version within the same LTS window.

## Feature growth between LTS releases [​](https://developer.axis.com/acap/axis-devices-and-compatibility/\#feature-growth-between-lts-releases "Direct link to Feature growth between LTS releases")

To get new features, always use the latest ACAP SDK release. A new feature
could be, for example, a new version of an API.

New SDK versions between LTS releases always add functionality in a way that an
ACAP built using a previous version will still compile with the new version
of the SDK.

Read more about AXIS OS release tracks and related information
[here](https://help.axis.com/axis-os).

## Supporting older AXIS OS [​](https://developer.axis.com/acap/axis-devices-and-compatibility/\#supporting-older-axis-os "Direct link to Supporting older AXIS OS")

If you want an ACAP application to be compatible with older AXIS OS, you need
to choose an SDK for an older AXIS OS.

See the [version table](https://developer.axis.com/acap/#acap-versioning) to find the appropriate documentation depending
on the AXIS OS LTS version you want to target.

## Camera loan tool [​](https://developer.axis.com/acap/axis-devices-and-compatibility/\#camera-loan-tool "Direct link to Camera loan tool")

Find information about [virtual\\
loan](https://www.axis.com/developer-community/axis-virtual-loan-tool).

Last updated on **Sep 25, 2025**

[Previous\\
\\
Version history](https://developer.axis.com/acap/version-history/) [Next\\
\\
Computer vision on device](https://developer.axis.com/acap/computer-vision-on-device/)

- [Hardware compatibility](https://developer.axis.com/acap/axis-devices-and-compatibility/#hardware-compatibility)
- [Software compatibility](https://developer.axis.com/acap/axis-devices-and-compatibility/#software-compatibility)
  - [ACAP version 12 releases](https://developer.axis.com/acap/axis-devices-and-compatibility/#acap-version-12-releases)
- [Forward compatibility](https://developer.axis.com/acap/axis-devices-and-compatibility/#forward-compatibility)
- [Feature growth between LTS releases](https://developer.axis.com/acap/axis-devices-and-compatibility/#feature-growth-between-lts-releases)
- [Supporting older AXIS OS](https://developer.axis.com/acap/axis-devices-and-compatibility/#supporting-older-axis-os)
- [Camera loan tool](https://developer.axis.com/acap/axis-devices-and-compatibility/#camera-loan-tool)

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