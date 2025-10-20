---
url: "https://developer.axis.com/computer-vision/computer-vision-on-device/dlpu-model-conversion/"
title: "Deep Learning Processing Unit (DLPU) model conversion | Axis developer documentation"
---

[Skip to main content](https://developer.axis.com/computer-vision/computer-vision-on-device/dlpu-model-conversion/#__docusaurus_skipToContent_fallback)

[![Axis Communications Logo](https://developer.axis.com/img/axis-logo.svg)\\
**Axis developer documentation**](https://developer.axis.com/)

```

```

[Blog](https://developer.axis.com/blog/)

- [Computer vision](https://developer.axis.com/computer-vision/)
- [Computer vision on device](https://developer.axis.com/computer-vision/computer-vision-on-device/dlpu-model-conversion/#)

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
- [How-to guides](https://developer.axis.com/computer-vision/computer-vision-on-device/dlpu-model-conversion/#)


- [Home page](https://developer.axis.com/)
- Computer vision on device
- Deep Learning Processing Unit (DLPU) model conversion

On this page

# Deep Learning Processing Unit (DLPU) model conversion

To convert your model for the Deep Learning Processing Unit (DLPU) in your device, you need to follow a specific procedure depending on the DLPU available. Here is a schematic that shows the different steps required for each DLPU:

![DLPU Conversion Schematic](https://developer.axis.com/assets/images/dlpu-model-conversion.schematic-c6cf48ab3513c700cc749258c7b0628a.png)

## ARTPEC-7 [​](https://developer.axis.com/computer-vision/computer-vision-on-device/dlpu-model-conversion/\#artpec-7 "Direct link to ARTPEC-7")

To convert a model for the ARTPEC-7 DLPU, follow these steps:

1. Train a model using TensorFlow.
2. Quantize and export the saved model to TensorFlow Lite (tflite) using the TensorFlow Lite converter.
3. Compile the model using the EdgeTPU compiler.

Since the ARTPEC-7 DLPU is based on the EdgeTPU, you can refer to the [EdgeTPU guide](https://coral.ai/docs/edgetpu/compiler/) for instructions on converting your model. We recommend the [example](https://coral.ai/docs/edgetpu/retrain-detection/#download-and-configure-the-training-data) on retraining and converting SSD MobileNet for object detection. Note that this guide is only for training and converting the model to EdgeTPU. To deploy the model on the device, refer to one of the examples in [develop your own deep learning application](https://developer.axis.com/computer-vision/computer-vision-on-device/develop-your-own-deep-learning-application/).

There are also many other open-source examples available on how to train, quantize, and export a model for the EdgeTPU. You can find these examples on the [Google Coral tutorials](https://colab.research.google.com/github/google-coral/tutorials/) page. Most of these tutorials use TensorFlow 1, which is no longer supported by Google Coral, but they can still be used as guidance and executed locally on your machine by installing TensorFlow 1.

Axis provides another example in the acap-native-examples GitHub repository called [tensorflow-to-larod](https://github.com/AxisCommunications/acap-native-sdk-examples/tree/main/tensorflow-to-larod) on how to train, quantize, and export an image classification model for ARTPEC-7.

## ARTPEC-8 [​](https://developer.axis.com/computer-vision/computer-vision-on-device/dlpu-model-conversion/\#artpec-8 "Direct link to ARTPEC-8")

To convert a model for the ARTPEC-8 DLPU, follow these steps:

1. Train a model using TensorFlow.
2. Quantize and export the saved model to tflite using the TensorFlow Lite converter.

Please note that the ARTPEC-8 DLPU is optimized for per-tensor quantization. You can achieve per-tensor quantization by using the TensorFlow Lite converter with TensorFlow 1.15 or with TensorFlow 2 and adding the conversion flag `_experimental_disable_per_channel = True`.

We recommend referring to the example on how to retrain and convert SSD MobileNet for object detection. You can find this example [here](https://coral.ai/docs/edgetpu/retrain-detection/#download-and-configure-the-training-data). Please note that you should only follow this guide to train, quantize, and export the model to tflite. **You should not convert it to EdgeTPU.** To deploy the model on the device, refer to one of the examples in the [develop-your-own-deep-learning-application](https://developer.axis.com/computer-vision/computer-vision-on-device/develop-your-own-deep-learning-application/) page.

Axis also provides an example on how to train, quantize, and export an image classification model for ARTPEC-8 in the [acap-native-examples GitHub repository](https://github.com/AxisCommunications/acap-native-sdk-examples). The example is called [tensorflow-to-larod-artpec8](https://github.com/AxisCommunications/acap-native-sdk-examples/tree/main/tensorflow-to-larod-artpec8).

We provide a guide on how to convert YOLOv5 to tflite, optimizing it for ARTPEC-8. You can find this guide in the [Axis-model-zoo](https://github.com/AxisCommunications/axis-model-zoo/blob/main/docs/yolov5.md) repository.

## ARTPEC-9 [​](https://developer.axis.com/computer-vision/computer-vision-on-device/dlpu-model-conversion/\#artpec-9 "Direct link to ARTPEC-9")

To convert a model for the ARTPEC-9 DLPU, follow these steps:

1. Train a model using TensorFlow.
2. Quantize and export the saved model to TensorFlow Lite (tflite) using the TensorFlow Lite converter.

The conversion process for ARTPEC-9 DLPU is very similar to the conversion process for [ARTPEC-7](https://developer.axis.com/computer-vision/computer-vision-on-device/dlpu-model-conversion/#artpec-7) DLPU. The only difference is that you don't have to compile your model using the EdgeTPU compiler.
The ARTPEC-9 DLPU supports both per-channel and per-tensor quantization.

We recommend referring to the example on how to retrain and convert SSD MobileNet for object detection. You can find this example [here](https://coral.ai/docs/edgetpu/retrain-detection/#download-and-configure-the-training-data). Please note that you should only follow this guide to train, quantize, and export the model to tflite. **You should not convert it to EdgeTPU.** To deploy the model on the device, refer to one of the examples in the [develop-your-own-deep-learning-application](https://developer.axis.com/computer-vision/computer-vision-on-device/develop-your-own-deep-learning-application/) page.

Axis also provides an example of how to train, quantize, and export an image classification model for ARTPEC-9 in the [acap-native-examples GitHub repository](https://github.com/AxisCommunications/acap-native-sdk-examples). It is the same example as for ARTPEC-8, [tensorflow-to-larod-artpec8](https://github.com/AxisCommunications/acap-native-sdk-examples/tree/main/tensorflow-to-larod-artpec8).

We provide a [guide](https://github.com/AxisCommunications/axis-model-zoo/blob/main/docs/yolov5.md) on how to optimize and train YOLOv5 for ARTPEC-9 in the axis-model-zoo repository.

## CV25 [​](https://developer.axis.com/computer-vision/computer-vision-on-device/dlpu-model-conversion/\#cv25 "Direct link to CV25")

To convert a model for the CV25 DLPU, follow these steps:

1. Train a model using TensorFlow.
2. Export the saved model to TensorFlow Lite format using the TensorFlow Lite converter.
3. Compile the model using the Ambarella toolchain.

Converting a model for the CV25 DLPU is slightly more complex compared to other DLPUs. The toolchain for converting your model is not publicly available. To gain access to the toolchain, please contact [Ambarella](https://customer.ambarella.com/ng/) directly.

Once you have access to the toolchain, refer to their examples on how to convert a model. Axis also provides an example on how to convert a model for the CV25 DLPU in the acap-native-examples GitHub repository, [tensorflow-to-larod-cv25](https://github.com/AxisCommunications/acap-native-sdk-examples/tree/main/tensorflow-to-larod-cv25).

Differently from the other DLPUs, the CV25 DLPU requires the input to be in the RGB-planar format.

Last updated on **Oct 10, 2025**

[Previous\\
\\
Quantization](https://developer.axis.com/computer-vision/computer-vision-on-device/quantization/) [Next\\
\\
General suggestions](https://developer.axis.com/computer-vision/computer-vision-on-device/general-suggestions/)

- [ARTPEC-7](https://developer.axis.com/computer-vision/computer-vision-on-device/dlpu-model-conversion/#artpec-7)
- [ARTPEC-8](https://developer.axis.com/computer-vision/computer-vision-on-device/dlpu-model-conversion/#artpec-8)
- [ARTPEC-9](https://developer.axis.com/computer-vision/computer-vision-on-device/dlpu-model-conversion/#artpec-9)
- [CV25](https://developer.axis.com/computer-vision/computer-vision-on-device/dlpu-model-conversion/#cv25)

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