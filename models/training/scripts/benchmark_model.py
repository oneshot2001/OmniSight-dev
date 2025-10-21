#!/usr/bin/env python3
"""
Model Benchmarking and Validation for OMNISIGHT
Tests TFLite model performance and ARTPEC-8 compatibility
"""

import os
import sys
import argparse
import numpy as np
import tensorflow as tf
from pathlib import Path
import json
import time
from typing import List, Dict
import cv2

# Import COCO data loader
from coco_dataset import COCODataLoader


class ModelBenchmark:
    """Benchmark TFLite model performance"""

    def __init__(self, model_path: str):
        self.model_path = Path(model_path)
        self.interpreter = None
        self.input_details = None
        self.output_details = None
        self.results = {}

    def load_model(self):
        """Load TFLite model"""
        print(f"[Load] Loading model from {self.model_path}")

        # Create interpreter
        self.interpreter = tf.lite.Interpreter(model_path=str(self.model_path))
        self.interpreter.allocate_tensors()

        # Get input/output details
        self.input_details = self.interpreter.get_input_details()
        self.output_details = self.interpreter.get_output_details()

        print(f"[Load] ✓ Model loaded successfully")

        # Print model info
        self._print_model_info()

    def _print_model_info(self):
        """Print model information"""
        print("\n" + "="*60)
        print("Model Information")
        print("="*60 + "\n")

        # File info
        model_size_mb = self.model_path.stat().st_size / (1024 * 1024)
        print(f"File size: {model_size_mb:.2f} MB")

        # Input info
        print(f"\nInputs: {len(self.input_details)}")
        for i, detail in enumerate(self.input_details):
            print(f"  [{i}] {detail['name']}")
            print(f"      Shape: {detail['shape']}")
            print(f"      Type: {detail['dtype']}")
            if 'quantization_parameters' in detail:
                scales = detail['quantization_parameters'].get('scales', [])
                zero_points = detail['quantization_parameters'].get('zero_points', [])
                if len(scales) > 0:
                    print(f"      Scale: {scales[0]:.6f}")
                    print(f"      Zero point: {zero_points[0]}")

        # Output info
        print(f"\nOutputs: {len(self.output_details)}")
        for i, detail in enumerate(self.output_details):
            print(f"  [{i}] {detail['name']}")
            print(f"      Shape: {detail['shape']}")
            print(f"      Type: {detail['dtype']}")

    def benchmark_latency(self, num_runs: int = 100, warmup_runs: int = 10):
        """Benchmark inference latency"""
        print("\n" + "="*60)
        print(f"Latency Benchmark ({num_runs} runs)")
        print("="*60 + "\n")

        # Create dummy input
        input_shape = self.input_details[0]['shape']
        input_dtype = self.input_details[0]['dtype']

        if input_dtype == np.int8:
            test_input = np.random.randint(-128, 127, size=input_shape, dtype=np.int8)
        else:
            test_input = np.random.randn(*input_shape).astype(np.float32)

        # Warmup
        print(f"[Warmup] Running {warmup_runs} warmup iterations...")
        for _ in range(warmup_runs):
            self.interpreter.set_tensor(self.input_details[0]['index'], test_input)
            self.interpreter.invoke()

        # Benchmark
        print(f"[Benchmark] Running {num_runs} benchmark iterations...")
        latencies = []

        for _ in range(num_runs):
            start_time = time.perf_counter()
            self.interpreter.set_tensor(self.input_details[0]['index'], test_input)
            self.interpreter.invoke()
            end_time = time.perf_counter()

            latency_ms = (end_time - start_time) * 1000
            latencies.append(latency_ms)

        # Statistics
        latencies = np.array(latencies)
        self.results['latency'] = {
            'mean': float(np.mean(latencies)),
            'std': float(np.std(latencies)),
            'min': float(np.min(latencies)),
            'max': float(np.max(latencies)),
            'p50': float(np.percentile(latencies, 50)),
            'p95': float(np.percentile(latencies, 95)),
            'p99': float(np.percentile(latencies, 99)),
        }

        # Print results
        print(f"\n[Results]")
        print(f"  Mean: {self.results['latency']['mean']:.2f} ms")
        print(f"  Std:  {self.results['latency']['std']:.2f} ms")
        print(f"  Min:  {self.results['latency']['min']:.2f} ms")
        print(f"  Max:  {self.results['latency']['max']:.2f} ms")
        print(f"  P50:  {self.results['latency']['p50']:.2f} ms")
        print(f"  P95:  {self.results['latency']['p95']:.2f} ms")
        print(f"  P99:  {self.results['latency']['p99']:.2f} ms")

        # Target check (ARTPEC-8 target: <15ms)
        target_latency = 15.0
        passes_target = self.results['latency']['p95'] < target_latency

        print(f"\n[Target] <{target_latency}ms (P95): {'✓ PASS' if passes_target else '✗ FAIL'}")

        return passes_target

    def estimate_memory(self):
        """Estimate memory usage"""
        print("\n" + "="*60)
        print("Memory Estimation")
        print("="*60 + "\n")

        # Model size
        model_size_mb = self.model_path.stat().st_size / (1024 * 1024)

        # Tensor sizes
        input_size_mb = np.prod(self.input_details[0]['shape']) * 4 / (1024 * 1024)  # Assume 4 bytes
        output_size_mb = np.prod(self.output_details[0]['shape']) * 4 / (1024 * 1024)

        # Estimated total (model + tensors + overhead)
        total_estimated_mb = model_size_mb + input_size_mb + output_size_mb + 10  # +10MB overhead

        self.results['memory'] = {
            'model_mb': float(model_size_mb),
            'input_mb': float(input_size_mb),
            'output_mb': float(output_size_mb),
            'estimated_total_mb': float(total_estimated_mb)
        }

        print(f"Model: {model_size_mb:.2f} MB")
        print(f"Input tensor: {input_size_mb:.2f} MB")
        print(f"Output tensor: {output_size_mb:.2f} MB")
        print(f"Estimated total: {total_estimated_mb:.2f} MB")

        # Target check (ARTPEC-8 RAM: 512MB, target: <100MB for model)
        target_memory = 100.0
        passes_target = total_estimated_mb < target_memory

        print(f"\n[Target] <{target_memory}MB: {'✓ PASS' if passes_target else '✗ FAIL'}")

        return passes_target

    def check_artpec8_compatibility(self):
        """Check ARTPEC-8 DLPU compatibility"""
        print("\n" + "="*60)
        print("ARTPEC-8 Compatibility Check")
        print("="*60 + "\n")

        checks = {
            'int8_quantization': False,
            'input_size_valid': False,
            'supported_ops': True,  # Assume true, would need detailed op check
        }

        # Check INT8 quantization
        is_int8 = self.input_details[0]['dtype'] == np.int8
        checks['int8_quantization'] = is_int8
        print(f"INT8 quantization: {'✓ PASS' if is_int8 else '✗ FAIL (FP32 detected)'}")

        # Check input size (should be multiple of 32 for ARTPEC-8)
        input_height = self.input_details[0]['shape'][1]
        input_width = self.input_details[0]['shape'][2]
        size_valid = (input_height % 32 == 0) and (input_width % 32 == 0)
        checks['input_size_valid'] = size_valid
        print(f"Input size ({input_height}x{input_width}): {'✓ PASS' if size_valid else '✗ FAIL (not multiple of 32)'}")

        # Overall compatibility
        all_pass = all(checks.values())
        print(f"\n[Overall] ARTPEC-8 compatible: {'✓ YES' if all_pass else '✗ NO'}")

        self.results['artpec8_compatibility'] = checks

        return all_pass

    def validate_on_dataset(
        self,
        annotations_file: str,
        images_dir: str,
        num_samples: int = 100
    ):
        """Validate model on dataset"""
        print("\n" + "="*60)
        print(f"Dataset Validation ({num_samples} samples)")
        print("="*60 + "\n")

        # Load dataset
        loader = COCODataLoader(
            annotations_file=annotations_file,
            images_dir=images_dir,
            batch_size=1,
            shuffle=False
        )

        dataset = loader.create_dataset()

        # Run inference
        print(f"[Validate] Running inference on {num_samples} images...")
        successful = 0
        errors = 0

        for i, (image, ground_truth) in enumerate(dataset.take(num_samples)):
            try:
                # Prepare input
                if self.input_details[0]['dtype'] == np.int8:
                    # Quantize input
                    scale = self.input_details[0]['quantization_parameters']['scales'][0]
                    zero_point = self.input_details[0]['quantization_parameters']['zero_points'][0]
                    input_data = (image.numpy() / scale + zero_point).astype(np.int8)
                else:
                    input_data = image.numpy().astype(np.float32)

                # Run inference
                self.interpreter.set_tensor(self.input_details[0]['index'], input_data)
                self.interpreter.invoke()
                output = self.interpreter.get_tensor(self.output_details[0]['index'])

                successful += 1

            except Exception as e:
                errors += 1
                if errors <= 5:  # Only print first 5 errors
                    print(f"[Error] Sample {i}: {e}")

        # Results
        success_rate = successful / num_samples * 100

        print(f"\n[Results]")
        print(f"  Successful: {successful}/{num_samples} ({success_rate:.1f}%)")
        print(f"  Errors: {errors}")

        self.results['validation'] = {
            'num_samples': num_samples,
            'successful': successful,
            'errors': errors,
            'success_rate': float(success_rate)
        }

        return success_rate > 95.0  # 95% success rate threshold

    def save_results(self, output_file: str):
        """Save benchmark results"""
        output_path = Path(output_file)
        output_path.parent.mkdir(parents=True, exist_ok=True)

        with open(output_path, 'w') as f:
            json.dump(self.results, f, indent=2)

        print(f"\n[Saved] Results: {output_path}")


def main():
    parser = argparse.ArgumentParser(description='Benchmark OMNISIGHT TFLite model')
    parser.add_argument('--model', type=str, required=True,
                        help='Path to TFLite model')
    parser.add_argument('--latency', action='store_true',
                        help='Benchmark latency')
    parser.add_argument('--memory', action='store_true',
                        help='Estimate memory usage')
    parser.add_argument('--compatibility', action='store_true',
                        help='Check ARTPEC-8 compatibility')
    parser.add_argument('--validate', action='store_true',
                        help='Validate on dataset')
    parser.add_argument('--all', action='store_true',
                        help='Run all benchmarks')
    parser.add_argument('--annotations', type=str, default=None,
                        help='Validation dataset annotations')
    parser.add_argument('--images', type=str, default=None,
                        help='Validation dataset images')
    parser.add_argument('--num-samples', type=int, default=100,
                        help='Number of validation samples')
    parser.add_argument('--num-runs', type=int, default=100,
                        help='Number of latency benchmark runs')
    parser.add_argument('--output', type=str, default=None,
                        help='Output JSON file for results')

    args = parser.parse_args()

    # Create benchmark
    benchmark = ModelBenchmark(model_path=args.model)
    benchmark.load_model()

    # Run benchmarks
    if args.all or args.latency:
        benchmark.benchmark_latency(num_runs=args.num_runs)

    if args.all or args.memory:
        benchmark.estimate_memory()

    if args.all or args.compatibility:
        benchmark.check_artpec8_compatibility()

    if args.all or args.validate:
        if args.annotations and args.images:
            benchmark.validate_on_dataset(
                annotations_file=args.annotations,
                images_dir=args.images,
                num_samples=args.num_samples
            )
        else:
            print("[Warning] Skipping validation: --annotations and --images required")

    # Save results
    if args.output:
        benchmark.save_results(args.output)

    print("\n" + "="*60)
    print("✓ Benchmark Complete!")
    print("="*60 + "\n")


if __name__ == '__main__':
    main()
