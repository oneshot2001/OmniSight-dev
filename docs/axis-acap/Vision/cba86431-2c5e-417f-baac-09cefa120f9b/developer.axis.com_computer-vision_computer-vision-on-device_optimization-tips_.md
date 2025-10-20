---
url: "https://developer.axis.com/computer-vision/computer-vision-on-device/optimization-tips/"
title: "Optimization tips | Axis developer documentation"
---

[Skip to main content](https://developer.axis.com/computer-vision/computer-vision-on-device/optimization-tips/#__docusaurus_skipToContent_fallback)

[![Axis Communications Logo](https://developer.axis.com/img/axis-logo.svg)\\
**Axis developer documentation**](https://developer.axis.com/)

```

```

[Blog](https://developer.axis.com/blog/)

- [Computer vision](https://developer.axis.com/computer-vision/)
- [Computer vision on device](https://developer.axis.com/computer-vision/computer-vision-on-device/optimization-tips/#)

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
- [How-to guides](https://developer.axis.com/computer-vision/computer-vision-on-device/optimization-tips/#)


- [Home page](https://developer.axis.com/)
- Computer vision on device
- Optimization tips

On this page

# Optimization tips

This document provides optimization tips for each Deep Learning Processing Unit (DLPU). It is meant for a more advanced audience, that want to obtain the best performance possible from their model investing more time in the optimization process. If you are not an expert, we recommend using only the models provided in the [recommended model architecture](https://developer.axis.com/computer-vision/computer-vision-on-device/recommended-model-architecture/) section of this documentation or in the [Axis Model Zoo](https://github.com/AxisCommunications/axis-model-zoo).

## ARTPEC-7 [​](https://developer.axis.com/computer-vision/computer-vision-on-device/optimization-tips/\#artpec-7 "Direct link to ARTPEC-7")

The ARTPEC-7 DLPU has a dedicated memory for the DLPU. To maximize the DLPU's performance, it is recommended to use lightweight models that can fit in this memory, such as SSD MobileNet v2 300x300.

When converting your model to EdgeTPU using `edgetpu-converter`, you may receive warnings about instructions that cannot be executed by the TPU. It is important to avoid using these instructions in your model. Both per-channel and per-tensor quantization offer similar performance, but per-channel quantization is recommended for better accuracy. For more details on how to optimize your model for ARTPEC-7 DLPU, refer to the [EdgeTPU documentation](https://coral.ai/docs/).

## ARTPEC-8 [​](https://developer.axis.com/computer-vision/computer-vision-on-device/optimization-tips/\#artpec-8 "Direct link to ARTPEC-8")

The ARTPEC-8 DLPU performs well with models of any size, as long as they fit in the device's memory. This allows for better performance with larger models compared to ARTPEC-7. Here are some additional optimizations to enhance DLPU performance:

- **Use per-tensor quantization.**
- Prefer regular convolutions over depth-wise convolutions, which means that architecture like RegNet-18 are more efficient than MobileNet.
- Optimal kernel size is 3x3.
- Use stride 2 whenever possible as it is natively supported by the convolution engine. For other cases, consider using pooling.
- Ensure the number of filters per convolution block is a multiple of 6.
- Applying ReLU as the activation function after a convolution will result in a faster fused layer.
- Sparsification can improve the performance of the model.

Here is also a more detailed table of specific supported operators for the ARTPEC-8 DLPU:

| **Operator Category** | **Operators** |
| --- | --- |
| Neural Network | FullyConnected, Conv2d, DepthwiseConv2d, TransposeConv, MaxPool2d, AveragePool2d |
| Data Manipulation | Concatenation, Reshape, ExpandDims, Squeeze, Slice, StridedSlice, Gather, GatherNd, OneHot, Split, Transpose, ResizeNearestNeighbor, ResizeBilinear, Cast |
| Math | Add, AddN, Mul, Div, FloorDiv, Pow, Square, Sin, Tanh, Abs, Neg, Sqrt, Floor, Minimum, Maximum |
| Comparison | Equal, NotEqual, Less, LessEqual, Greater, GreaterEqual |
| Logic | LogicalNot, LogicalAnd, LogicalOr |
| Activation Functions | Relu, Relu6, Softmax, Gelu, Elu, HardSwish |
| Reduction | ReduceMin, ReduceMax, ReduceAny, ReduceProd |
| Indexing | ArgMin, ArgMax |

## ARTPEC-9 [​](https://developer.axis.com/computer-vision/computer-vision-on-device/optimization-tips/\#artpec-9 "Direct link to ARTPEC-9")

The ARTPEC-9 DLPU performs well with models of any size, as long as they fit in the device's memory.
Here are some optimization tips to further increase performance.

- Tensors with up to 4 dimensions are supported.
- A `Batch` dimension >1 is not supported.
- `NHWC` tensors are supported.
- Most operations are not supported when the input or output tensor depths are too large.
- Use `ReLU6` as activation function when possible.
- Avoid explicit padding when possible.
- It is most optimal to use filters evenly divisible by 16.

Below is a detailed table of operators supported on the ARTPEC-9 DLPU, along with operator specific
restrictions. Unsupported operations or supported operations that do not meet the requirements will
be automatically offloaded to the CPU.

| **Operator** | **Restrictions** |
| --- | --- |
| Add | Element by element addition is supported. |
| AveragePool | \- Pooling size 3 x 3, with stride 1, 1 and `SAME` padding is supported.<br> \- "Mean" average pooling is supported where the pooling size is 7 or 8 and the input width and height is equal to the pool size. |
| Concat | \- Output quantization scale smaller than the input quantization scale divided by 128 is not supported. <br> \- If concatenating along the channel dimension, the channel dimension of every input tensor must be a multiple of 16. |
| Constant | No specific restrictions. |
| Conv2D | \- `HWIO` format weights are supported.<br> \- The supported kernel heights and widths (the kernel does not have to be square) are: { 1, 2, 3, 5, 7, 9 }. <br> \- The supported strides (the height and width stride have to match) are: { 1, 2 }.<br> \- For kernels with height or width >7, only a stride of 1 is supported.<br> \- `SAME` and `VALID` padding are supported.<br> \- `I*W/O` must be between 0 and 65536, where:<br>     \- `I` is the input quantization scale.<br>     \- `W` is the weight quantization scale.<br>     \- `O` is the output quantization scale. |
| DepthToSpace | \- A block size of 2 is supported.<br> \- Depth must be a multiple of the square of the block size. |
| DepthwiseConvolution2D | \- `HWIM` format weights are supported.<br> \- The supported kernel heights and widths (the kernel does not have to be square) are: { 1, 2, 3, 5, 7, 9 }.<br> \- The supported strides (the height and width stride have to match) are: { 1, 2 }.<br> \- For kernels with height or width >7, only a stride of 1 is supported.<br> \- `SAME` and `VALID` padding are supported.<br> \- A channel multiplier of 1 is supported. A channel multiplier >1 is not supported.<br> \- `I*W/O` must be between 0 and 65536, where:<br>     \- `I` is the input quantization scale.<br>     \- `W` is the weight quantization scale.<br>     \- `O` is the output quantization scale. |
| FullyConnected | \- `HWIO` format weights are supported, `H` and `W` must be 1.<br> \- `I*W/O` must be between 0 and 65536, where:<br>     \- `I` is the input quantization scale.<br>     \- `W` is the weight quantization scale.<br>     \- `O` is the output quantization scale. |
| LeakyReLU | Alpha must be less than 1 and greater than 0. |
| MaxPool | \- Supported configurations:<br>     \- 1 x 1 pooling size, 2, 2 stride (equivalent to downsample 2 x 2).<br>     \- 2 x 2 pooling size, 2, 2 stride, `VALID` padding, input sizes must be even.<br>     \- 2 x 2 pooling size, 2, 2 stride, `SAME` padding, input sizes must be odd.<br>     \- 3 x 3 pooling size, 2, 2 stride, `VALID` padding, input sizes must even, maximum tensor width is 417.<br>     \- 3 x 3 pooling size, 2, 2 stride, `SAME` padding, input sizes must be odd, maximum tensor width is 417.<br>     \- 1, 1 stride with pooling sizes up to 9x9 for `VALID` padding.<br>     \- 1, 1 stride with pooling sizes up to 17x17 for `SAME` padding.<br> \- Input size must not be smaller than the pooling size. |
| MeanXY | \- Supports mean reduction of `H x W` dimensions to 1 x 1.<br> \- Only supports:<br>     \- `N x 7 x 7 x C` input with `N x 1 x 1 x C` output.<br>     \- `N x 8 x 8 x C` input with `N x 1 x 1 x C` output. |
| Mul | \- The multiplication of a tensor with a constant tensor is supported when the constant shape is `1 x 1 x 1 x C`.<br> \- The multiplication of a variable with a scalar constant is supported when the quantized values in the output are the same as the input. |
| Pad | \- Only zero padding in the `H` and `W` dimension is supported.<br> \- Padding of up to 7 each side of the tensor in those dimensions is supported.<br> \- Padding amounts can differ per side, e.g. pad of 1 before the tensor in the `H` dimension and a pad of 3 after the tensor in the `H` dimension.<br> \- Quantization for input and output tensors must be identical. |
| Reinterpret quantization | No specific restrictions. |
| ReLU | Lower bound must be less than the upper bound. |
| Requantize | \- Output quantization scale smaller than the input quantization scale divided by 128 is not supported.<br> \- Requantize with different input/output type is supported. |
| Reshape | No specific restrictions. |
| Resize | \- The resized height or width must be `2n` or `2n-1` where `n` is the original height or width.<br> \- If resized height and width are not both odd or both even the result might be less accurate.<br> \- Some Resize Bilinear configurations ( `align_corners=True`, `half_pixel_centres=True` when heights and widths are not both even or both odd) produce inaccurate results. |
| Sigmoid | The output for sigmoid always has a quantization zero point equal to the minimum value of the quantized data type and a quantization scale of 1 / 256. |
| Split | If splitting along the channel dimension, the channel dimension of every output tensor must be a multiple of 16. |
| Tanh | The output for tanh always has a quantization zero point equal to the middle value of the quantized data type and a quantization scale of 1 / 128. |
| TransposeConvolution2D | \- `HWIO` format weights are supported.<br> \- The supported kernel heights and widths (the kernel does not have to be square) are: { 1, 2, 3, 5, 7, 9 }.<br> \- Only a stride of 2 is supported.<br> \- `SAME` and `VALID` padding are supported.<br> \- `I*W/O` must be between 0 and 65536, where:<br>     \- `I` is the input quantization scale.<br>     \- `W` is the weight quantization scale.<br>     \- `O` is the output quantization scale. |

## CV25 [​](https://developer.axis.com/computer-vision/computer-vision-on-device/optimization-tips/\#cv25 "Direct link to CV25")

For CV25, model quantization and optimizations are mainly performed by the compiler. It is recommended to refer to the documentation provided with the Ambarella SDK for more information.

One important consideration is to have a model with an input size that is a multiple of 32. Otherwise, padding will be required for the input, making the conversion process slightly more complex, and the model less efficient.

Last updated on **Aug 27, 2025**

[Previous\\
\\
Recommended model architecture](https://developer.axis.com/computer-vision/computer-vision-on-device/recommended-model-architecture/) [Next\\
\\
Test your model](https://developer.axis.com/computer-vision/computer-vision-on-device/test-your-model/)

- [ARTPEC-7](https://developer.axis.com/computer-vision/computer-vision-on-device/optimization-tips/#artpec-7)
- [ARTPEC-8](https://developer.axis.com/computer-vision/computer-vision-on-device/optimization-tips/#artpec-8)
- [ARTPEC-9](https://developer.axis.com/computer-vision/computer-vision-on-device/optimization-tips/#artpec-9)
- [CV25](https://developer.axis.com/computer-vision/computer-vision-on-device/optimization-tips/#cv25)

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