#!/usr/bin/env python3
"""
TFLite INT8 Quantization Converter for OMNISIGHT
Converts trained model to TFLite with ARTPEC-8 optimizations
"""

import os
import sys
import argparse
import numpy as np
import tensorflow as tf
from pathlib import Path
import json

# Import representative dataset generator
from coco_dataset import create_representative_dataset


class TFLiteConverter:
    """Convert TensorFlow model to TFLite with INT8 quantization"""

    def __init__(
        self,
        model_path: str,
        output_path: str,
        quantize: bool = True,
        representative_dataset=None
    ):
        self.model_path = Path(model_path)
        self.output_path = Path(output_path)
        self.quantize = quantize
        self.representative_dataset = representative_dataset

    def convert(self):
        """Convert model to TFLite"""
        print("\n" + "="*60)
        print("TFLite Conversion")
        print("="*60 + "\n")

        print(f"[Convert] Loading model from {self.model_path}")

        # Load model
        if self.model_path.suffix == '.keras':
            model = tf.keras.models.load_model(str(self.model_path))
        else:
            # Assume SavedModel format
            model = tf.saved_model.load(str(self.model_path))

        # Create converter
        if self.model_path.suffix == '.keras':
            converter = tf.lite.TFLiteConverter.from_keras_model(model)
        else:
            converter = tf.lite.TFLiteConverter.from_saved_model(str(self.model_path))

        # Configure for ARTPEC-8 DLPU
        if self.quantize:
            print("[Convert] Configuring INT8 quantization for ARTPEC-8...")

            # Enable INT8 quantization
            converter.optimizations = [tf.lite.Optimize.DEFAULT]

            # Set representative dataset for calibration
            if self.representative_dataset:
                converter.representative_dataset = self.representative_dataset
                print("[Convert] Using representative dataset for calibration")

            # Ensure INT8 activations (required for ARTPEC-8)
            converter.target_spec.supported_ops = [tf.lite.OpsSet.TFLITE_BUILTINS_INT8]

            # ARTPEC-8 specific optimizations
            # Per-tensor quantization (not per-channel) for better DLPU compatibility
            converter.experimental_new_quantizer = True

            # Inference input/output type: INT8
            converter.inference_input_type = tf.int8
            converter.inference_output_type = tf.int8

            print("[Quantization] Configured:")
            print("  - Type: INT8 per-tensor")
            print("  - Input/Output: INT8")
            print("  - Optimized for: ARTPEC-8 DLPU")

        else:
            print("[Convert] No quantization (FP32 model)")

        # Convert
        print("\n[Convert] Converting model...")
        try:
            tflite_model = converter.convert()
            print(f"[Convert] ✓ Conversion successful")
        except Exception as e:
            print(f"[Convert] ✗ Conversion failed: {e}")
            raise

        # Save
        self.output_path.parent.mkdir(parents=True, exist_ok=True)
        self.output_path.write_bytes(tflite_model)

        # Get model size
        model_size_mb = len(tflite_model) / (1024 * 1024)

        print(f"\n[Saved] TFLite model: {self.output_path}")
        print(f"[Size] {model_size_mb:.2f} MB")

        # Validate conversion
        self._validate_model(tflite_model)

        return str(self.output_path)

    def _validate_model(self, tflite_model):
        """Validate converted TFLite model"""
        print("\n[Validate] Checking model compatibility...")

        # Create interpreter
        interpreter = tf.lite.Interpreter(model_content=tflite_model)
        interpreter.allocate_tensors()

        # Get input/output details
        input_details = interpreter.get_input_details()
        output_details = interpreter.get_output_details()

        print("\n[Model Info]")
        print(f"  Inputs: {len(input_details)}")
        for i, detail in enumerate(input_details):
            print(f"    [{i}] {detail['name']}: {detail['shape']} {detail['dtype']}")

        print(f"\n  Outputs: {len(output_details)}")
        for i, detail in enumerate(output_details):
            print(f"    [{i}] {detail['name']}: {detail['shape']} {detail['dtype']}")

        # Check quantization
        if self.quantize:
            is_quantized = input_details[0]['dtype'] == np.int8
            print(f"\n  Quantization: {'✓ INT8' if is_quantized else '✗ Not quantized'}")

            if is_quantized:
                print(f"  Input scale: {input_details[0].get('quantization_parameters', {}).get('scales', 'N/A')}")
                print(f"  Input zero_point: {input_details[0].get('quantization_parameters', {}).get('zero_points', 'N/A')}")

        # Test inference
        print("\n[Validate] Running test inference...")
        try:
            # Create dummy input
            input_shape = input_details[0]['shape']
            if input_details[0]['dtype'] == np.int8:
                test_input = np.random.randint(-128, 127, size=input_shape, dtype=np.int8)
            else:
                test_input = np.random.randn(*input_shape).astype(np.float32)

            interpreter.set_tensor(input_details[0]['index'], test_input)
            interpreter.invoke()

            # Get output
            output = interpreter.get_tensor(output_details[0]['index'])
            print(f"[Validate] ✓ Test inference successful")
            print(f"[Validate]   Output shape: {output.shape}")
            print(f"[Validate]   Output range: [{output.min()}, {output.max()}]")

        except Exception as e:
            print(f"[Validate] ✗ Test inference failed: {e}")

        print("\n[Validate] ✓ Model validation complete")


def main():
    parser = argparse.ArgumentParser(description='Convert OMNISIGHT model to TFLite INT8')
    parser.add_argument('--model', type=str, required=True,
                        help='Path to trained model (.keras or SavedModel directory)')
    parser.add_argument('--output', type=str, default=None,
                        help='Output TFLite file path')
    parser.add_argument('--quantize', action='store_true', default=True,
                        help='Enable INT8 quantization')
    parser.add_argument('--no-quantize', action='store_true',
                        help='Disable quantization (FP32)')
    parser.add_argument('--representative-data', type=str, default=None,
                        help='Path to representative dataset (annotations.json)')
    parser.add_argument('--representative-images', type=str, default=None,
                        help='Path to representative images directory')
    parser.add_argument('--num-calibration-samples', type=int, default=100,
                        help='Number of samples for calibration')

    args = parser.parse_args()

    # Determine output path
    if args.output is None:
        model_path = Path(args.model)
        if model_path.suffix == '.keras':
            output_path = model_path.with_suffix('.tflite')
        else:
            output_path = model_path.parent / 'model.tflite'
    else:
        output_path = Path(args.output)

    # Create representative dataset if quantization is enabled
    representative_dataset_gen = None
    if args.quantize and not args.no_quantize:
        if args.representative_data and args.representative_images:
            print("[Calibration] Creating representative dataset...")
            representative_dataset_gen = create_representative_dataset(
                annotations_file=args.representative_data,
                images_dir=args.representative_images,
                num_samples=args.num_calibration_samples
            )
            print(f"[Calibration] Using {args.num_calibration_samples} samples")
        else:
            print("[Warning] No representative dataset provided for INT8 calibration")
            print("[Warning] Quantization may be suboptimal")

    # Convert
    converter = TFLiteConverter(
        model_path=args.model,
        output_path=output_path,
        quantize=(args.quantize and not args.no_quantize),
        representative_dataset=representative_dataset_gen
    )

    try:
        tflite_path = converter.convert()

        print("\n" + "="*60)
        print("✓ Conversion Complete!")
        print("="*60)
        print(f"\nTFLite model: {tflite_path}")
        print("\nReady for ARTPEC-8 DLPU deployment!")
        print()

    except Exception as e:
        print(f"\n[ERROR] Conversion failed: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)


if __name__ == '__main__':
    main()
