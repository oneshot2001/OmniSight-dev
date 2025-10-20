---
url: "https://developer.axis.com/computer-vision/computer-vision-on-device/supported-frameworks/"
title: "Supported frameworks | Axis developer documentation"
---

[Skip to main content](https://developer.axis.com/computer-vision/computer-vision-on-device/supported-frameworks/#__docusaurus_skipToContent_fallback)

[![Axis Communications Logo](https://developer.axis.com/img/axis-logo.svg)\\
**Axis developer documentation**](https://developer.axis.com/)

```

```

[Blog](https://developer.axis.com/blog/)

- [Computer vision](https://developer.axis.com/computer-vision/)
- [Computer vision on device](https://developer.axis.com/computer-vision/computer-vision-on-device/supported-frameworks/#)

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
- [How-to guides](https://developer.axis.com/computer-vision/computer-vision-on-device/supported-frameworks/#)


- [Home page](https://developer.axis.com/)
- Computer vision on device
- Supported frameworks

On this page

# Supported frameworks

All Deep Learning Processing Units (DLPUs), except for CV25, support only the [TensorFlow Lite](https://www.tensorflow.org/lite) format. CV25 uses a proprietary format, but it can be converted from TensorFlow Lite or ONNX. When using the tflite model, keep in mind that only tflite built-in int8 ops are supported to run on the DLPU `tf.lite.OpsSet.TFLITE_BUILTINS_INT8`. Custom ops or TensorFlow ops are not supported.
The exact version of TensorFlow on the device can change depending on the device model and firmware. You can find the exact TensorFlow version in the
Software Bill of Materials associated to the firmware of your specific device in the [Download device software](https://www.axis.com/support/device-software) page.

## Converting from TensorFlow or Keras to TensorFlow Lite [​](https://developer.axis.com/computer-vision/computer-vision-on-device/supported-frameworks/\#converting-from-tensorflow-or-keras-to-tensorflow-lite "Direct link to Converting from TensorFlow or Keras to TensorFlow Lite")

To convert a TensorFlow/Keras model to a tflite model, you can use the tflite converter. This is the most reliable way to convert a model to tflite. Here is an example code snippet on how to convert a model to tflite:

Convert to tflite

```codeBlockLines_e6Vv
import tensorflow as tf

converter = tf.lite.TFLiteConverter.from_saved_model(saved_model_dir)
converter.optimizations = [tf.lite.Optimize.DEFAULT]
converter.representative_dataset = a_representative_datagenerator
converter.target_spec.supported_ops = [tf.lite.OpsSet.TFLITE_BUILTINS_INT8]
converter.inference_input_type = tf.uint8
converter.inference_output_type = tf.uint8
tflite_quant_model = converter.convert()

```

You can find the full code example [here](https://github.com/AxisCommunications/acap-native-sdk-examples/blob/main/tensorflow-to-larod/env/convert_model.py).

In the case of ARTPEC-8, if the script is based on TensorFlow 2, the following flag is also needed to achieve optimal performance.

Convert to tflite

```codeBlockLines_e6Vv
converter._experimental_disable_per_channel = True

```

## Converting from ONNX to TensorFlow Lite [​](https://developer.axis.com/computer-vision/computer-vision-on-device/supported-frameworks/\#converting-from-onnx-to-tensorflow-lite "Direct link to Converting from ONNX to TensorFlow Lite")

No Axis DLPUs, except for CV25, support ONNX models. However, you can try converting an ONNX model to a tflite model using the unofficial [onnx2tflite](https://github.com/MPolaris/onnx2tflite) or [onnx2tf](https://github.com/PINTO0309/onnx2tf) tools. Please note that these tools are not supported by Axis, and there is no guarantee that the conversion will work or that the converted model will run on the device. Debugging may be required to make the model run on the device.

## Converting from PyTorch to TensorFlow Lite [​](https://developer.axis.com/computer-vision/computer-vision-on-device/supported-frameworks/\#converting-from-pytorch-to-tensorflow-lite "Direct link to Converting from PyTorch to TensorFlow Lite")

Axis DLPUs do not support PyTorch models. However, you can export a PyTorch model to ONNX and then convert it to tflite. You can find an example [here](https://pytorch.org/tutorials/advanced/super_resolution_with_onnxruntime.html). Please note that jumping from PyTorch to ONNX to TFlite is not recommended and may require a lot of trial and error.

## Using Non-Supported Frameworks [​](https://developer.axis.com/computer-vision/computer-vision-on-device/supported-frameworks/\#using-non-supported-frameworks "Direct link to Using Non-Supported Frameworks")

Although PyTorch and ONNX are not supported, it is theoretically possible to run a Python script that uses PyTorch or ONNX runtime to run the model in a Docker container on the device. Please note that this is not supported by Axis, and there is no guarantee that it will work smoothly. Keep in mind that the model will only run on the CPU and will not take advantage of the DLPU to accelerate the inference.

Last updated on **Aug 18, 2025**

[Previous\\
\\
Axis Deep Learning Processing Unit (DLPU)](https://developer.axis.com/computer-vision/computer-vision-on-device/axis-dlpu/) [Next\\
\\
Quantization](https://developer.axis.com/computer-vision/computer-vision-on-device/quantization/)

- [Converting from TensorFlow or Keras to TensorFlow Lite](https://developer.axis.com/computer-vision/computer-vision-on-device/supported-frameworks/#converting-from-tensorflow-or-keras-to-tensorflow-lite)
- [Converting from ONNX to TensorFlow Lite](https://developer.axis.com/computer-vision/computer-vision-on-device/supported-frameworks/#converting-from-onnx-to-tensorflow-lite)
- [Converting from PyTorch to TensorFlow Lite](https://developer.axis.com/computer-vision/computer-vision-on-device/supported-frameworks/#converting-from-pytorch-to-tensorflow-lite)
- [Using Non-Supported Frameworks](https://developer.axis.com/computer-vision/computer-vision-on-device/supported-frameworks/#using-non-supported-frameworks)

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