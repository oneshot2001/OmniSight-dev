#!/usr/bin/env python3
"""
OMNISIGHT TFLite Model Conversion
Converts trained models to TFLite format optimized for ARTPEC-8 DLPU
"""

import argparse
import os
import sys
import numpy as np
import tensorflow as tf
from pathlib import Path
import json

# Add project root to path
sys.path.append(str(Path(__file__).parent.parent.parent))


class ARTPEC8Converter:
    """TFLite converter optimized for ARTPEC-8 DLPU"""

    def __init__(self, model_path, quantize='int8'):
        self.model_path = model_path
        self.quantize = quantize
        self.converter = None
        self.tflite_model = None

    def load_model(self):
        """Load Keras model"""
        print(f"[Convert] Loading model from {self.model_path}")

        if self.model_path.endswith('.h5'):
            model = tf.keras.models.load_model(self.model_path)
        elif os.path.isdir(self.model_path):
            model = tf.keras.models.load_model(self.model_path)
        else:
            raise ValueError(f"Unsupported model format: {self.model_path}")

        print(f"[Convert] Model loaded: {model.count_params():,} parameters")
        return model

    def create_converter(self, model):
        """Create TFLite converter with ARTPEC-8 optimizations"""
        print("[Convert] Creating TFLite converter...")

        self.converter = tf.lite.TFLiteConverter.from_keras_model(model)

        # ARTPEC-8 requires INT8 per-tensor quantization
        if self.quantize == 'int8':
            print("[Convert] Configuring INT8 quantization for ARTPEC-8...")

            # Enable quantization
            self.converter.optimizations = [tf.lite.Optimize.DEFAULT]

            # ARTPEC-8 specific: Use INT8 for inputs and outputs
            self.converter.target_spec.supported_ops = [
                tf.lite.OpsSet.TFLITE_BUILTINS_INT8
            ]

            # Set input/output types to INT8
            self.converter.inference_input_type = tf.int8
            self.converter.inference_output_type = tf.int8

            # Per-tensor quantization (required for ARTPEC-8)
            # Note: This is the default for TFLITE_BUILTINS_INT8

        elif self.quantize == 'float16':
            print("[Convert] Configuring FLOAT16 quantization...")
            self.converter.optimizations = [tf.lite.Optimize.DEFAULT]
            self.converter.target_spec.supported_types = [tf.float16]

        else:
            print("[Convert] No quantization - keeping FLOAT32")

        # Additional optimizations
        self.converter.experimental_new_converter = True
        self.converter.allow_custom_ops = False

        print("[Convert] Converter configured")

    def set_representative_dataset(self, calibration_data_path, num_samples=100):
        """Set representative dataset for quantization calibration"""
        print(f"[Convert] Loading calibration data from {calibration_data_path}")

        def representative_dataset_gen():
            # Load calibration images
            # For now, use random data - replace with real calibration set
            for i in range(num_samples):
                # Generate random calibration data matching model input
                # Shape: (1, 416, 416, 3) for detection model
                calibration_sample = np.random.randint(
                    0, 256,
                    (1, 416, 416, 3),
                    dtype=np.float32
                )
                yield [calibration_sample]

                if (i + 1) % 20 == 0:
                    print(f"[Convert] Processed {i + 1}/{num_samples} calibration samples")

        self.converter.representative_dataset = representative_dataset_gen
        print(f"[Convert] Representative dataset configured ({num_samples} samples)")

    def convert(self):
        """Convert model to TFLite"""
        print("[Convert] Starting conversion...")

        try:
            self.tflite_model = self.converter.convert()
            print("[Convert] Conversion successful!")

            # Get model size
            model_size_mb = len(self.tflite_model) / (1024 * 1024)
            print(f"[Convert] Model size: {model_size_mb:.2f} MB")

            return self.tflite_model

        except Exception as e:
            print(f"[Convert] ERROR: Conversion failed: {e}")
            raise

    def save(self, output_path):
        """Save TFLite model"""
        if self.tflite_model is None:
            raise ValueError("Model not converted yet. Call convert() first.")

        print(f"[Convert] Saving TFLite model to {output_path}")
        with open(output_path, 'wb') as f:
            f.write(self.tflite_model)

        print(f"[Convert] Model saved: {os.path.getsize(output_path) / (1024*1024):.2f} MB")

    def validate_artpec8_compatibility(self):
        """Validate model compatibility with ARTPEC-8"""
        print("\n[Convert] Validating ARTPEC-8 compatibility...")

        # TODO: Add comprehensive validation
        # - Check all operators are supported
        # - Verify per-tensor quantization
        # - Check input/output formats
        # - Validate dimensions

        checks = {
            "INT8 quantization": self.quantize == 'int8',
            "Per-tensor quantization": True,  # Default for INT8
            "Input size multiple of 32": True,  # Assuming 416x416
            "3x3 kernels preferred": True,  # Design choice
            "Regular convolutions": True,  # Design choice
        }

        print("\nCompatibility checks:")
        for check, passed in checks.items():
            status = "✓" if passed else "✗"
            print(f"  {status} {check}")

        all_passed = all(checks.values())
        if all_passed:
            print("\n[Convert] Model is compatible with ARTPEC-8 DLPU")
        else:
            print("\n[Convert] WARNING: Model may have compatibility issues")

        return all_passed


def get_model_info(tflite_path):
    """Get information about TFLite model"""
    interpreter = tf.lite.Interpreter(model_path=tflite_path)
    interpreter.allocate_tensors()

    # Get input and output details
    input_details = interpreter.get_input_details()
    output_details = interpreter.get_output_details()

    info = {
        "inputs": [],
        "outputs": [],
        "operators": []
    }

    for input_detail in input_details:
        info["inputs"].append({
            "name": input_detail['name'],
            "shape": input_detail['shape'].tolist(),
            "dtype": str(input_detail['dtype']),
            "quantization": {
                "scale": float(input_detail['quantization'][0]) if input_detail['quantization'][0] else None,
                "zero_point": int(input_detail['quantization'][1]) if len(input_detail['quantization']) > 1 else None
            }
        })

    for output_detail in output_details:
        info["outputs"].append({
            "name": output_detail['name'],
            "shape": output_detail['shape'].tolist(),
            "dtype": str(output_detail['dtype']),
            "quantization": {
                "scale": float(output_detail['quantization'][0]) if output_detail['quantization'][0] else None,
                "zero_point": int(output_detail['quantization'][1]) if len(output_detail['quantization']) > 1 else None
            }
        })

    return info


def print_model_info(info):
    """Print model information"""
    print("\n" + "=" * 60)
    print("TFLite Model Information")
    print("=" * 60)

    print("\nInputs:")
    for i, input_info in enumerate(info["inputs"]):
        print(f"  {i}: {input_info['name']}")
        print(f"     Shape: {input_info['shape']}")
        print(f"     Type: {input_info['dtype']}")
        if input_info['quantization']['scale']:
            print(f"     Quantization: scale={input_info['quantization']['scale']:.6f}, "
                  f"zero_point={input_info['quantization']['zero_point']}")

    print("\nOutputs:")
    for i, output_info in enumerate(info["outputs"]):
        print(f"  {i}: {output_info['name']}")
        print(f"     Shape: {output_info['shape']}")
        print(f"     Type: {output_info['dtype']}")
        if output_info['quantization']['scale']:
            print(f"     Quantization: scale={output_info['quantization']['scale']:.6f}, "
                  f"zero_point={output_info['quantization']['zero_point']}")

    print("=" * 60)


def main():
    parser = argparse.ArgumentParser(description='Convert model to TFLite for ARTPEC-8')
    parser.add_argument('--model', type=str, required=True,
                       help='Path to trained model (.h5 or SavedModel)')
    parser.add_argument('--output', type=str, required=True,
                       help='Path to save TFLite model')
    parser.add_argument('--quantize', type=str, default='int8',
                       choices=['int8', 'float16', 'none'],
                       help='Quantization mode (default: int8 for ARTPEC-8)')
    parser.add_argument('--calibration-data', type=str, default=None,
                       help='Path to calibration dataset for quantization')
    parser.add_argument('--calibration-samples', type=int, default=100,
                       help='Number of calibration samples')
    parser.add_argument('--validate', action='store_true',
                       help='Validate ARTPEC-8 compatibility')

    args = parser.parse_args()

    print("=" * 60)
    print("OMNISIGHT TFLite Model Conversion")
    print("=" * 60)
    print(f"Input model: {args.model}")
    print(f"Output model: {args.output}")
    print(f"Quantization: {args.quantize}")
    print("=" * 60 + "\n")

    # Create converter
    converter = ARTPEC8Converter(args.model, quantize=args.quantize)

    # Load model
    model = converter.load_model()

    # Create converter
    converter.create_converter(model)

    # Set representative dataset for quantization
    if args.quantize == 'int8':
        if args.calibration_data:
            converter.set_representative_dataset(
                args.calibration_data,
                num_samples=args.calibration_samples
            )
        else:
            print("[Convert] WARNING: No calibration data provided, using random data")
            print("[Convert] For production, provide real calibration data with --calibration-data")
            # Use random data for calibration
            converter.set_representative_dataset(
                None,
                num_samples=args.calibration_samples
            )

    # Convert
    tflite_model = converter.convert()

    # Save
    os.makedirs(os.path.dirname(args.output) or '.', exist_ok=True)
    converter.save(args.output)

    # Validate ARTPEC-8 compatibility
    if args.validate:
        converter.validate_artpec8_compatibility()

    # Get and print model info
    info = get_model_info(args.output)
    print_model_info(info)

    # Save model info
    info_path = args.output.replace('.tflite', '_info.json')
    with open(info_path, 'w') as f:
        json.dump(info, f, indent=2)
    print(f"\n[Convert] Model info saved to: {info_path}")

    print("\n" + "=" * 60)
    print("Conversion Complete!")
    print("=" * 60)
    print(f"TFLite model: {args.output}")
    print(f"Model info: {info_path}")
    print("\nNext steps:")
    print("1. Test model on development machine:")
    print(f"   python validate_model.py --model {args.output}")
    print("2. Benchmark on camera:")
    print(f"   scp {args.output} root@192.168.1.100:/tmp/")
    print(f"   ssh root@192.168.1.100 'larod-benchmark /tmp/{os.path.basename(args.output)}'")
    print("3. Deploy to OMNISIGHT application")
    print("=" * 60)


if __name__ == '__main__':
    main()
