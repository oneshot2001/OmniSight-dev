---
url: "https://developer.axis.com/analytics/axis-scene-metadata/reference/modules/fusion-tracker/"
title: "Fusion tracker | Axis developer documentation"
---

[Skip to main content](https://developer.axis.com/analytics/axis-scene-metadata/reference/modules/fusion-tracker/#__docusaurus_skipToContent_fallback)

[![Axis Communications Logo](https://developer.axis.com/img/axis-logo.svg)\\
**Axis developer documentation**](https://developer.axis.com/)

```

```

[Blog](https://developer.axis.com/blog/)

- [Metadata & Analytics](https://developer.axis.com/analytics/)
- [AXIS Object Analytics](https://developer.axis.com/analytics/axis-scene-metadata/reference/modules/fusion-tracker/#)

- [AXIS Scene Metadata](https://developer.axis.com/analytics/axis-scene-metadata/)

  - [Getting started](https://developer.axis.com/analytics/axis-scene-metadata/getting-started/)
  - [How-to guides](https://developer.axis.com/analytics/axis-scene-metadata/reference/modules/fusion-tracker/#)

  - [Reference](https://developer.axis.com/analytics/axis-scene-metadata/reference/modules/fusion-tracker/#)

    - [Concepts](https://developer.axis.com/analytics/axis-scene-metadata/reference/concepts/)

    - [Definitions](https://developer.axis.com/analytics/axis-scene-metadata/reference/modules/fusion-tracker/#)

    - [Modules](https://developer.axis.com/analytics/axis-scene-metadata/reference/modules/)

      - [Fusion tracker](https://developer.axis.com/analytics/axis-scene-metadata/reference/modules/fusion-tracker/)
      - [Video motion tracker](https://developer.axis.com/analytics/axis-scene-metadata/reference/modules/video-motion-tracker/)
      - [Video object detection tracker](https://developer.axis.com/analytics/axis-scene-metadata/reference/modules/video-object-detection-tracker/)
      - [Radar motion tracker](https://developer.axis.com/analytics/axis-scene-metadata/reference/modules/radar-motion-tracker/)
      - [Track consolidation](https://developer.axis.com/analytics/axis-scene-metadata/reference/modules/track-consolidation/)
    - [Data Formats](https://developer.axis.com/analytics/axis-scene-metadata/reference/modules/fusion-tracker/#)

    - [Protocols](https://developer.axis.com/analytics/axis-scene-metadata/reference/modules/fusion-tracker/#)

    - [Related](https://developer.axis.com/analytics/axis-scene-metadata/reference/modules/fusion-tracker/#)
- [Cloud Integrations](https://developer.axis.com/analytics/axis-scene-metadata/reference/modules/fusion-tracker/#)

- [Radar](https://developer.axis.com/analytics/axis-scene-metadata/reference/modules/fusion-tracker/#)


- [Home page](https://developer.axis.com/)
- [AXIS Scene Metadata](https://developer.axis.com/analytics/axis-scene-metadata/)
- Reference
- [Modules](https://developer.axis.com/analytics/axis-scene-metadata/reference/modules/)
- Fusion tracker

On this page

# Fusion tracker

The Fusion Tracker [module](https://developer.axis.com/analytics/axis-scene-metadata/reference/concepts/processing-modules/) works to fuse the output from several more specialized trackers into a single more comprehensive and accurate tracking data output. The module is also responsible for incorporating other features such as, for example, producing object [snapshots](https://developer.axis.com/analytics/axis-scene-metadata/reference/modules/fusion-tracker/#best-snapshot-feature) and producing object [geographical coordinates](https://developer.axis.com/analytics/axis-scene-metadata/reference/modules/fusion-tracker/#geographic-coordinates).

The data processing pipeline that includes Fusion Tracker instance can be visualized as,

video

radar

Video Motion Tracker

Video Object Detection Tracker

Radar Motion Tracker

Fusion Tracker

Object Tracking Data

A single Fusion Tracker module instance is dependent on data from module instances of the types:

- [Video Object Detection Tracker](https://developer.axis.com/analytics/axis-scene-metadata/reference/modules/video-object-detection-tracker/)
- [Video Motion Tracker](https://developer.axis.com/analytics/axis-scene-metadata/reference/modules/video-motion-tracker/)
- [Radar Motion Tracker](https://developer.axis.com/analytics/axis-scene-metadata/reference/modules/radar-motion-tracker/) (if [Radar Video Fusion Feature](https://developer.axis.com/analytics/axis-scene-metadata/reference/modules/fusion-tracker/#radar-video-fusion-feature) is supported)

Since this module builds on data from previous modules, the configuration of the input modules will also effect the fusion module instance.

## Instances [​](https://developer.axis.com/analytics/axis-scene-metadata/reference/modules/fusion-tracker/\#instances "Direct link to Instances")

A device is preconfigured with a fixed number of instances of this module type.

In general there is one instance of this module type for each physical video channel corresponding to each image sensor on the device. There is an exception for multisensor cameras that producer a panorama image that is stitched from several image sensors.

- Devices with one image sensor will have one tracker instance.
- [Multidirectional cameras](https://www.axis.com/products/multidirectional-cameras) will have one tracker instance for each image sensor.
- [Multisensor cameras](https://www.axis.com/products/multisensor-cameras) that produce a panorama image that is stitched together from several image senors will have a one instance of the tracker corresponding to the viewarea of full stitched image.

## Function [​](https://developer.axis.com/analytics/axis-scene-metadata/reference/modules/fusion-tracker/\#function "Direct link to Function")

The Fusion Tracker module tracks and estimates state of objects in the scene.
The estimated state of each tracked object is continually updated to provide up to date estimates for each object's state currently present in the scene.

- Update frequency, approx 10Hz – on [multidirectional cameras](https://www.axis.com/products/multidirectional-cameras) when multiple instances are producing data the framerate may be lower.
- Latency, approx 1s – the latency of the output varies but is generally around 1 second.
- Heartbeat, every 2s – If no objects are detected in the scene a heartbeat message (an empty scene message) will be sent every 2s.

To indicate that an object's track has been terminated (never to be updated again) a `delete` track operation is sent. Tracked objects that are classified humans may be re-identified (ReID), this is represented by sending a `rename` track operation saying that an id has been renamed. For more information on ReID, refer to the description of ReID on the [ReID concept page](https://developer.axis.com/analytics/axis-scene-metadata/reference/concepts/re-identification/).

### Object state [​](https://developer.axis.com/analytics/axis-scene-metadata/reference/modules/fusion-tracker/\#object-state "Direct link to Object state")

Depending on how the module is configured, what sensors are considered, and other factors the exact estimated state might be different. An overview of the object state estimated for each object is found below.

**General:**

- `Class` \- Object classification.

  - `Human`
  - `Human Face`
  - `Vehicle - Car`
  - `Vehicle - Bus`
  - `Vehicle - Truck`
  - `Bike`
  - `License Plate`
- `Vehicle Color` \- Vehicle color
- `Human Upper Clothing Color` \- Human upper clothing color
- `Human Lower Clothing Color` \- Human lower clothing color
- `License Plate Text` \- License plate text (requires [license plate text feature](https://developer.axis.com/analytics/axis-scene-metadata/reference/modules/fusion-tracker/#license-plate-verifier-integration-feature))
- `Image` \- The current snapshot image of the object (requires [best snapshot feature](https://developer.axis.com/analytics/axis-scene-metadata/reference/modules/fusion-tracker/#best-snapshot-feature))

**2D Image Perspective:**

- `Bounding Box` \- Object position as bounding box in the image
- `CenterOfGravity`/ `Centroid` \- Position of the object as a point in the image

**3D Device Perspective:**

- `Spherical Coordinate` \- Position of the object in spherical coordinates (requires [radar-video fusion feature](https://developer.axis.com/analytics/axis-scene-metadata/reference/modules/fusion-tracker/#radar-video-fusion-feature))
- `Speed` \- Speed of the object (requires [radar-video fusion feature](https://developer.axis.com/analytics/axis-scene-metadata/reference/modules/fusion-tracker/#radar-video-fusion-feature))
- `Direction` \- Direction of the object (requires [radar-video fusion feature](https://developer.axis.com/analytics/axis-scene-metadata/reference/modules/fusion-tracker/#radar-video-fusion-feature))

**3D World Perspective:**

- `Geolocation` \- Geographic coordinates of the object (requires [geographic coordinates feature](https://developer.axis.com/analytics/axis-scene-metadata/reference/modules/fusion-tracker/#geographic-coordinates))

### Track operations [​](https://developer.axis.com/analytics/axis-scene-metadata/reference/modules/fusion-tracker/\#track-operations "Direct link to Track operations")

- `Delete` \- marks track end
- `Rename` \- renames one track id to another id
- `Merge` \- marks a merger to two ids into one (only on [M1135 Mk II](https://www.axis.com/products/axis-m1135-mk-ii), [M1137 Mk II](https://www.axis.com/products/axis-m1137-mk-ii) and [Q3819-PVE](https://www.axis.com/products/axis-q3819-pve))
- `Split` \- marks a split of one id into two (only on [M1135 Mk II](https://www.axis.com/products/axis-m1135-mk-ii), [M1137 Mk II](https://www.axis.com/products/axis-m1137-mk-ii) and [Q3819-PVE](https://www.axis.com/products/axis-q3819-pve))

## Output protocols [​](https://developer.axis.com/analytics/axis-scene-metadata/reference/modules/fusion-tracker/\#output-protocols "Direct link to Output protocols")

This module's output can be retrieved using a variety of methods.
Depending on what method is used the data can be sent on different formats, below is a full list of ways to receive the output.

| Protocol | Name/Address | Format | Guide |
| --- | --- | --- | --- |
| [RTSP](https://developer.axis.com/analytics/axis-scene-metadata/reference/protocols/rtsp/) | **Address**: rtsp://ip-address/axis-media/media.amp?analytics=polygon **Source**: AnalyticsSceneDescription | [ONVIF `tt:Frame`](https://developer.axis.com/analytics/axis-scene-metadata/reference/data-formats/onvif/) | [Configure scene metadata over RTSP](https://developer.axis.com/analytics/axis-scene-metadata/how-to-guides/scene-metadata-over-rtsp/) |
| [MQTT](https://developer.axis.com/analytics/axis-scene-metadata/reference/protocols/mqtt/) | com.axis.analytics\_scene\_description.v0.beta | [ADF Frame](https://developer.axis.com/analytics/axis-scene-metadata/reference/data-formats/analytics-data-format/frame/) | [scene metadata over MQTT](https://developer.axis.com/analytics/axis-scene-metadata/how-to-guides/scene-metadata-over-mqtt/) |
| [Message Broker](https://developer.axis.com/analytics/axis-scene-metadata/reference/protocols/message-broker/) | com.axis.analytics\_scene\_description.v0.beta | [ADF Frame](https://developer.axis.com/analytics/axis-scene-metadata/reference/data-formats/analytics-data-format/frame/) | [ACAP example (consume-scene-metadata)](https://github.com/AxisCommunications/acap-native-sdk-examples/tree/main/message-broker/consume-scene-metadata) |

## Configuration [​](https://developer.axis.com/analytics/axis-scene-metadata/reference/modules/fusion-tracker/\#configuration "Direct link to Configuration")

There are a number of configuration options effecting the output from this module.
All of the below configuration effects all instances unless otherwise stated.

Some configuration options require a device restart to take effect.
Best practice is to restart the device when a configuration options is changed to be sure it has taken effect.

All configuration options is not available on all devices.

| Description | Method | Default value | Note | Configuration guide |
| --- | --- | --- | --- | --- |
| Best snapshot feature | Rest API, `http://<camera-ip>/config/rest/best-snapshot/v1/enabled` | OFF | See details [below](https://developer.axis.com/analytics/axis-scene-metadata/reference/modules/fusion-tracker/#best-snapshot-feature). | [Enable and Retrieve Object Snapshots](https://developer.axis.com/analytics/axis-scene-metadata/how-to-guides/best-snapshot-start/) |
| Geographic coordinates feature | Configure device geolocation and device geoorentation using designated cgi:s | OFF | This feature is implicitly enabled by configuring the device geolocation and geoorientation. Once enabled it can not be turned off other than by a device reset. Only available on some devices, see more details [below](https://developer.axis.com/analytics/axis-scene-metadata/reference/modules/fusion-tracker/#geographic-coordinates). | [Enable Geographic Coordinates](https://developer.axis.com/analytics/axis-scene-metadata/how-to-guides/configure-device-geolocation/) |
| License plate text feature | Install [AXIS License Plate Verifier ACAP](https://www.axis.com/products/axis-license-plate-verifier) | OFF | This feature is implicitly enabled by installing the [AXIS License Plate Verifier ACAP](https://www.axis.com/products/axis-license-plate-verifier). Only available on some devices, see more details [below](https://developer.axis.com/analytics/axis-scene-metadata/reference/modules/fusion-tracker/#license-plate-verifier-integration-feature). | Install ACAP |
| Image rotation | Image rotation CGI | Device dependent | The object state related using a 2D image perspective will be rotated according to the input video rotation rotation, this configuration is per instance. | [Image source rotation](https://developer.axis.com/vapix/network-video/image-source-rotation/#image-source-rotation-common-examples) |

### Features behind feature flag [​](https://developer.axis.com/analytics/axis-scene-metadata/reference/modules/fusion-tracker/\#features-behind-feature-flag "Direct link to Features behind feature flag")

warning

These are experimental features not ready for use in production. No support is provided for issues at this stage.

| Description | Feature flag name | Default value | Note |
| --- | --- | --- | --- |
| Experimental classes feature | `metadata_fusion_experimental_classes` | OFF | See details [below](https://developer.axis.com/analytics/axis-scene-metadata/reference/modules/fusion-tracker/#experimental-classes). |

### Best snapshot feature [​](https://developer.axis.com/analytics/axis-scene-metadata/reference/modules/fusion-tracker/\#best-snapshot-feature "Direct link to Best snapshot feature")

A complete list of devices that supports this feature can be found at the [AXIS Scene Metadata product page](https://www.axis.com/products/axis-scene-metadata#compatible-products) by filtering on the functionality "Best snapshot".

The Best Snapshot feature enables the capturing of object snapshots in a scene. When enabled this feature adds a base-64 encoded cropped image to both classified and unclassified objects.
The current implementation is heavily focused on bounding box size during the track lifetime.
Snapshots may be sent more than once for each object, if better alternatives are found.

This feature interfaces with the Fusion Tracker module as shown below,

video

radar

Video Motion Tracker

Video Object Detection Tracker

Radar Motion Tracker

Fusion Tracker

Best Snapshot Generator

Tracking Data with Image Snapshots

To enable this feature see [guide](https://developer.axis.com/analytics/axis-scene-metadata/how-to-guides/best-snapshot-start/).

### Radar video fusion feature [​](https://developer.axis.com/analytics/axis-scene-metadata/reference/modules/fusion-tracker/\#radar-video-fusion-feature "Direct link to Radar video fusion feature")

info

This feature is only available radar video fusion cameras such as [Q1686-DLE](https://www.axis.com/products/axis-q1686-dle) and [Q1656-DLE](https://www.axis.com/products/axis-q1656-dle).

The radar video fusion feature enables the Fusion Tracker module to also fuse tracking data generated from radar scans.

This feature will add additional information to the object state for objects detected by the radar.
The additional info includes

- `Spherical Coordinate`
- `Speed`
- `Direction`

As the radar sensor is designed to detect objects far away, using this feature will also improve detection range.

This feature can also be used in combination with the [Geographic Coordinates](https://developer.axis.com/analytics/axis-scene-metadata/reference/modules/fusion-tracker/#geographic-coordinates) this feature to receive geolocation data.

### License plate verifier integration feature [​](https://developer.axis.com/analytics/axis-scene-metadata/reference/modules/fusion-tracker/\#license-plate-verifier-integration-feature "Direct link to License plate verifier integration feature")

info

This feature is only available on [Q1686-DLE](https://www.axis.com/products/axis-q1686-dle).

The [AXIS License Plate Verifier ACAP](https://www.axis.com/products/axis-license-plate-verifier) enables capturing and recognizing of license plate text. By integrating this data into the Fusion Tracker module through this feature, the same information can now be provided for the Fusion Tracker module output.

The ALPV ACAP acap can be visualized as a processing module together with the Fusion Tracker module as such,

video

radar

Video Motion Tracker

Video Object Detection Tracker

Radar Motion Tracker

Fusion Tracker

ALPV ACAP

Tracking Data with License Plate text

### Geographic coordinates [​](https://developer.axis.com/analytics/axis-scene-metadata/reference/modules/fusion-tracker/\#geographic-coordinates "Direct link to Geographic coordinates")

info

This feature is only available radar video fusion cameras such as [Q1686-DLE](https://www.axis.com/products/axis-q1686-dle) and [Q1656-DLE](https://www.axis.com/products/axis-q1656-dle).

The geographic coordinates feature allows the object state in the output to include global, decimal degrees, geographical coordinates.
For the device to be able to calculate correct coordinates it has to be correctly and precisely configured.
Coordinates are calculated using the lowest, middle point of each bounding-box, therefore scenarios where a human's feet are not visible will result in miscalculated coordinates.

A this features is based on the data from the [Radar Motion Tracker](https://developer.axis.com/analytics/axis-scene-metadata/reference/modules/radar-motion-tracker/) module only objects being detected by the radar for a specific frame will receive geolocation data.

Read the [how-to](https://developer.axis.com/analytics/axis-scene-metadata/how-to-guides/configure-device-geolocation/) on how to enable this feature.

### Experimental classes [​](https://developer.axis.com/analytics/axis-scene-metadata/reference/modules/fusion-tracker/\#experimental-classes "Direct link to Experimental classes")

Some classes are considered experimental and can be turned on by the enabling the `experimental classes` feature flag, see [feature flags](https://developer.axis.com/analytics/axis-scene-metadata/reference/related/feature-flags/).

**Classes:**

- `Bag`
  - `Backpack`
  - `Suitcase`
  - `Other`
- `Animal`

**Attributes:**

- `Hat` on (head/face)

  - `Hard hat`
  - `Other`

## Limitations [​](https://developer.axis.com/analytics/axis-scene-metadata/reference/modules/fusion-tracker/\#limitations "Direct link to Limitations")

- [Mirroring](https://developer.axis.com/vapix/network-video/video-output-api/#set-mirroring) is not supported.

  - If the object tracking should be visualized on a mirrored image, the user must apply mirroring.
- Objects must be moving to be considered by this tracker.
- This tracker only support video stream default aspect ratios.

Last updated on **Aug 18, 2025**

[Previous\\
\\
Modules](https://developer.axis.com/analytics/axis-scene-metadata/reference/modules/) [Next\\
\\
Video motion tracker](https://developer.axis.com/analytics/axis-scene-metadata/reference/modules/video-motion-tracker/)

- [Instances](https://developer.axis.com/analytics/axis-scene-metadata/reference/modules/fusion-tracker/#instances)
- [Function](https://developer.axis.com/analytics/axis-scene-metadata/reference/modules/fusion-tracker/#function)
  - [Object state](https://developer.axis.com/analytics/axis-scene-metadata/reference/modules/fusion-tracker/#object-state)
  - [Track operations](https://developer.axis.com/analytics/axis-scene-metadata/reference/modules/fusion-tracker/#track-operations)
- [Output protocols](https://developer.axis.com/analytics/axis-scene-metadata/reference/modules/fusion-tracker/#output-protocols)
- [Configuration](https://developer.axis.com/analytics/axis-scene-metadata/reference/modules/fusion-tracker/#configuration)
  - [Features behind feature flag](https://developer.axis.com/analytics/axis-scene-metadata/reference/modules/fusion-tracker/#features-behind-feature-flag)
  - [Best snapshot feature](https://developer.axis.com/analytics/axis-scene-metadata/reference/modules/fusion-tracker/#best-snapshot-feature)
  - [Radar video fusion feature](https://developer.axis.com/analytics/axis-scene-metadata/reference/modules/fusion-tracker/#radar-video-fusion-feature)
  - [License plate verifier integration feature](https://developer.axis.com/analytics/axis-scene-metadata/reference/modules/fusion-tracker/#license-plate-verifier-integration-feature)
  - [Geographic coordinates](https://developer.axis.com/analytics/axis-scene-metadata/reference/modules/fusion-tracker/#geographic-coordinates)
  - [Experimental classes](https://developer.axis.com/analytics/axis-scene-metadata/reference/modules/fusion-tracker/#experimental-classes)
- [Limitations](https://developer.axis.com/analytics/axis-scene-metadata/reference/modules/fusion-tracker/#limitations)

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