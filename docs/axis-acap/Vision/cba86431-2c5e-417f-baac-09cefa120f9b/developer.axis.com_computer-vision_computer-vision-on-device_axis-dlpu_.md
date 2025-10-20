---
url: "https://developer.axis.com/computer-vision/computer-vision-on-device/axis-dlpu/"
title: "Axis Deep Learning Processing Unit (DLPU) | Axis developer documentation"
---

[Skip to main content](https://developer.axis.com/computer-vision/computer-vision-on-device/axis-dlpu/#__docusaurus_skipToContent_fallback)

[![Axis Communications Logo](https://developer.axis.com/img/axis-logo.svg)\\
**Axis developer documentation**](https://developer.axis.com/)

```

```

[Blog](https://developer.axis.com/blog/)

- [Computer vision](https://developer.axis.com/computer-vision/)
- [Computer vision on device](https://developer.axis.com/computer-vision/computer-vision-on-device/axis-dlpu/#)

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
- [How-to guides](https://developer.axis.com/computer-vision/computer-vision-on-device/axis-dlpu/#)


- [Home page](https://developer.axis.com/)
- Computer vision on device
- Axis Deep Learning Processing Unit (DLPU)

On this page

# Axis Deep Learning Processing Unit (DLPU)

Some Axis devices are equipped with a Deep Learning Processing Unit (DLPU). The DLPU is designed to accelerate the execution of your model, making inference significantly faster compared to the CPU. The speed-up you achieve using the DLPU may vary and depends on how optimized your model is for the DLPU.

## Axis devices with DLPU [​](https://developer.axis.com/computer-vision/computer-vision-on-device/axis-dlpu/\#axis-devices-with-dlpu "Direct link to Axis devices with DLPU")

To determine if a device is equipped with a DLPU, you can use the [Axis product selector](https://www.axis.com/support/tools/product-selector/shared/%5B%7B%22index%22%3A%5B4%2C1%5D%2C%22value%22%3A%22DLPU%22%7D%5D) and filter the results to show only devices with a DLPU. It is important to note that not all devices have the same DLPU. To identify the DLPU of your Axis device, you can check the System on Chip name (SoC) of your device. There are currently 3 different families of device that are using a DLPU:

- ARTPEC-7
- ARTPEC-8
- ARTPEC-9
- CV25

## DLPU characteristics [​](https://developer.axis.com/computer-vision/computer-vision-on-device/axis-dlpu/\#dlpu-characteristics "Direct link to DLPU characteristics")

Different DLPUs perform best with different models and may require different conversion or quantization processes. Each DLPU will be discussed individually in the following sections. Here is a brief overview of the characteristics of these DLPUs:

| SoC | Speed | Supported model format | Quantization | Ease of usage |
| --- | --- | --- | --- | --- |
| CPU | 🐢 | .tflite | Not needed | Easy |
| ARTPEC-7 | 🚲 | .tflite + edgetpu-compiler | INT8 | Easy |
| ARTPEC-8 | 🚗 | .tflite | INT8 per-tensor | Medium |
| ARTPEC-9 | 🚗 | .tflite | INT8 | Easy |
| CV25 | 🚗 | .tflite/.onnx + compiler | _Not needed_ | Hard |

\\* Quantization is performed by the compiler

Devices with the same SoC will have the same DLPU and perform similarly. There may be some performance differences between products with the same SoC due to variations in hardware configurations (e.g., clock speed, memory, etc.). For example, Q1656 and P3265 have the same SoC (ARTPEC-8), but the Q1656 has an higher clock speed and more memory, which results in better performance.

The above table intentionally does not provide specific speed values, as the performance of the accelerators depends on the model used, and it is difficult to quantify them in absolute terms. For more insights on DLPU performance, you can refer to the [Axis model zoo](https://github.com/AxisCommunications/axis-model-zoo), where you can find benchmarks of the different accelerators using different models. As shown in the table above, different DLPUs require different types of quantization. More details about quantization can be found in the [Quantization](https://developer.axis.com/computer-vision/computer-vision-on-device/quantization/) page.

Last updated on **Aug 18, 2025**

[Previous\\
\\
Develop your own deep learning application](https://developer.axis.com/computer-vision/computer-vision-on-device/develop-your-own-deep-learning-application/) [Next\\
\\
Supported frameworks](https://developer.axis.com/computer-vision/computer-vision-on-device/supported-frameworks/)

- [Axis devices with DLPU](https://developer.axis.com/computer-vision/computer-vision-on-device/axis-dlpu/#axis-devices-with-dlpu)
- [DLPU characteristics](https://developer.axis.com/computer-vision/computer-vision-on-device/axis-dlpu/#dlpu-characteristics)

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