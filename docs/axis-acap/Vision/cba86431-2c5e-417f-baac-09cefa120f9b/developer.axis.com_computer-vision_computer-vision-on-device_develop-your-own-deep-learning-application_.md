---
url: "https://developer.axis.com/computer-vision/computer-vision-on-device/develop-your-own-deep-learning-application/"
title: "Develop your own deep learning application | Axis developer documentation"
---

[Skip to main content](https://developer.axis.com/computer-vision/computer-vision-on-device/develop-your-own-deep-learning-application/#__docusaurus_skipToContent_fallback)

[![Axis Communications Logo](https://developer.axis.com/img/axis-logo.svg)\\
**Axis developer documentation**](https://developer.axis.com/)

```

```

[Blog](https://developer.axis.com/blog/)

- [Computer vision](https://developer.axis.com/computer-vision/)
- [Computer vision on device](https://developer.axis.com/computer-vision/computer-vision-on-device/develop-your-own-deep-learning-application/#)

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
- [How-to guides](https://developer.axis.com/computer-vision/computer-vision-on-device/develop-your-own-deep-learning-application/#)


- [Home page](https://developer.axis.com/)
- Computer vision on device
- Develop your own deep learning application

On this page

# Develop your own deep learning application

The **Axis Camera Application Platform (ACAP)** is the native development framework for deploying applications on Axis cameras. ACAP provides access to camera resources, including the **image pipeline, processing power, and network interfaces**, enabling seamless integration of **deep learning models** with on-device AI processing.

## Workflow to deploy a deep learning model [​](https://developer.axis.com/computer-vision/computer-vision-on-device/develop-your-own-deep-learning-application/\#workflow-to-deploy-a-deep-learning-model "Direct link to Workflow to deploy a deep learning model")

### 1\. Train a model [​](https://developer.axis.com/computer-vision/computer-vision-on-device/develop-your-own-deep-learning-application/\#1-train-a-model "Direct link to 1. Train a model")

- Use the standard deep learning framework **TensorFlow** to train your model.
- Choose a model architecture that is optimized for **Axis DLPU**. Refer to:

  - [Recommended Model Architectures](https://developer.axis.com/computer-vision/computer-vision-on-device/recommended-model-architecture/)
  - [Axis Model Zoo](https://github.com/AxisCommunications/axis-model-zoo) for pre-trained models and training scripts.

### 2\. Convert the model for deployment [​](https://developer.axis.com/computer-vision/computer-vision-on-device/develop-your-own-deep-learning-application/\#2-convert-the-model-for-deployment "Direct link to 2. Convert the model for deployment")

- Axis cameras of different system-on-chip (SoC) have different **DLPU**. Ensure compatibility by following the guidelines for model conversion:

  - [DLPU Model Conversion Guide](https://developer.axis.com/computer-vision/computer-vision-on-device/dlpu-model-conversion/)
- Optimize the model by using techniques such as **quantization and pruning** to ensure efficient execution on the camera.

### 3\. Integrate the model with an ACAP application [​](https://developer.axis.com/computer-vision/computer-vision-on-device/develop-your-own-deep-learning-application/\#3-integrate-the-model-with-an-acap-application "Direct link to 3. Integrate the model with an ACAP application")

- Develop a **custom ACAP application** to manage video input, run inference, and handle outputs.
- Use ACAP APIs to access **image data, event handling, and network communication**.
- See [ACAP SDK Examples](https://github.com/AxisCommunications/acap-native-sdk-examples) for sample implementations of ACAP applications.

### 4\. Deploy and run on the camera [​](https://developer.axis.com/computer-vision/computer-vision-on-device/develop-your-own-deep-learning-application/\#4-deploy-and-run-on-the-camera "Direct link to 4. Deploy and run on the camera")

- Package the application and deploy it directly onto the Axis camera.
- Run real-time inference with minimal latency, enabling **immediate event triggers, metadata generation, or integration with external systems**.

## Get started with ACAP and deep learning [​](https://developer.axis.com/computer-vision/computer-vision-on-device/develop-your-own-deep-learning-application/\#get-started-with-acap-and-deep-learning "Direct link to Get started with ACAP and deep learning")

To get started, you can explore the examples of deep learning applications available in our GitHub repositories. The [ACAP SDK Examples](https://github.com/AxisCommunications/acap-native-sdk-examples) repository contains examples to run deep learning applications directly on the Axis device.

Examples such as [vdo-larod](https://github.com/AxisCommunications/acap-native-sdk-examples/tree/main/vdo-larod) and [object-detection](https://github.com/AxisCommunications/acap-native-sdk-examples/tree/main/object-detection) demonstrate the usage of our Machine Learning API also called Larod. Larod is a C API that enables you to run deep learning models in the TensorFlow Lite format.

Before diving into the development of your application, we recommend that you continue reading the rest of this documentation to understand the advantages and limitations of deploying a deep learning model on an Axis device.

Last updated on **Mar 25, 2025**

[Previous\\
\\
Computer vision](https://developer.axis.com/computer-vision/) [Next\\
\\
Axis Deep Learning Processing Unit (DLPU)](https://developer.axis.com/computer-vision/computer-vision-on-device/axis-dlpu/)

- [Workflow to deploy a deep learning model](https://developer.axis.com/computer-vision/computer-vision-on-device/develop-your-own-deep-learning-application/#workflow-to-deploy-a-deep-learning-model)
  - [1\. Train a model](https://developer.axis.com/computer-vision/computer-vision-on-device/develop-your-own-deep-learning-application/#1-train-a-model)
  - [2\. Convert the model for deployment](https://developer.axis.com/computer-vision/computer-vision-on-device/develop-your-own-deep-learning-application/#2-convert-the-model-for-deployment)
  - [3\. Integrate the model with an ACAP application](https://developer.axis.com/computer-vision/computer-vision-on-device/develop-your-own-deep-learning-application/#3-integrate-the-model-with-an-acap-application)
  - [4\. Deploy and run on the camera](https://developer.axis.com/computer-vision/computer-vision-on-device/develop-your-own-deep-learning-application/#4-deploy-and-run-on-the-camera)
- [Get started with ACAP and deep learning](https://developer.axis.com/computer-vision/computer-vision-on-device/develop-your-own-deep-learning-application/#get-started-with-acap-and-deep-learning)

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