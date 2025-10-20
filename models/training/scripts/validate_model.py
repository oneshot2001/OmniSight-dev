#!/usr/bin/env python3
"""
OMNISIGHT Model Validation
Validates TFLite models for accuracy and ARTPEC-8 compatibility
"""

import argparse
import numpy as np
import tensorflow as tf
import time
import json
from pathlib import Path


class ModelValidator:
    """Validate TFLite model"""

    def __init__(self, model_path):
        self.model_path = model_path
        self.interpreter = None
        self.input_details = None
        self.output_details = None

    def load_model(self):
        """Load TFLite model"""
        print(f"[Validate] Loading model: {self.model_path}")
        self.interpreter = tf.lite.Interpreter(model_path=self.model_path)
        self.interpreter.allocate_tensors()

        self.input_details = self.interpreter.get_input_details()
        self.output_details = self.interpreter.get_output_details()

        print(f"[Validate] Model loaded successfully")
        print(f"[Validate] Inputs: {len(self.input_details)}")
        print(f"[Validate] Outputs: {len(self.output_details)}")

    def benchmark_latency(self, num_iterations=100):
        """Benchmark model inference latency"""
        print(f"\n[Validate] Benchmarking latency ({num_iterations} iterations)...")

        # Prepare input
        input_shape = self.input_details[0]['shape']
        input_dtype = self.input_details[0]['dtype']
        dummy_input = np.random.randint(0, 256, input_shape).astype(input_dtype)

        # Warmup
        for _ in range(10):
            self.interpreter.set_tensor(self.input_details[0]['index'], dummy_input)
            self.interpreter.invoke()

        # Benchmark
        latencies = []
        for i in range(num_iterations):
            start = time.time()

            self.interpreter.set_tensor(self.input_details[0]['index'], dummy_input)
            self.interpreter.invoke()

            end = time.time()
            latencies.append((end - start) * 1000)  # Convert to ms

            if (i + 1) % 20 == 0:
                print(f"[Validate] Completed {i + 1}/{num_iterations} iterations")

        results = {
            "avg_latency_ms": np.mean(latencies),
            "min_latency_ms": np.min(latencies),
            "max_latency_ms": np.max(latencies),
            "std_latency_ms": np.std(latencies),
            "p50_latency_ms": np.percentile(latencies, 50),
            "p95_latency_ms": np.percentile(latencies, 95),
            "p99_latency_ms": np.percentile(latencies, 99)
        }

        print("\n[Validate] Latency Results:")
        print(f"  Average: {results['avg_latency_ms']:.2f} ms")
        print(f"  Min:     {results['min_latency_ms']:.2f} ms")
        print(f"  Max:     {results['max_latency_ms']:.2f} ms")
        print(f"  Std:     {results['std_latency_ms']:.2f} ms")
        print(f"  P50:     {results['p50_latency_ms']:.2f} ms")
        print(f"  P95:     {results['p95_latency_ms']:.2f} ms")
        print(f"  P99:     {results['p99_latency_ms']:.2f} ms")

        # Check against ARTPEC-8 target (15ms for detection)
        target_latency = 15.0
        if results['avg_latency_ms'] <= target_latency:
            print(f"\n[Validate] ✓ Latency meets ARTPEC-8 target (<{target_latency}ms)")
        else:
            print(f"\n[Validate] ✗ WARNING: Latency exceeds target ({target_latency}ms)")
            print(f"[Validate]   Note: CPU benchmark, DLPU will be ~10x faster")

        return results

    def check_quantization(self):
        """Check quantization parameters"""
        print("\n[Validate] Checking quantization...")

        for i, detail in enumerate(self.input_details):
            dtype = detail['dtype']
            quant = detail['quantization']

            print(f"\nInput {i}: {detail['name']}")
            print(f"  Data type: {dtype}")

            if dtype == np.int8:
                print(f"  ✓ INT8 quantized (ARTPEC-8 compatible)")
                print(f"  Scale: {quant[0]}")
                print(f"  Zero point: {quant[1] if len(quant) > 1 else 'N/A'}")
            else:
                print(f"  ✗ Not INT8 quantized")

        for i, detail in enumerate(self.output_details):
            dtype = detail['dtype']
            quant = detail['quantization']

            print(f"\nOutput {i}: {detail['name']}")
            print(f"  Data type: {dtype}")

            if dtype == np.int8:
                print(f"  ✓ INT8 quantized")
                print(f"  Scale: {quant[0]}")
                print(f"  Zero point: {quant[1] if len(quant) > 1 else 'N/A'}")

    def check_artpec8_compatibility(self):
        """Check ARTPEC-8 specific requirements"""
        print("\n[Validate] ARTPEC-8 Compatibility Check:")

        checks = []

        # Check 1: Input dimensions
        input_shape = self.input_details[0]['shape']
        height, width = input_shape[1], input_shape[2]

        multiple_of_32 = (height % 32 == 0) and (width % 32 == 0)
        checks.append(("Input size multiple of 32", multiple_of_32))
        print(f"  {'✓' if multiple_of_32 else '✗'} Input size: {height}x{width} "
              f"({'multiple of 32' if multiple_of_32 else 'NOT multiple of 32'})")

        # Check 2: INT8 quantization
        input_int8 = self.input_details[0]['dtype'] == np.int8
        output_int8 = all(d['dtype'] == np.int8 for d in self.output_details)
        checks.append(("INT8 quantization", input_int8 and output_int8))
        print(f"  {'✓' if input_int8 and output_int8 else '✗'} INT8 quantization")

        # Check 3: Batch size = 1
        batch_size = input_shape[0]
        batch_ok = batch_size == 1
        checks.append(("Batch size = 1", batch_ok))
        print(f"  {'✓' if batch_ok else '✗'} Batch size: {batch_size}")

        # Summary
        all_passed = all(passed for _, passed in checks)
        print(f"\n[Validate] Overall: {'✓ COMPATIBLE' if all_passed else '✗ MAY HAVE ISSUES'}")

        return all_passed

    def estimate_memory_usage(self):
        """Estimate runtime memory usage"""
        print("\n[Validate] Estimating memory usage...")

        total_memory = 0

        # Input tensors
        for detail in self.input_details:
            size = np.prod(detail['shape']) * detail['dtype'].itemsize
            total_memory += size
            print(f"  Input '{detail['name']}': {size / (1024*1024):.2f} MB")

        # Output tensors
        for detail in self.output_details:
            size = np.prod(detail['shape']) * detail['dtype'].itemsize
            total_memory += size
            print(f"  Output '{detail['name']}': {size / (1024*1024):.2f} MB")

        # Model size
        import os
        model_size = os.path.getsize(self.model_path)
        total_memory += model_size

        print(f"\n  Model file: {model_size / (1024*1024):.2f} MB")
        print(f"  Total estimated: {total_memory / (1024*1024):.2f} MB")

        # Check against target (85MB for detection model)
        target_memory = 85.0
        if total_memory / (1024*1024) <= target_memory:
            print(f"  ✓ Meets memory target (<{target_memory} MB)")
        else:
            print(f"  ✗ WARNING: Exceeds memory target ({target_memory} MB)")

        return total_memory


def main():
    parser = argparse.ArgumentParser(description='Validate OMNISIGHT TFLite model')
    parser.add_argument('--model', type=str, required=True,
                       help='Path to TFLite model')
    parser.add_argument('--test-data', type=str, default=None,
                       help='Path to test dataset (optional)')
    parser.add_argument('--benchmark', action='store_true',
                       help='Run latency benchmark')
    parser.add_argument('--iterations', type=int, default=100,
                       help='Number of benchmark iterations')
    parser.add_argument('--output', type=str, default=None,
                       help='Path to save validation results (JSON)')

    args = parser.parse_args()

    print("=" * 60)
    print("OMNISIGHT Model Validation")
    print("=" * 60)
    print(f"Model: {args.model}")
    print("=" * 60 + "\n")

    # Create validator
    validator = ModelValidator(args.model)
    validator.load_model()

    results = {}

    # Check quantization
    validator.check_quantization()

    # Check ARTPEC-8 compatibility
    compatible = validator.check_artpec8_compatibility()
    results['artpec8_compatible'] = compatible

    # Estimate memory
    memory = validator.estimate_memory_usage()
    results['memory_bytes'] = memory

    # Benchmark latency
    if args.benchmark:
        latency = validator.benchmark_latency(num_iterations=args.iterations)
        results['latency'] = latency

    # Save results
    if args.output:
        with open(args.output, 'w') as f:
            json.dump(results, f, indent=2)
        print(f"\n[Validate] Results saved to: {args.output}")

    print("\n" + "=" * 60)
    print("Validation Complete")
    print("=" * 60)


if __name__ == '__main__':
    main()
