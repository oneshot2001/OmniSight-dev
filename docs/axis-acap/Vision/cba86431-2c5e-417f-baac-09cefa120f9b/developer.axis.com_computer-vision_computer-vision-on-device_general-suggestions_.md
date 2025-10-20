---
url: "https://developer.axis.com/computer-vision/computer-vision-on-device/general-suggestions/"
title: "General suggestions | Axis developer documentation"
---

[Skip to main content](https://developer.axis.com/computer-vision/computer-vision-on-device/general-suggestions/#__docusaurus_skipToContent_fallback)

[![Axis Communications Logo](https://developer.axis.com/img/axis-logo.svg)\\
**Axis developer documentation**](https://developer.axis.com/)

```

```

[Blog](https://developer.axis.com/blog/)

- [Computer vision](https://developer.axis.com/computer-vision/)
- [Computer vision on device](https://developer.axis.com/computer-vision/computer-vision-on-device/general-suggestions/#)

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
- [How-to guides](https://developer.axis.com/computer-vision/computer-vision-on-device/general-suggestions/#)


- [Home page](https://developer.axis.com/)
- Computer vision on device
- General suggestions

On this page

# General suggestions

## CPU vs Deep Learning Processing Unit [​](https://developer.axis.com/computer-vision/computer-vision-on-device/general-suggestions/\#cpu-vs-deep-learning-processing-unit "Direct link to CPU vs Deep Learning Processing Unit")

Using the Deep Learning Processing Unit (DLPU) will speed up the inference time, but it comes at an extra development cost, as you need to adapt the model to the DLPU. Depending on the DLPU available in your device and the model you are trying to use, the conversion overhead can vary. If you need to run inference infrequently, and your CPU is not under other loads, and you don't have latency constraints, it might be easier to just use the CPU.

## Accuracy of your application [​](https://developer.axis.com/computer-vision/computer-vision-on-device/general-suggestions/\#accuracy-of-your-application "Direct link to Accuracy of your application")

The overall accuracy of your application is determined by three factors: the model architecture, the quality of data used in training, and the training pipeline. When running a model on an edge device like an Axis camera, you are constrained in the type of model architecture you can use. Therefore, it becomes even more important to have a good dataset and a good training pipeline. It is crucial to have a training dataset that contains images similar to the ones your application will encounter. Adding Axis camera frames to the dataset is the best way to improve the performance of your model, on the other hand, having only camera frame from the same static scene in your dataset will reduce the generalizability of your model. Having a good training pipeline will also improve the quality of the final model. Adding features like data augmentation, custom loss function, regularization, early stopping, and learning rate decay will help you get a better model.

## Picking the model [​](https://developer.axis.com/computer-vision/computer-vision-on-device/general-suggestions/\#picking-the-model "Direct link to Picking the model")

If you are not an expert, we recommend using only the models provided in the [recommended model architecture](https://developer.axis.com/computer-vision/computer-vision-on-device/recommended-model-architecture/) section of this documentation or in the [Axis Model Zoo](https://github.com/AxisCommunications/axis-model-zoo). Picking a model from another repository might require a lot of work to adapt it to the DLPU, and it is probably not worth the effort, unless you know what you are doing.

## Use standard architectures [​](https://developer.axis.com/computer-vision/computer-vision-on-device/general-suggestions/\#use-standard-architectures "Direct link to Use standard architectures")

When running Deep Learning models on the edge, you should not expect the same flexibility and performance as when you run your model on the cloud on a GPU. Therefore, you should not expect to be able to run the latest cutting-edge model architecture on your Axis device. Visual transformers and deformable convolutions are not recommended at the moment. Stick to more traditional architectures like `ResNet`, `MobileNet`, and `EfficientNet`. These architectures are well supported by the DLPU and are known to perform well on edge devices.

## Use simple layers [​](https://developer.axis.com/computer-vision/computer-vision-on-device/general-suggestions/\#use-simple-layers "Direct link to Use simple layers")

Most DLPUs (Deep Learning Processing Units) are optimized to accelerate a small set of layers. If your network is composed of simple, well-supported layers such as `Conv2D`, `Add` and `ReLU`, you will encounter fewer compatibility issues and higher performance.
You can visualize your model's architecture using a tool like [Netron](https://netron.app/). For inspiration on layers that work well, you can explore the [Axis Model Zoo](https://github.com/AxisCommunications/axis-model-zoo) and refer to the [optimization tips](https://developer.axis.com/computer-vision/computer-vision-on-device/optimization-tips/) page.

Last updated on **Aug 28, 2024**

[Previous\\
\\
Deep Learning Processing Unit (DLPU) model conversion](https://developer.axis.com/computer-vision/computer-vision-on-device/dlpu-model-conversion/) [Next\\
\\
Recommended model architecture](https://developer.axis.com/computer-vision/computer-vision-on-device/recommended-model-architecture/)

- [CPU vs Deep Learning Processing Unit](https://developer.axis.com/computer-vision/computer-vision-on-device/general-suggestions/#cpu-vs-deep-learning-processing-unit)
- [Accuracy of your application](https://developer.axis.com/computer-vision/computer-vision-on-device/general-suggestions/#accuracy-of-your-application)
- [Picking the model](https://developer.axis.com/computer-vision/computer-vision-on-device/general-suggestions/#picking-the-model)
- [Use standard architectures](https://developer.axis.com/computer-vision/computer-vision-on-device/general-suggestions/#use-standard-architectures)
- [Use simple layers](https://developer.axis.com/computer-vision/computer-vision-on-device/general-suggestions/#use-simple-layers)

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