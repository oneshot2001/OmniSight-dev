---
url: "https://developer.axis.com/computer-vision/computer-vision-on-device/recommended-model-architecture/"
title: "Recommended model architecture | Axis developer documentation"
---

[Skip to main content](https://developer.axis.com/computer-vision/computer-vision-on-device/recommended-model-architecture/#__docusaurus_skipToContent_fallback)

[![Axis Communications Logo](https://developer.axis.com/img/axis-logo.svg)\\
**Axis developer documentation**](https://developer.axis.com/)

```

```

[Blog](https://developer.axis.com/blog/)

- [Computer vision](https://developer.axis.com/computer-vision/)
- [Computer vision on device](https://developer.axis.com/computer-vision/computer-vision-on-device/recommended-model-architecture/#)

  - [Develop your own deep learning application](https://developer.axis.com/computer-vision/computer-vision-on-device/develop-your-own-deep-learning-application/)
  - [Axis Deep Learning Processing Unit (DLPU)](https://developer.axis.com/computer-vision/computer-vision-on-device/axis-dlpu/)
  - [Supported frameworks](https://developer.axis.com/computer-vision/computer-vision-on-device/supported-frameworks/)
  - [Quantization](https://developer.axis.com/computer-vision/computer-vision-on-device/quantization/)
  - [Deep Learning Processing Unit (DLPU) model conversion](https://developer.axis.com/computer-vision/computer-vision-on-device/dlpu-model-conversion/)
  - [General suggestions](https://developer.axis.com/computer-vision/computer-vision-on-device/general-suggestions/)
  - [Recommended model architecture](https://developer.axis.com/computer-vision/computer-vision-on-device/recommended-model-architecture/)
  - [Optimization tips](https://developer.axis.com/computer-vision/computer-vision-on-device/optimization-tips/)
  - [Test your model](https://developer.axis.com/computer-vision/computer-vision-on-device/test-your-model/)
  - [Glossary](https://developer.axis.com/computer-vision/computer-vision-on-device/glossary/)
- [How-to guides](https://developer.axis.com/computer-vision/computer-vision-on-device/recommended-model-architecture/#)


- [Home page](https://developer.axis.com/)
- Computer vision on device
- Recommended model architecture

On this page

# Recommended model architecture

When working on image classification, object detection, segmentation, or any other problem, it is important to understand the components of a model architecture: the backbone and the head. The backbone extracts features from the input image, while the head is responsible for the specific task.

When choosing a model architecture, consider the Deep Learning Processing Unit (DLPU) you plan to deploy on. The backbone should be optimized for that DLPU, while the head can be switched based on the task at hand.

For example, the `SSD MobileNet` architecture has a MobileNet backbone and an SSD head. You can use the same backbone with a different head for a different task.

## General considerations [​](https://developer.axis.com/computer-vision/computer-vision-on-device/recommended-model-architecture/\#general-considerations "Direct link to General considerations")

The models suggested in this page are the best trade off between easy of use, accuracy and speed on an Axis device. They are meant to allow real time inference on the edge, if your application requires higher accuracy than what the standard versions of the model can provide, the best approach is to change the hyperparameters of these same models to scale them up. For example, you can increase the input size of the model, or the number of layers in the backbone. This will allow you to get a more accurate model, but it will also increase the inference time. Be aware that changing some of the hyperparameters of the model will require you to retrain the model from scratch.

## ARTPEC-7 [​](https://developer.axis.com/computer-vision/computer-vision-on-device/recommended-model-architecture/\#artpec-7 "Direct link to ARTPEC-7")

The ARTPEC-7 DLPU is based on the EdgeTPU, so you can use any model architecture supported by the EdgeTPU. Check out the [EdgeTPU image classification models](https://coral.ai/models/image-classification/) and [EdgeTPU object detection models](https://coral.ai/models/object-detection/) for a list of supported models. We recommend using `MobileNet v2` as the backbone.

## ARTPEC-8 [​](https://developer.axis.com/computer-vision/computer-vision-on-device/recommended-model-architecture/\#artpec-8 "Direct link to ARTPEC-8")

For ARTPEC-8, we recommend models with `MobileNet v2` as the backbone. For object detection, you can consider using `SSD MobileNet v2` or `SSDLite MobileDet`. These models can be trained using the [TensorFlow Object Detection API](https://github.com/tensorflow/models/blob/master/research/object_detection/g3doc/tf1.md). `Resnet-18` is a better backbone than `MobileNet v2` for ARTPEC-8, it is a good idea to switch to it if you aim for better performance, however there are no ready to use configuration for that backbone in the TensorFlow Object Detection API.
Please note that TensorFlow 1.15 is recommended since its default quantization (per-tensor) is optimized for ARTPEC-8.

Another option for ARTPEC-8 is to use YOLOv5. Refer to the [YOLOv5 guide](https://github.com/AxisCommunications/axis-model-zoo/blob/main/docs/yolov5.md) tutorial for more information on training YOLOv5 for ARTPEC-8.

## ARTPEC-9 [​](https://developer.axis.com/computer-vision/computer-vision-on-device/recommended-model-architecture/\#artpec-9 "Direct link to ARTPEC-9")

For ARTPEC-9, we recommend models with `MobileNet v2` as the backbone. For object detection, you can consider using `SSD MobileNet v2` or `SSDLite MobileDet`. These models can be trained using the [TensorFlow Object Detection API](https://github.com/tensorflow/models/blob/master/research/object_detection/g3doc/tf1.md).

Another option for ARTPEC-9 is to use YOLOv5, refer to the following [guide](https://github.com/AxisCommunications/axis-model-zoo/blob/main/docs/yolov5.md) for more information.

## CV25 [​](https://developer.axis.com/computer-vision/computer-vision-on-device/recommended-model-architecture/\#cv25 "Direct link to CV25")

For CV25, we also recommend models with MobileNet v2 as the backbone. `SSD MobileNet v2` and `SSDLite MobileDet` are good choices for object detection. These models can be trained using the [TensorFlow Object Detection API](https://github.com/tensorflow/models/blob/master/research/object_detection/g3doc/tf1.md).

The Ambarella toolchain documentation provides additional examples and guides on training and optimizing models for the CV25 DLPU.

Last updated on **Oct 10, 2025**

[Previous\\
\\
General suggestions](https://developer.axis.com/computer-vision/computer-vision-on-device/general-suggestions/) [Next\\
\\
Optimization tips](https://developer.axis.com/computer-vision/computer-vision-on-device/optimization-tips/)

- [General considerations](https://developer.axis.com/computer-vision/computer-vision-on-device/recommended-model-architecture/#general-considerations)
- [ARTPEC-7](https://developer.axis.com/computer-vision/computer-vision-on-device/recommended-model-architecture/#artpec-7)
- [ARTPEC-8](https://developer.axis.com/computer-vision/computer-vision-on-device/recommended-model-architecture/#artpec-8)
- [ARTPEC-9](https://developer.axis.com/computer-vision/computer-vision-on-device/recommended-model-architecture/#artpec-9)
- [CV25](https://developer.axis.com/computer-vision/computer-vision-on-device/recommended-model-architecture/#cv25)

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