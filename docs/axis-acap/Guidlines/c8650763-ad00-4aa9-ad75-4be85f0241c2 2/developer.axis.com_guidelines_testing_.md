---
url: "https://developer.axis.com/guidelines/testing/"
title: "Testing | Axis developer documentation"
---

[Skip to main content](https://developer.axis.com/guidelines/testing/#__docusaurus_skipToContent_fallback)

[![Axis Communications Logo](https://developer.axis.com/img/axis-logo.svg)\\
**Axis developer documentation**](https://developer.axis.com/)

```

```

[Blog](https://developer.axis.com/blog/)

- [Guidelines](https://developer.axis.com/guidelines/)
- [Testing](https://developer.axis.com/guidelines/testing/)

- [Home page](https://developer.axis.com/)
- Testing

On this page

# Testing

The testing guidelines cover various domains, each outlined in a separate chapter.

## Axis product series and AXIS OS development approach [​](https://developer.axis.com/guidelines/testing/\#axis-product-series-and-axis-os-development-approach "Direct link to Axis product series and AXIS OS development approach")

### Axis camera series [​](https://developer.axis.com/guidelines/testing/\#axis-camera-series "Direct link to Axis camera series")

Most Axis IP camera models are categorized as either M, P or Q line. These categories indicate differences in areas like power consumption, level of performance, RAM, etc., things that will affect number of streams, FPS, etc.

The fact that these categories of cameras differ in specifications also means these groups will differ in the required testing. Consider this in your test strategy so that you don’t expect the same results from different cameras.

### About AXIS OS [​](https://developer.axis.com/guidelines/testing/\#about-axis-os "Direct link to About AXIS OS")

In an Axis network video product, the AXIS OS functions as the operating system, being the software that manages all aspects of the product’s performance. That includes, for example:

- The imaging pipeline that collects and processes raw video data
- The web server that transmits video over a network
- The camera’s web interface for access via an internet browser
- The event handling system that relays alarms and messages to users
- All API interfaces (like [VAPIX®](https://developer.axis.com/vapix/) and ONVIF) for handling integration with other systems

AXIS OS is customized for each product, to support both the imaging hardware and any product specific features. However, AXIS OS is developed on and released from a single code branch, ensuring that different product models will have similar or same behavior if they have the same AXIS OS version. Before a new product is released, the stability of the AXIS OS is ensured through extensive testing of the product’s features.

After release, AXIS OS updates are scheduled to add features and/or to fix bugs. These updates are also tested extensively, both to ensure continued stability and to maintain backward compatibility.

### AXIS OS development [​](https://developer.axis.com/guidelines/testing/\#axis-os-development "Direct link to AXIS OS development")

To learn more about AXIS OS development please visit the [AXIS OS portal](https://help.axis.com/axis-os#axis-os-tracks).

### Axis product naming [​](https://developer.axis.com/guidelines/testing/\#axis-product-naming "Direct link to Axis product naming")

The product name reveals various product specifications in a standardized way. To learn more about the naming convention for Axis devices, run the [online course](https://www.axis.com/learning/academy/elearning/axis-network-camera-naming-convention).

### AXIS OS tracks [​](https://developer.axis.com/guidelines/testing/\#axis-os-tracks "Direct link to AXIS OS tracks")

Axis offers two separate AXIS OS tracks, Active and Long-Term Support (LTS), which both have their own release intervals. These tracks also differ in the number of changes that can be expected in each release, so the test strategy for each type should be different.

For some older camera models and in exceptional cases, there may be product specific AXIS OS versions, but this is the exception. Most AXIS OS releases are either on the Active track or the LTS track.

Visit the [AXIS OS portal](http://help.axis.com/axis-os#axis-os-tracks) to read more about the different tracks.

## Test strategies and recommendations [​](https://developer.axis.com/guidelines/testing/\#test-strategies-and-recommendations "Direct link to Test strategies and recommendations")

With these insights on Axis’ different AXIS OS tracks, and with consideration to which track supports your business objectives, it’s time to look at the test strategy. Our main purpose with these guidelines and recommendations is to help you test smarter, so let's see how you can use the above knowledge to improve your interoperability testing.

### A risk-based testing approach [​](https://developer.axis.com/guidelines/testing/\#a-risk-based-testing-approach "Direct link to A risk-based testing approach")

Instead of “testing everything on every camera and every AXIS OS release” it is possible to adjust the testing based on risk. Here we cover two dimensions of risk:

- Grouping of similar cameras. Testing on one of the cameras is assumed to be enough to cover the whole group. Where more risk is accepted, more aggressive grouping can be applied (grouping more cameras together).
- Adjusting test scope based on changes made in the AXIS OS release. When large changes are expected, a larger test scope is needed. When changes are small, a limited test scope is assumed to be enough. Where more risk is accepted, more reductions in scoping can be done.

### Grouping of cameras for test purposes [​](https://developer.axis.com/guidelines/testing/\#grouping-of-cameras-for-test-purposes "Direct link to Grouping of cameras for test purposes")

As a rule of thumb, similar names will mean that the cameras are similar. But cameras can also be similar from a video management system point of view, although the names are radically different. For interoperability testing, the most important grouping parameters are:

- Feature set
- Hardware platform

Depending on the risk accepted, the whole Axis portfolio of roughly 100-200 cameras could be represented by 3-10 cameras for testing purposes.

## AXIS OS tracks and risk-based testing [​](https://developer.axis.com/guidelines/testing/\#axis-os-tracks-and-risk-based-testing "Direct link to AXIS OS tracks and risk-based testing")

The AXIS OS versions on the Active track and the AXIS OS versions on the LTS track are quite different in the number of changes that can be expected in each release, so the test strategy for each type should be different.

### Active track [​](https://developer.axis.com/guidelines/testing/\#active-track "Direct link to Active track")

All AXIS OS releases that are on the Active track (x.y0, e.g. 9.10) have been developed and tested by Axis on a common code base. Therefore, they can be grouped with high confidence. The changes on the Active track are greater than on an LTS track, so risk due to code changes is higher than for LTS.

#### Recommended test strategy [​](https://developer.axis.com/guidelines/testing/\#recommended-test-strategy-active-track "Direct link to Recommended test strategy")

- Test all features in each group. Either use one camera to represent the group or spread the tests on different cameras in the same group. Typically, this could be 3-10 different cameras that would represent all 50-100 cameras for that AXIS OS release.

### LTS track [​](https://developer.axis.com/guidelines/testing/\#lts-track "Direct link to LTS track")

Within each LTS track, there are typically very small changes and Axis will do everything to keep new LTS releases as backward compatible as possible. All AXIS OS releases that have the same LTS version have been developed and tested together. For this reason, the risk due to code changes is very low and very aggressive grouping can be done.

#### Recommended test strategy [​](https://developer.axis.com/guidelines/testing/\#recommended-test-strategy-lts-track "Direct link to Recommended test strategy")

- Run the most important tests on one camera model.
- Note that this test strategy only applies to updates on a specific LTS track, e.g. new releases of the 9.80 version. This does not apply to jumping from one LTS track to another (e.g. 9.80 to 10.12). A new LTS track is branched out from an Active release and thus will include new features, changes to default settings, and performance adjustments that could affect compatibility with your system.

### Product specific AXIS OS versions [​](https://developer.axis.com/guidelines/testing/\#product-specific-axis-os-versions "Direct link to Product specific AXIS OS versions")

For some older camera models and in exceptional cases, there may be product specific AXIS OS versions. Any product specific AXIS OS version (x.y5, e.g. 8.55) is specifically built for that product. Although it is based on the common AXIS OS platform, it has been developed and tested independently and as such it carries the greatest risk.

#### Recommended test strategy [​](https://developer.axis.com/guidelines/testing/\#recommended-test-strategy "Direct link to Recommended test strategy")

- Test all features on each camera with a product specific AXIS OS version.

## Balancing new camera integration, Active track and LTS [​](https://developer.axis.com/guidelines/testing/\#balancing-new-camera-integration-active-track-and-lts "Direct link to Balancing new camera integration, Active track and LTS")

### Benefits of focusing on LTS tracks [​](https://developer.axis.com/guidelines/testing/\#benefits-of-focusing-on-lts-tracks "Direct link to Benefits of focusing on LTS tracks")

If the number of new cameras and new AXIS OS versions from Axis is a challenge, then a good solution can be to primarily focus on supporting the LTS tracks. This is the track that requires by far the least amount of work to test and to maintain, as the changes from the Axis side are small. Axis has a focus on backward compatibility on the LTS tracks, which means that each release on an LTS track should be fully backwards compatible with previous releases on that track, making it safe and easy to keep AXIS OS updated.

This means that two substantial benefits of fully supporting an LTS track are:

- You will support the full portfolio of Axis devices released on that track (e.g., 150+ Axis devices are released on LTS 2022)
- There is little or no testing required for supporting new LTS releases within that track

However, it should also be noted that new cameras typically are introduced on the Active track and there are other benefits of also following Active, or at least keeping an eye on the Active track.

### Benefits of following the Active track [​](https://developer.axis.com/guidelines/testing/\#benefits-of-following-the-active-track "Direct link to Benefits of following the Active track")

Following the Active track has benefits, also on the testing:

- Easier integration of new cameras
- Less risk due to small incremental changes (compared to waiting two years for the next LTS version)
- Quick feedback on upcoming changes (compared to waiting two years for the next LTS version)

Focusing on LTS and not Active track releases will mean less test effort and therefore can be a good choice. However, stacking up a large amount of changes will make compatibility issues harder to track down. What you save in testing may result in greater surprises later on.

If you follow the Active track, you will continuously upgrade your device integration support and possible issues will be handled regularly. This means that any such issue should not be so widespread before you have a chance to identify and correct it, i.e., you get a quick feedback loop by following the Active track. If you don’t, potential issues might get widespread usage before you notice it and that makes the deployment of fixes much more of a challenge. The speed with which possible bugs and compatibility issues are reported back to Axis will minimize how much impact the issue will have on your customers.

### New cameras and the Active track [​](https://developer.axis.com/guidelines/testing/\#new-cameras-and-the-active-track "Direct link to New cameras and the Active track")

New camera models are primarily released on the Active track. These cameras can be used as a part of a test group and thus make the testing more efficient. E.g. if the new model P1468-LE is released with 10.11, then this new camera can be used to represent a group of other cameras on 10.11. In this way, any testing done to integrate the new camera with 10.11 can be counted towards the testing of 10.11 in general. Following the Active track will make it easier to integrate new cameras.

### Avoid product specific versions where possible [​](https://developer.axis.com/guidelines/testing/\#avoid-product-specific-versions-where-possible "Direct link to Avoid product specific versions where possible")

If a camera was first released with a product specific AXIS OS version, e.g. 8.55, and now there exists a later Active or LTS version, then the original AXIS OS version should be ignored, and a newer AXIS OS release should be used for integration.

## Selecting Axis devices to use when testing new AXIS OS releases [​](https://developer.axis.com/guidelines/testing/\#selecting-axis-devices-to-use-when-testing-new-axis-os-releases "Direct link to Selecting Axis devices to use when testing new AXIS OS releases")

The selection of devices to use when testing new AXIS OS releases should be risk based. Axis can provide general recommendations, but you will have to ask yourself the following questions:

- How much risk do I accept? More risk = fewer cameras
- How much effort do I want to put in? Fewer cameras = less effort
- Are there some important specific features, or features that have been problematic for me in the past? Select at least one camera with the relevant features.

### Testing pool [​](https://developer.axis.com/guidelines/testing/\#testing-pool "Direct link to Testing pool")

The following list is an example that can be used for testing after track AXIS OS version 10.12 and later releases. The devices have been selected based on both features and hardware platforms.

| Priority | Camera name | Chip platform | Special features |
| --- | --- | --- | --- |
| 1 | [AXIS Q6075 PTZ Network Camera](https://www.axis.com/products/axis-q6075) | ARTPEC-7 | PTZ |
| 2 | [AXIS Q1656-LE Box Camera](https://www.axis.com/products/axis-q1656-le) | ARTPEC-8 | Latest chipset |
| 3 | [AXIS P1375 Network Camera](https://www.axis.com/products/axis-p1375) | ARTPEC-7 | Popular model |
| 4 | [AXIS M3077-PLVE Network Camera](https://www.axis.com/products/axis-m3077-plve) | ARTPEC-7 | Fisheye |
| 5 | [AXIS FA54 Main Unit](https://www.axis.com/products/axis-fa54-main-unit) | ARTPEC-6 | Modular |
| 6 | [AXIS I8016-LVE Network Video Intercom](https://www.axis.com/products/axis-i8016-lve) | ARTPEC-7 | Intercom |
| 7 | [AXIS M4216-V](https://www.axis.com/products/axis-m4216-v) / [LV Dome Camera](https://www.axis.com/products/axis-m4216-lv) | CV25 |  |
|  | **Others to consider, in no particular order** |  |  |
|  | [AXIS D2110-VE Security Radar](https://www.axis.com/products/axis-d2110-ve-security-radar) | ARTPEC-7 | Security radar |
|  | [AXIS C1310-E Network Horn Speaker](https://www.axis.com/products/axis-c1310-e) | 6ULL | Speaker |

info

Please note that this list is only an example. Other devices can be more relevant based on region, vertical, VMS feature set, etc. The important part is selecting a set of devices that is a good representation of the differences in the Axis portfolio and a relevant sample for your context.

## More information [​](https://developer.axis.com/guidelines/testing/\#more-information "Direct link to More information")

- [Axis firmware downloads](http://www.axis.com/support/firmware)
- [AXIS OS portal](https://help.axis.com/axis-os)
- [AXIS OS release notes](https://help.axis.com/axis-os-release-notes#axis-os-11-1)
- [AXIS OS roadmap](https://help.axis.com/axis-os#developer-information)
- [AXIS OS - Beta and test versions](http://www.axis.com/partner_pages/development_news.php?t=cv&mid=5057) (Requires Technology Integration Partner (TIP) access)

info

Log in to your TIP account to gain access to the latest AXIS OS beta and test versions on the [partner web](https://www.axis.com/partner_pages). Read more about the [Axis Technology Integration Partner Program](https://www.axis.com/partner/technology-integration-partner-program).

Last updated on **Aug 27, 2025**

[Previous\\
\\
Guidelines](https://developer.axis.com/guidelines/)

- [Axis product series and AXIS OS development approach](https://developer.axis.com/guidelines/testing/#axis-product-series-and-axis-os-development-approach)
  - [Axis camera series](https://developer.axis.com/guidelines/testing/#axis-camera-series)
  - [About AXIS OS](https://developer.axis.com/guidelines/testing/#about-axis-os)
  - [AXIS OS development](https://developer.axis.com/guidelines/testing/#axis-os-development)
  - [Axis product naming](https://developer.axis.com/guidelines/testing/#axis-product-naming)
  - [AXIS OS tracks](https://developer.axis.com/guidelines/testing/#axis-os-tracks)
- [Test strategies and recommendations](https://developer.axis.com/guidelines/testing/#test-strategies-and-recommendations)
  - [A risk-based testing approach](https://developer.axis.com/guidelines/testing/#a-risk-based-testing-approach)
  - [Grouping of cameras for test purposes](https://developer.axis.com/guidelines/testing/#grouping-of-cameras-for-test-purposes)
- [AXIS OS tracks and risk-based testing](https://developer.axis.com/guidelines/testing/#axis-os-tracks-and-risk-based-testing)
  - [Active track](https://developer.axis.com/guidelines/testing/#active-track)
  - [LTS track](https://developer.axis.com/guidelines/testing/#lts-track)
  - [Product specific AXIS OS versions](https://developer.axis.com/guidelines/testing/#product-specific-axis-os-versions)
- [Balancing new camera integration, Active track and LTS](https://developer.axis.com/guidelines/testing/#balancing-new-camera-integration-active-track-and-lts)
  - [Benefits of focusing on LTS tracks](https://developer.axis.com/guidelines/testing/#benefits-of-focusing-on-lts-tracks)
  - [Benefits of following the Active track](https://developer.axis.com/guidelines/testing/#benefits-of-following-the-active-track)
  - [New cameras and the Active track](https://developer.axis.com/guidelines/testing/#new-cameras-and-the-active-track)
  - [Avoid product specific versions where possible](https://developer.axis.com/guidelines/testing/#avoid-product-specific-versions-where-possible)
- [Selecting Axis devices to use when testing new AXIS OS releases](https://developer.axis.com/guidelines/testing/#selecting-axis-devices-to-use-when-testing-new-axis-os-releases)
  - [Testing pool](https://developer.axis.com/guidelines/testing/#testing-pool)
- [More information](https://developer.axis.com/guidelines/testing/#more-information)

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