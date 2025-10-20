---
url: "https://developer.axis.com/acap/api/"
title: "Supported APIs | Axis developer documentation"
---

[Skip to main content](https://developer.axis.com/acap/api/#__docusaurus_skipToContent_fallback)

[![Axis Communications Logo](https://developer.axis.com/img/axis-logo.svg)\\
**Axis developer documentation**](https://developer.axis.com/)

[ACAP version 12](https://developer.axis.com/acap/api/)

- [ACAP version 12](https://developer.axis.com/acap/api/)
- [ACAP version 4](https://developer.axis.com/acap/4/api/)
- [ACAP version 3](https://developer.axis.com/acap/3/api/)

```

```

[Blog](https://developer.axis.com/blog/)

- [ACAP version 12](https://developer.axis.com/acap/)
- [Get started](https://developer.axis.com/acap/api/#)

- [Develop ACAP applications](https://developer.axis.com/acap/develop/)

- [Supported APIs](https://developer.axis.com/acap/api/)

  - [API compatibility guide](https://developer.axis.com/acap/api/api_compatibility_guide/)
- [Version history](https://developer.axis.com/acap/version-history/)
- [Axis devices & compatibility](https://developer.axis.com/acap/axis-devices-and-compatibility/)
- [Computer vision on device](https://developer.axis.com/acap/computer-vision-on-device/)
- [Cloud integrations](https://developer.axis.com/acap/cloud-integrations/)
- [Service for partners](https://developer.axis.com/acap/api/#)

- [FAQ](https://developer.axis.com/acap/faq/)

- [Get help & give feedback](https://developer.axis.com/acap/get-help-and-give-feedback/)
- [Develop container applications](https://developer.axis.com/acap/develop-container-applications/)

- [Release notes](https://developer.axis.com/acap/release-notes/)


- [Home page](https://developer.axis.com/)
- Supported APIs

Version: ACAP version 12

On this page

# Supported APIs

With the Native SDK, you can create ACAP applications using a variety of APIs to
interact with the functionality of Axis devices.

For a complete overview of supported APIs in each SDK version, see the [API Compatibility Guide](https://developer.axis.com/acap/api/api_compatibility_guide/).

The SDK version for your application is determined by the AXIS OS version on your target device.
The [API Compatibility Guide](https://developer.axis.com/acap/api/api_compatibility_guide/) lists all officially supported APIs that are recommended for production use.
These APIs follow our [lifecycle management policy](https://www.axis.com/support/axis-os) for updates and deprecations, ensuring stable and reliable functionality for your applications.

Below is a list of the
supported APIs. For each API, you will first find a link to its technical
reference documentation, followed by a short description of its purpose, its
device compatibility, and finally links to one or more official ACAP examples
that demonstrate its use.

info

APIs on this page list which chips they are supported on, but note that the
underlying feature also needs to be found and supported on the product. For
example, the Video capture API is only available on devices with an image
sensor.

## Beta APIs [​](https://developer.axis.com/acap/api/\#beta-apis "Direct link to Beta APIs")

An API marked as Beta is made for prototyping a solution early on in the process
to get feedback from developers.

warning

A Beta API should not be used for production, it's likely to introduce
breaking changes.

Application developers are encouraged to test Beta APIs and leave feedback.
The best way to leave feedback is to use the GitHub issue tracker of the Beta
API implementation (for example if the prototype is an ACAP application image)
or in the Beta API example if there is one available.

## Axis APIs [​](https://developer.axis.com/acap/api/\#axis-apis "Direct link to Axis APIs")

### Edge storage API [​](https://developer.axis.com/acap/api/\#edge-storage-api "Direct link to Edge storage API")

Go to the [ACAP API Documentation](https://developer.axis.com/acap/api/src/api/axstorage/html/index.html) for detailed functional descriptions of this API.

The Edge storage API allows the application to save and retrieve data on mounted storage devices such as SD cards and NAS (Network Attached Storage) units. An application can only modify its own files on the storage device.
An application can both send and receive events.

#### Compatibility [​](https://developer.axis.com/acap/api/\#compatibility-edge-storage-api "Direct link to Compatibility")

The API is supported on products with the following chips:

- ARTPEC-9
- ARTPEC-8
- ARTPEC-7
- ARTPEC-6
- Ambarella CV25
- i.MX 6SoloX

#### Version history [​](https://developer.axis.com/acap/api/\#version-history-edge-storage-api "Direct link to Version history")

The Edge storage API was introduced in Native SDK 1.11.

#### Code examples [​](https://developer.axis.com/acap/api/\#code-examples-edge-storage-api "Direct link to Code examples")

- [axstorage](https://github.com/AxisCommunications/acap-native-sdk-examples/tree/main/axstorage)
  - This application demonstrates the usage of axstorage APIs, offering the following functionality:
    - List configured storage devices.
    - Subscribe to events from all storage devices.
    - Set up and utilize all available/mounted storage devices.
    - Continuously write data to two files on all available storage devices.
    - Automatically release any unmounted storage devices.

* * *

### Event API [​](https://developer.axis.com/acap/api/\#event-api "Direct link to Event API")

Go to the [ACAP API Documentation](https://developer.axis.com/acap/api/src/api/axevent/html/index.html) for detailed functional descriptions of this API.

The Axevent API provides:

- an interface to the event system found in Axis products.
- applications with a mechanism for sending and receiving events.

An application can both send and receive events.

#### Event types [​](https://developer.axis.com/acap/api/\#event-types "Direct link to Event types")

**Stateless (Pulse)** – An event that indicates that something has occurred. Typically used to trigger some action rule.

**Stateful (State)** – An event with an active and inactive state. Typically used in action rules like “record while active”.

**Data (Application Data)** – An event that includes data that needs to be processed by the consuming application such as transaction data, license plate or other dynamic data. A data event is normally not used to trigger generic action rules.

#### Supported namespaces [​](https://developer.axis.com/acap/api/\#supported-namespaces "Direct link to Supported namespaces")

When declaring events it is required to set a namespace. Following are the supported namespaces:

**tnsaxis** – Axis namespace to use with Axis events

**tns1** – ONVIF namespace to use with ONVIF events

#### Compatibility [​](https://developer.axis.com/acap/api/\#compatibility-event-api "Direct link to Compatibility")

The API is supported on products with the following chips:

- ARTPEC-9
- ARTPEC-8
- ARTPEC-7
- ARTPEC-6
- Ambarella CV25
- i.MX 6SoloX

#### Version history [​](https://developer.axis.com/acap/api/\#version-history-event-api "Direct link to Version history")

The Axevent API was introduced in Native SDK 1.0.

#### Code examples [​](https://developer.axis.com/acap/api/\#code-examples-event-api "Direct link to Code examples")

- [send\_event](https://github.com/AxisCommunications/acap-native-sdk-examples/tree/main/axevent/send_event)
  - The example code is written in C which sends an ONVIF event periodically.
- [subscribe\_to\_event](https://github.com/AxisCommunications/acap-native-sdk-examples/tree/main/axevent/subscribe_to_event)
  - The example code is written in C which subscribe to the ONVIF event sent from application "send\_event".
- [subscribe\_to\_events](https://github.com/AxisCommunications/acap-native-sdk-examples/tree/main/axevent/subscribe_to_events)
  - The example code is written in C which subscribes to different predefined events.

* * *

### License Key API [​](https://developer.axis.com/acap/api/\#license-key-api "Direct link to License Key API")

Go to the [ACAP API Documentation](https://developer.axis.com/acap/api/src/api/licensekey/html/index.html) for detailed functional descriptions of this API.

Use the License Key API to validate an application license key.

A license key is a signed file, generated for a specific device ID and application ID. The ACAP Service Portal maintains both license keys and application IDs.

#### Compatibility [​](https://developer.axis.com/acap/api/\#compatibility-license-key-api "Direct link to Compatibility")

The API is supported on products with the following chips:

- ARTPEC-9
- ARTPEC-8
- ARTPEC-7
- ARTPEC-6
- Ambarella CV25
- i.MX 6SoloX

#### Version history [​](https://developer.axis.com/acap/api/\#version-history-license-key-api "Direct link to Version history")

This API was introduced in Native SDK 1.0.

#### Code examples [​](https://developer.axis.com/acap/api/\#code-examples-license-key-api "Direct link to Code examples")

- [licensekey](https://github.com/AxisCommunications/acap-native-sdk-examples/tree/main/licensekey/)
  - The example code is written in C which illustrates how to check the licensekey status.

* * *

### Machine learning API (Larod) [​](https://developer.axis.com/acap/api/\#machine-learning-api-larod "Direct link to Machine learning API (Larod)")

Go to the [ACAP API Documentation](https://developer.axis.com/acap/api/src/api/larod/html/index.html) for detailed functional descriptions of this API.

Larod is a service provides a simple unified C API for running machine learning and image preprocessing efficiently. The purpose of Larod is to provide a unified API for all hardware platforms with very little overhead and to arbitrate between different processes (apps) requesting access to the same hardware.

#### Compatibility [​](https://developer.axis.com/acap/api/\#compatibility-machine-learning-api "Direct link to Compatibility")

The Larod API is supported on products with the following chips:

- ARTPEC-9
- ARTPEC-8
- ARTPEC-7
- Ambarella CV25

For products with a DLPU (Deep Learning Processing Unit), inference runs on the DLPU otherwise it runs on the CPU.

#### Version history [​](https://developer.axis.com/acap/api/\#version-history-machine-learning-api "Direct link to Version history")

The Machine learning API was introduced in Native SDK 1.0. All larod API versions are available.

#### Code examples [​](https://developer.axis.com/acap/api/\#code-examples-machine-learning-api "Direct link to Code examples")

- [vdo-larod](https://github.com/AxisCommunications/acap-native-sdk-examples/tree/main/vdo-larod/)
  - The example code is written in C and loads an image classification model to the [Machine learning API (Larod)](https://developer.axis.com/acap/api/#machine-learning-api-larod) and then uses the [Video capture API (VDO)](https://developer.axis.com/acap/api/#video-capture-api-vdo) to fetch frames of size WIDTH x HEIGHT in YUV format which are converted to interleaved RGB format and then sent to larod for inference on MODEL.
- [object-detection](https://github.com/AxisCommunications/acap-native-sdk-examples/tree/main/object-detection/)
  - The example code focus on object detection, cropping and saving detected objects into JPEG files.
  - A separate example is available for [CV25](https://github.com/AxisCommunications/acap-native-sdk-examples/tree/main/object-detection-cv25) cameras.

* * *

### Message Broker API [​](https://developer.axis.com/acap/api/\#message-broker-api "Direct link to Message Broker API")

info

This API is a [Beta version](https://developer.axis.com/acap/api/#beta-apis) and developers are encouraged to test and leave feedback.

info

This API was formerly known as Metadata Broker.

Go to the [ACAP API Documentation](https://developer.axis.com/acap/api/src/api/message-broker/html/index.html) for detailed
functional descriptions of this API.

The Message Broker API allows an ACAP application to consume metadata from
a producer in AXIS OS by subscribing to a `topic`.

The API implements the Publish/Subscribe messaging paradigm.

#### Compatibility [​](https://developer.axis.com/acap/api/\#compatibility-message-broker-api "Direct link to Compatibility")

The Message Broker API is supported on products with the following chips:

- ARTPEC-9
- ARTPEC-8
- ARTPEC-7
- Ambarella CV25

#### Version history [​](https://developer.axis.com/acap/api/\#version-history-message-broker-api "Direct link to Version history")

The Message Broker API was introduced in Native SDK 1.13.

| AXIS OS version | Message Broker API version | New functions added |
| --- | --- | --- |
| 11.9 | 0.23.2 | New topic `com.axis.analytics_scene_description.v0.beta` |
| 11.11 | 0.26.5 | New topic `com.axis.consolidated_track.v1.beta` |

#### Code examples [​](https://developer.axis.com/acap/api/\#code-examples-message-broker-api "Direct link to Code examples")

- [consume-scene-metadata](https://github.com/AxisCommunications/acap-native-sdk-examples/tree/main/message-broker/consume-scene-metadata)
  - The example is written in C and shows how to consume metadata stream
    **Analytics Scene Description**.

* * *

### Overlay APIs [​](https://developer.axis.com/acap/api/\#overlay-apis "Direct link to Overlay APIs")

Axis offers different Overlay APIs available for different chips and with
different use cases.

### Axoverlay API [​](https://developer.axis.com/acap/api/\#axoverlay-api "Direct link to Axoverlay API")

Go to the [ACAP API Documentation](https://developer.axis.com/acap/api/src/api/axoverlay/html/index.html) for detailed functional descriptions of this API.

The Axoverlay API is a helper library that enables an ACAP to draw overlays in selected video streams. It has built-in support for Cairo as rendering API, as well as an open backend for any other custom rendering.

#### Compatibility [​](https://developer.axis.com/acap/api/\#compatibility-axoverlay-api "Direct link to Compatibility")

The API is supported on products with the following chips:

- ARTPEC-9
- ARTPEC-8
- ARTPEC-7
- ARTPEC-6

#### Version history [​](https://developer.axis.com/acap/api/\#version-history-axoverlay-api "Direct link to Version history")

The Axoverlay API was introduced in Native SDK 1.0.

#### Code examples [​](https://developer.axis.com/acap/api/\#code-examples-axoverlay-api "Direct link to Code examples")

- [axoverlay](https://github.com/AxisCommunications/acap-native-sdk-examples/tree/main/axoverlay/)
  - The example code is written in C which illustrates how to draw plain boxes and text as overlays in a stream.

### Bounding Box API [​](https://developer.axis.com/acap/api/\#bounding-box-api "Direct link to Bounding Box API")

Go to the [ACAP API Documentation](https://developer.axis.com/acap/api/src/api/bbox/html/index.html) for detailed functional descriptions of this API.

The Bounding Box API allows for the simple drawing of boxes on a scene. It supports all new chips and utilizes the most optimized drawing mechanisms available for each chip.
This API is recommended for adding basic box overlays. For more advanced features, refer to the Axoverlay API.

#### Compatibility [​](https://developer.axis.com/acap/api/\#compatibility-bounding-box-api "Direct link to Compatibility")

The API is supported on products with the following chips:

- ARTPEC-9
- ARTPEC-8
- ARTPEC-7
- Ambarella CV25

#### Version history [​](https://developer.axis.com/acap/api/\#version-history-bounding-box-api "Direct link to Version history")

The Bounding Box API was introduced in Native SDK 1.15.

#### Code examples [​](https://developer.axis.com/acap/api/\#code-examples-bounding-box-api "Direct link to Code examples")

- [bounding-box](https://github.com/AxisCommunications/acap-native-sdk-examples/tree/main/bounding-box)
  - An example in C that demonstrates how to portably draw burnt-in bounding boxes on selected video sources or channels.

* * *

### Parameter API [​](https://developer.axis.com/acap/api/\#parameter-api "Direct link to Parameter API")

Go to the [ACAP API Documentation](https://developer.axis.com/acap/api/src/api/axparameter/html/index.html) for
detailed functional descriptions of this API.

The AXParameter C library provides the following functionality:

- Read and modify **application parameters** stated in `manifest.json`.
- Add and remove **application parameters** in C code, in addition to the ones
already defined in `manifest.json`.
- Set up **callbacks** so the application can act immediately on changes made
to the **application parameters** via the
[application settings web page](https://developer.axis.com/acap/api/#application-settings-web-page) or VAPIX.
- Read **system parameters**.

**Application parameters** have the following properties:

- They are preserved when an application is restarted or upgraded.
- They are preserved when the device is restarted and when firmware is upgraded.
- They are displayed and possible to set in the
[application settings web page](https://developer.axis.com/acap/api/#application-settings-web-page).
- They can be read and modified using VAPIX API
[param.cgi](https://www.axis.com/vapix-library/subjects/t10175981/section/t10036014/display).

warning

The application parameters are **not private** to the application, they can
be:

- _read_ via VAPIX by a user with `operator` privileges.
- _read_ and _modified_ via VAPIX by a user with `admin` privileges.
- _read_ and _modified_ by another application if the application users belongs
to the same group.

#### Application settings web page [​](https://developer.axis.com/acap/api/\#application-settings-web-page "Direct link to Application settings web page")

The application settings web page can be used to modify **application**
**parameters**.

In the Axis device web page:

- Go to _Apps_ tab
- Open the application options
- Click _Settings_ to open a dialog where parameters can be set

Note that a reload of the web page is required to display values set from C code
or VAPIX.

#### Compatibility [​](https://developer.axis.com/acap/api/\#compatibility-parameter-api "Direct link to Compatibility")

The Parameter API is supported on products with the following chips:

- ARTPEC-9
- ARTPEC-8
- ARTPEC-7
- ARTPEC-6
- Ambarella CV25
- i.MX 6SoloX

#### Version history [​](https://developer.axis.com/acap/api/\#version-history-parameter-api "Direct link to Version history")

The Parameter API was introduced in Native SDK 1.13.

#### Code examples [​](https://developer.axis.com/acap/api/\#code-examples-parameter-api "Direct link to Code examples")

- [axparameter](https://github.com/AxisCommunications/acap-native-sdk-examples/tree/main/axparameter)
  - An example in C that demonstrates how to manage application-defined parameters, allowing you to add, remove, set, get, and register callback functions for parameter value updates.

* * *

### Serial port API [​](https://developer.axis.com/acap/api/\#serial-port-api "Direct link to Serial port API")

Go to the [ACAP API Documentation](https://developer.axis.com/acap/api/src/api/axserialport/html/index.html) for detailed functional descriptions of this API.

The Serial port API allows the application to configure and control the external serial port on selected Axis products.

#### Compatibility [​](https://developer.axis.com/acap/api/\#compatibility-serial-port-api "Direct link to Compatibility")

- The API supports the following standards:
  - RS-232
  - RS-422
  - RS-485
- The API is product dependent since not all Axis products are equipped with a serial port.
- The API is supported on products with the following chips:
  - ARTPEC-9
  - ARTPEC-8
  - ARTPEC-7

#### Version history [​](https://developer.axis.com/acap/api/\#version-history-serial-port-api "Direct link to Version history")

The Serial port API was introduced in Native SDK 1.11.

#### Code examples [​](https://developer.axis.com/acap/api/\#code-examples-serial-port-api "Direct link to Code examples")

- [axserialport](https://github.com/AxisCommunications/acap-native-sdk-examples/tree/main/axserialport)
  - This example details the creation of an ACAP application utilizing the axserialport API, showcasing the following actions:
    - Enabling the serial port.
    - Configuring parameters using the API.
    - Establishing communication between two available ports in the Axis product using GLib IOChannel methods.

* * *

### Video capture API (VDO) [​](https://developer.axis.com/acap/api/\#video-capture-api-vdo "Direct link to Video capture API (VDO)")

Go to the [ACAP API Documentation](https://developer.axis.com/acap/api/src/api/vdostream/html/index.html) for detailed functional descriptions of this API.

The VdoChannel API provides:

- query video channel types
- query supported video resolutions

The VdoStream API provides:

- video and image stream
- video and image capture
- video and image configuration

#### Available video compression formats through VDO [​](https://developer.axis.com/acap/api/\#available-video-compression-formats-through-vdo "Direct link to Available video compression formats through VDO")

The table below shows available subformats for corresponding YUV format.

| Sub Formats | Corresponding format |
| --- | --- |
| nv12 | YUV |
| y800 | YUV |

An application to start a vdo stream can be found at [vdostream](https://github.com/AxisCommunications/acap-native-sdk-examples/tree/main/vdostream/), where the first argument is a string describing the video compression format. It takes `h264` (default), `h265`, `jpeg`, `nv12`, and `y800` as inputs. Both `nv12` and `y800` correspond to YUV format of VDO.

#### Compatibility [​](https://developer.axis.com/acap/api/\#compatibility-video-capture-api "Direct link to Compatibility")

The API is supported on products with the following chips:

- ARTPEC-9
- ARTPEC-8
- ARTPEC-7
- ARTPEC-6
- Ambarella CV25

##### Compatibility limitations [​](https://developer.axis.com/acap/api/\#compatibility-limitations "Direct link to Compatibility limitations")

- **Global rotation** \- All platforms except ARTPEC-6 support global image source rotation, this feature is designed to utilize the hardware more efficiently. When designing applications for multiple platforms this needs to be taken into consideration. The Video capture API on ARTPEC-6 allows specifying the desired rotation per stream in contrast to other chips where this is a global option chosen at camera installation time and any attempt to specify capture rotation will be ignored.

#### Version history [​](https://developer.axis.com/acap/api/\#version-history-video-capture-api "Direct link to Version history")

The Video capture API was introduced in Native SDK 1.0.

| AXIS OS version | VdoStream API version | New functions added |
| --- | --- | --- |
| 10.7 | 6.13.0 | `vdo_frame_take_chunk` |
| 10.8 | 6.24.1 | `vdo_zipstream_profile_get_type`, `vdo_frame_set_header_size` |
| 10.9 | 6.34.16 | `vdo_stream_get_event`, `vdo_stream_get_event_fd`, `vdo_map_swap` |
| 10.10 | 7.5.22 | `vdo_frame_take_chunk`, `vdo_frame_take_chunk_ex`, `vdo_stream_play` |
| 10.11 | 7.19.2 | - |
| 10.12 | 8.0.15 | `vdo_map_get_pair32i`, `vdo_map_get_pair32u`, `vdo_map_set_pair32i`, `vdo_map_set_pair32u` |
| 11.0 | 9.0.4 | - |
| 11.1 | 9.8.16 | - |
| 11.2 | 9.18.0 | - |
| 11.3 | 9.32.0 | - |
| 11.4 | 9.41.16 | `vdo_channel_get_ex` |
| 11.5 | 9.59.0 | - |
| 11.6 | 9.78.12 | `vdo_stream_set_gop_length`, `vdo_error_is_resource_limitation` |

#### Known issues [​](https://developer.axis.com/acap/api/\#known-issues "Direct link to Known issues")

- A memory leak in VDO was found in firmware versions from 10.10 for ARTPEC chips, later fixed in 10.11.65. The issue affects function `vdo_buffer_get_data`.

#### Code examples [​](https://developer.axis.com/acap/api/\#code-examples-video-capture-api "Direct link to Code examples")

- [vdostream](https://github.com/AxisCommunications/acap-native-sdk-examples/tree/main/vdostream/)
  - The example code is written in C which starts a vdo stream and then illustrates how to continuously capture frames from the vdo service, access the received buffer contents as well as the frame metadata.
- [vdo-larod](https://github.com/AxisCommunications/acap-native-sdk-examples/tree/main/vdo-larod/)
  - The example code is written in C and loads an image classification model to the [Machine learning API (Larod)](https://developer.axis.com/acap/api/#machine-learning-api-larod) and then uses the [Video capture API (VDO)](https://developer.axis.com/acap/api/#video-capture-api-vdo) to fetch frames of size WIDTH x HEIGHT in YUV format which are converted to interleaved RGB format and then sent to larod for inference on MODEL.
- [vdo-opencl-filtering](https://github.com/AxisCommunications/acap-native-sdk-examples/tree/main/vdo-opencl-filtering/)
  - This example illustrates how to capture frames from the vdo service, access the received buffer, and finally perform a GPU accelerated Sobel filtering with OpenCL.

* * *

## Open Source APIs [​](https://developer.axis.com/acap/api/\#open-source-apis "Direct link to Open Source APIs")

### Pipewire [​](https://developer.axis.com/acap/api/\#pipewire "Direct link to Pipewire")

Open-source multimedia framework designed for handling audio and video streams with a focus on low latency and security. See [Pipewire documentation](https://docs.pipewire.org/).

#### Compatibility [​](https://developer.axis.com/acap/api/\#compatibility-pipewire "Direct link to Compatibility")

The API is supported on products with the following chips:

- ARTPEC-9
- ARTPEC-8
- ARTPEC-7
- ARTPEC-6
- Ambarella CV25
- i.MX 6SoloX

#### Version history [​](https://developer.axis.com/acap/api/\#version-history-pipewire "Direct link to Version history")

The Pipewire API was fully introduced in 12.5.

#### Code examples [​](https://developer.axis.com/acap/api/\#code-examples-pipewire "Direct link to Code examples")

- [audio-capture](https://github.com/AxisCommunications/acap-native-sdk-examples/tree/main/audio-capture/)
  - This example demonstrates how to use Pipewire as a library in ACAP applications to capture audio from an input node.
- [audio-playback](https://github.com/AxisCommunications/acap-native-sdk-examples/tree/main/audio-playback/)
  - This example demonstrates how to use Pipewire as a library in ACAP applications to play audio to an output node.

#### Node to Device Mapping [​](https://developer.axis.com/acap/api/\#node-to-device-mapping "Direct link to Node to Device Mapping")

This section explains how Pipewire node identifiers correlate to physical device components and audio/video inputs/outputs in the camera.

The target nodes names relate to the device id and input/output id of the [Audio Device Control API](https://developer.axis.com/vapix/audio-systems/audio-device-control/). The pattern is as follows:

```codeBlockLines_e6Vv
TargetNodeName   ::= "AudioDevice" DeviceId Direction ConnectionId [ "." OptionalSuffix ] ;
Direction        ::= "Input" | "Output" ;
DeviceId         ::= "0" | NonZeroDigit { Digit } ;
ConnectionId     ::= "0" | NonZeroDigit { Digit } ;
OptionalSuffix   ::= "Unprocessed" ;
Digit            ::= "0" | NonZeroDigit ;
NonZeroDigit     ::= "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9" ;

```

If you want to get the unprocessed audio from input 0 on device 0, the target node should be "AudioDevice0Input0.Unprocessed". If you want to get the processed audio, including e.g. equalizers and such, from input 1 on device 0, the target node should be "AudioDevice0Input1". If you want to play to output 0 of device 1, the target node should be "AudioDevice1Output0".

Top level input nodes, e.g. AudioDevice0Input0, are removed when the corresponding input is disabled in audiodevicecontrol.cgi. This does not apply to other nodes, such as AudioDevice0Input0.Unprocessed.

### Cairo [​](https://developer.axis.com/acap/api/\#cairo "Direct link to Cairo")

Open-source rendering library for 2D vector graphics. See [Cairo documentation](https://www.cairographics.org/documentation/).

#### Compatibility [​](https://developer.axis.com/acap/api/\#compatibility-cairo "Direct link to Compatibility")

The Cairo API is supported on products with the following chips:

- ARTPEC-9
- ARTPEC-8
- ARTPEC-7
- ARTPEC-6

#### Version history [​](https://developer.axis.com/acap/api/\#version-history-cairo "Direct link to Version history")

The Cairo API was introduced in Native SDK 1.0.

#### Code examples [​](https://developer.axis.com/acap/api/\#code-examples-cairo "Direct link to Code examples")

- [axoverlay](https://github.com/AxisCommunications/acap-native-sdk-examples/tree/main/axoverlay/)
  - The example code is written in C which illustrates how to draw plain boxes and text as overlays in a stream.

* * *

### OpenCL [​](https://developer.axis.com/acap/api/\#opencl "Direct link to OpenCL")

Accelerate parallel compute with GPU. See [OpenCL documentation](https://www.khronos.org/opencl/).

#### Compatibility [​](https://developer.axis.com/acap/api/\#compatibility-opencl "Direct link to Compatibility")

The OpenCL API is supported on products with the following chips:

- ARTPEC-9
- ARTPEC-8
- ARTPEC-7

#### Version history [​](https://developer.axis.com/acap/api/\#version-history-opencl "Direct link to Version history")

The OpenCL 1.2 was introduced in Native SDK 1.0.

#### Code examples [​](https://developer.axis.com/acap/api/\#code-examples-opencl "Direct link to Code examples")

- [vdo-opencl-filtering](https://github.com/AxisCommunications/acap-native-sdk-examples/tree/main/vdo-opencl-filtering/)
  - This example illustrates how to capture frames from the vdo service, access the received buffer, and finally perform a GPU accelerated Sobel filtering with OpenCL.

* * *

### FastCGI [​](https://developer.axis.com/acap/api/\#fastcgi "Direct link to FastCGI")

[FastCGI](https://wikipedia.org/wiki/FastCGI) is a protocol for interfacing interactive programs with a web server.

#### Compatibility [​](https://developer.axis.com/acap/api/\#compatibility-fastcgi "Direct link to Compatibility")

The FastCGI API is supported on products with the following chips:

- ARTPEC-9
- ARTPEC-8
- ARTPEC-7
- ARTPEC-6
- Ambarella CV25
- i.MX 6SoloX

#### Version history [​](https://developer.axis.com/acap/api/\#version-history-fastcgi "Direct link to Version history")

The FastCGI API was introduced in Native SDK 1.6.

#### Code examples [​](https://developer.axis.com/acap/api/\#code-examples-fastcgi "Direct link to Code examples")

- [web-server-using-fastcgi](https://github.com/AxisCommunications/acap-native-sdk-examples/tree/main/web-server-using-fastcgi)
  - This example is written in C and explains how to build an ACAP application that can handle HTTP requests sent to the Axis device, using the device's own web server and FastCGI.

* * *

### OpenSSL [​](https://developer.axis.com/acap/api/\#openssl "Direct link to OpenSSL")

Open-source library for general-purpose cryptography and secure communication. See [OpenSSL documentation](https://www.openssl.org/docs/).

#### Compatibility [​](https://developer.axis.com/acap/api/\#compatibility-openssl "Direct link to Compatibility")

The OpenSSL API is supported on products with the following chips:

- ARTPEC-9
- ARTPEC-8
- ARTPEC-7
- ARTPEC-6
- Ambarella CV25
- i.MX 6SoloX

#### Version history [​](https://developer.axis.com/acap/api/\#version-history-openssl "Direct link to Version history")

The OpenSSL API was introduced in Native SDK 1.14.

#### Code examples [​](https://developer.axis.com/acap/api/\#code-examples-openssl "Direct link to Code examples")

- [curl-openssl](https://github.com/AxisCommunications/acap-native-sdk-examples/tree/main/curl-openssl/)
  - The example code is written in C which illustrates how to use curl and OpenSSL to retrieve a file securely from an external server for e.g. [example.com](https://www.example.com/)

* * *

### Jansson [​](https://developer.axis.com/acap/api/\#jansson "Direct link to Jansson")

Open-source library for for encoding, decoding and manipulating JSON data. See [Jansson documentation](https://jansson.readthedocs.io/en/latest/).

#### Compatibility [​](https://developer.axis.com/acap/api/\#compatibility-jansson "Direct link to Compatibility")

The Jansson API is supported on products with the following chips:

- ARTPEC-9
- ARTPEC-8
- ARTPEC-7
- ARTPEC-6
- Ambarella CV25
- i.MX 6SoloX

#### Version history [​](https://developer.axis.com/acap/api/\#version-history-jansson "Direct link to Version history")

The Jansson API was introduced in Native SDK 1.14.

#### Code examples [​](https://developer.axis.com/acap/api/\#code-examples-jansson "Direct link to Code examples")

- [vapix](https://github.com/AxisCommunications/acap-native-sdk-examples/tree/main/vapix)
  - The example code is written in C which illustrates calling VAPIX API with JSON request and response from within an ACAP application.

* * *

### Curl [​](https://developer.axis.com/acap/api/\#curl "Direct link to Curl")

Open-source library for transferring data with URLs. See [curl documentation](https://curl.se/docs/). For proxy configuration see [Global device proxy](https://developer.axis.com/acap/develop/proxy/).

#### Compatibility [​](https://developer.axis.com/acap/api/\#compatibility-curl "Direct link to Compatibility")

The curl API is supported on products with the following chips:

- ARTPEC-9
- ARTPEC-8
- ARTPEC-7
- ARTPEC-6
- Ambarella CV25
- i.MX 6SoloX

#### Version history [​](https://developer.axis.com/acap/api/\#version-history-curl "Direct link to Version history")

The curl API was introduced in Native SDK 1.14.

#### Code examples [​](https://developer.axis.com/acap/api/\#code-examples-curl "Direct link to Code examples")

- [curl-openssl](https://github.com/AxisCommunications/acap-native-sdk-examples/tree/main/curl-openssl/)
  - The example code is written in C which illustrates how to use curl and OpenSSL to retrieve a file securely from an external server for e.g. [example.com](https://www.example.com/)
- [vapix](https://github.com/AxisCommunications/acap-native-sdk-examples/tree/main/vapix)
  - The example code is written in C which illustrates how ,using curl, an ACAP application can call [VAPIX APIs](https://www.axis.com/vapix-library/).

* * *

## Supplementary APIs [​](https://developer.axis.com/acap/api/\#supplementary-apis "Direct link to Supplementary APIs")

### VAPIX access for ACAP [​](https://developer.axis.com/acap/api/\#vapix-access-for-acap "Direct link to VAPIX access for ACAP")

ACAP applications can request for VAPIX service account credentials with "admin" access level in runtime. With these credentials, the ACAP application can call a local virtual host to make VAPIX requests on the device. See [VAPIX access for ACAP](https://developer.axis.com/acap/develop/VAPIX-access-for-ACAP-applications/).

#### Code examples [​](https://developer.axis.com/acap/api/\#code-examples-vapix "Direct link to Code examples")

- [vapix](https://github.com/AxisCommunications/acap-native-sdk-examples/tree/main/vapix)
  - The example code is written in C which illustrates how an ACAP application can call [VAPIX APIs](https://www.axis.com/vapix-library/).

Last updated on **Oct 13, 2025**

[Previous\\
\\
Debugging](https://developer.axis.com/acap/develop/debugging/) [Next\\
\\
API compatibility guide](https://developer.axis.com/acap/api/api_compatibility_guide/)

- [Beta APIs](https://developer.axis.com/acap/api/#beta-apis)
- [Axis APIs](https://developer.axis.com/acap/api/#axis-apis)
  - [Edge storage API](https://developer.axis.com/acap/api/#edge-storage-api)
  - [Event API](https://developer.axis.com/acap/api/#event-api)
  - [License Key API](https://developer.axis.com/acap/api/#license-key-api)
  - [Machine learning API (Larod)](https://developer.axis.com/acap/api/#machine-learning-api-larod)
  - [Message Broker API](https://developer.axis.com/acap/api/#message-broker-api)
  - [Overlay APIs](https://developer.axis.com/acap/api/#overlay-apis)
  - [Axoverlay API](https://developer.axis.com/acap/api/#axoverlay-api)
  - [Bounding Box API](https://developer.axis.com/acap/api/#bounding-box-api)
  - [Parameter API](https://developer.axis.com/acap/api/#parameter-api)
  - [Serial port API](https://developer.axis.com/acap/api/#serial-port-api)
  - [Video capture API (VDO)](https://developer.axis.com/acap/api/#video-capture-api-vdo)
- [Open Source APIs](https://developer.axis.com/acap/api/#open-source-apis)
  - [Pipewire](https://developer.axis.com/acap/api/#pipewire)
  - [Cairo](https://developer.axis.com/acap/api/#cairo)
  - [OpenCL](https://developer.axis.com/acap/api/#opencl)
  - [FastCGI](https://developer.axis.com/acap/api/#fastcgi)
  - [OpenSSL](https://developer.axis.com/acap/api/#openssl)
  - [Jansson](https://developer.axis.com/acap/api/#jansson)
  - [Curl](https://developer.axis.com/acap/api/#curl)
- [Supplementary APIs](https://developer.axis.com/acap/api/#supplementary-apis)
  - [VAPIX access for ACAP](https://developer.axis.com/acap/api/#vapix-access-for-acap)

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