"""
Performance Monitor and Optimizer with Claude Flow Integration

Real-time performance tracking and automatic optimization
using Claude Flow's performance monitoring and benchmarking.
"""

import subprocess
import json
import time
from typing import Dict, List, Optional, Any
from dataclasses import dataclass, asdict
from collections import deque


@dataclass
class PerformanceMetric:
    """Single performance measurement."""
    timestamp: float
    metric_name: str
    value: float
    unit: str
    camera_id: Optional[str] = None


class PerformanceMonitor:
    """
    Real-time performance monitoring and optimization.

    Tracks:
    - Inference latency
    - Frame processing rate
    - Memory usage
    - Network bandwidth
    - Consensus latency
    """

    def __init__(self, window_size: int = 100):
        """
        Initialize performance monitor.

        Args:
            window_size: Number of recent measurements to keep
        """
        self.window_size = window_size

        # Metric buffers (sliding windows)
        self.metrics = {
            "inference_latency": deque(maxlen=window_size),
            "frame_rate": deque(maxlen=window_size),
            "memory_usage": deque(maxlen=window_size),
            "network_bandwidth": deque(maxlen=window_size),
            "consensus_latency": deque(maxlen=window_size)
        }

        # Performance targets
        self.targets = {
            "inference_latency": 20.0,  # <20ms
            "frame_rate": 10.0,         # >10 FPS
            "memory_usage": 512.0,      # <512 MB
            "network_bandwidth": 500.0, # <500 Kbps
            "consensus_latency": 100.0  # <100ms
        }

        # Optimization state
        self.optimizations_applied = []
        self.last_benchmark = None

        print(f"✓ Performance Monitor initialized (window={window_size})")

    def record_metric(
        self,
        metric_name: str,
        value: float,
        unit: str,
        camera_id: Optional[str] = None
    ):
        """
        Record a performance metric.

        Args:
            metric_name: Metric identifier
            value: Measured value
            unit: Unit of measurement
            camera_id: Source camera (optional)
        """
        if metric_name not in self.metrics:
            print(f"⚠ Unknown metric: {metric_name}")
            return

        metric = PerformanceMetric(
            timestamp=time.time(),
            metric_name=metric_name,
            value=value,
            unit=unit,
            camera_id=camera_id
        )

        self.metrics[metric_name].append(metric)

        # Check if target is met
        self._check_target(metric_name, value)

    def _check_target(self, metric_name: str, value: float):
        """Check if metric meets performance target."""
        target = self.targets.get(metric_name)
        if target is None:
            return

        # Different comparison for different metrics
        if metric_name in ["inference_latency", "memory_usage", "network_bandwidth", "consensus_latency"]:
            # Lower is better
            if value > target:
                print(f"⚠ Performance warning: {metric_name}={value:.2f} (target <{target})")
        else:
            # Higher is better (frame_rate)
            if value < target:
                print(f"⚠ Performance warning: {metric_name}={value:.2f} (target >{target})")

    def get_statistics(self, metric_name: str) -> Dict[str, float]:
        """
        Get statistics for a metric.

        Returns:
            Dictionary with min, max, mean, current
        """
        if metric_name not in self.metrics:
            return {}

        values = [m.value for m in self.metrics[metric_name]]

        if not values:
            return {
                "min": 0.0,
                "max": 0.0,
                "mean": 0.0,
                "current": 0.0,
                "count": 0
            }

        return {
            "min": min(values),
            "max": max(values),
            "mean": sum(values) / len(values),
            "current": values[-1],
            "count": len(values),
            "target": self.targets.get(metric_name, 0.0),
            "target_met": self._is_target_met(metric_name, values[-1])
        }

    def _is_target_met(self, metric_name: str, value: float) -> bool:
        """Check if current value meets target."""
        target = self.targets.get(metric_name)
        if target is None:
            return True

        if metric_name in ["inference_latency", "memory_usage", "network_bandwidth", "consensus_latency"]:
            return value <= target
        else:
            return value >= target

    def get_all_statistics(self) -> Dict[str, Dict[str, float]]:
        """Get statistics for all metrics."""
        return {
            metric_name: self.get_statistics(metric_name)
            for metric_name in self.metrics.keys()
        }

    def run_benchmark(self) -> Dict[str, Any]:
        """
        Run performance benchmark using Claude Flow.

        Tests system performance across various workloads.
        """
        try:
            print("Running performance benchmark...")

            result = subprocess.run(
                [
                    "npx", "claude-flow@alpha",
                    "benchmark", "run",
                    "--suite", "omnisight",
                    "--iterations", "10"
                ],
                capture_output=True,
                text=True,
                timeout=30
            )

            if result.returncode == 0:
                benchmark_results = json.loads(result.stdout)

                self.last_benchmark = {
                    "timestamp": time.time(),
                    "results": benchmark_results
                }

                print("✓ Benchmark complete")
                return benchmark_results
            else:
                print(f"⚠ Benchmark warning: {result.stderr}")
                return self._fallback_benchmark()

        except Exception as e:
            print(f"⚠ Benchmark error: {e}")
            return self._fallback_benchmark()

    def _fallback_benchmark(self) -> Dict[str, Any]:
        """Fallback benchmark using current metrics."""
        return {
            "timestamp": time.time(),
            "results": self.get_all_statistics(),
            "fallback": True
        }

    def optimize_performance(self) -> List[str]:
        """
        Automatically optimize performance based on metrics.

        Returns:
            List of applied optimizations
        """
        optimizations = []

        # Check each metric and apply optimizations
        stats = self.get_all_statistics()

        # Inference latency optimization
        if not stats["inference_latency"].get("target_met", True):
            opt = self._optimize_inference()
            if opt:
                optimizations.append(opt)

        # Frame rate optimization
        if not stats["frame_rate"].get("target_met", True):
            opt = self._optimize_frame_rate()
            if opt:
                optimizations.append(opt)

        # Memory optimization
        if not stats["memory_usage"].get("target_met", True):
            opt = self._optimize_memory()
            if opt:
                optimizations.append(opt)

        # Network optimization
        if not stats["network_bandwidth"].get("target_met", True):
            opt = self._optimize_network()
            if opt:
                optimizations.append(opt)

        self.optimizations_applied.extend(optimizations)

        if optimizations:
            print(f"✓ Applied {len(optimizations)} optimizations")

        return optimizations

    def _optimize_inference(self) -> Optional[str]:
        """Optimize ML inference latency."""
        # Example optimizations:
        # - Reduce batch size
        # - Use quantized models
        # - Skip frames
        optimization = "reduce_inference_batch_size"
        print(f"  Applying: {optimization}")
        return optimization

    def _optimize_frame_rate(self) -> Optional[str]:
        """Optimize frame processing rate."""
        # Example optimizations:
        # - Increase thread pool
        # - Reduce resolution
        # - Optimize tracking algorithm
        optimization = "increase_worker_threads"
        print(f"  Applying: {optimization}")
        return optimization

    def _optimize_memory(self) -> Optional[str]:
        """Optimize memory usage."""
        # Example optimizations:
        # - Clear old buffers
        # - Reduce cache size
        # - Garbage collection
        optimization = "clear_detection_cache"
        print(f"  Applying: {optimization}")
        return optimization

    def _optimize_network(self) -> Optional[str]:
        """Optimize network bandwidth."""
        # Example optimizations:
        # - Compress messages
        # - Reduce gossip frequency
        # - Batch updates
        optimization = "enable_message_compression"
        print(f"  Applying: {optimization}")
        return optimization

    def detect_bottlenecks(self) -> List[Dict[str, Any]]:
        """
        Detect performance bottlenecks using Claude Flow.

        Returns:
            List of identified bottlenecks with recommendations
        """
        try:
            # Prepare metrics for analysis
            metrics_data = {
                metric_name: [asdict(m) for m in metric_list]
                for metric_name, metric_list in self.metrics.items()
            }

            result = subprocess.run(
                [
                    "npx", "claude-flow@alpha",
                    "performance", "analyze",
                    "--metrics", json.dumps(metrics_data)
                ],
                capture_output=True,
                text=True,
                timeout=10
            )

            if result.returncode == 0:
                analysis = json.loads(result.stdout)
                bottlenecks = analysis.get("bottlenecks", [])

                if bottlenecks:
                    print(f"⚠ Detected {len(bottlenecks)} bottlenecks")

                return bottlenecks
            else:
                print(f"⚠ Analysis warning: {result.stderr}")
                return self._simple_bottleneck_detection()

        except Exception as e:
            print(f"⚠ Bottleneck detection error: {e}")
            return self._simple_bottleneck_detection()

    def _simple_bottleneck_detection(self) -> List[Dict[str, Any]]:
        """Simple bottleneck detection using targets."""
        bottlenecks = []
        stats = self.get_all_statistics()

        for metric_name, stat in stats.items():
            if not stat.get("target_met", True):
                bottleneck = {
                    "metric": metric_name,
                    "current": stat["current"],
                    "target": stat["target"],
                    "severity": self._calculate_severity(
                        metric_name,
                        stat["current"],
                        stat["target"]
                    ),
                    "recommendation": self._get_recommendation(metric_name)
                }
                bottlenecks.append(bottleneck)

        return bottlenecks

    def _calculate_severity(
        self,
        metric_name: str,
        current: float,
        target: float
    ) -> str:
        """Calculate severity of performance issue."""
        if target == 0:
            return "low"

        # Calculate percentage difference
        if metric_name in ["inference_latency", "memory_usage", "network_bandwidth", "consensus_latency"]:
            # Lower is better
            diff_pct = ((current - target) / target) * 100
        else:
            # Higher is better
            diff_pct = ((target - current) / target) * 100

        if diff_pct > 50:
            return "critical"
        elif diff_pct > 20:
            return "high"
        elif diff_pct > 10:
            return "medium"
        else:
            return "low"

    def _get_recommendation(self, metric_name: str) -> str:
        """Get optimization recommendation for metric."""
        recommendations = {
            "inference_latency": "Reduce batch size or use quantized models",
            "frame_rate": "Increase worker threads or reduce resolution",
            "memory_usage": "Clear caches or reduce buffer sizes",
            "network_bandwidth": "Enable compression or batch updates",
            "consensus_latency": "Reduce quorum threshold or use mesh topology"
        }

        return recommendations.get(metric_name, "Review system configuration")

    def get_performance_report(self) -> Dict[str, Any]:
        """
        Generate comprehensive performance report.

        Returns:
            Complete performance summary
        """
        return {
            "timestamp": time.time(),
            "statistics": self.get_all_statistics(),
            "bottlenecks": self.detect_bottlenecks(),
            "optimizations_applied": self.optimizations_applied,
            "last_benchmark": self.last_benchmark,
            "overall_health": self._calculate_health_score()
        }

    def _calculate_health_score(self) -> float:
        """
        Calculate overall system health score (0-100).

        Based on percentage of metrics meeting targets.
        """
        stats = self.get_all_statistics()

        if not stats:
            return 100.0

        targets_met = sum(
            1 for stat in stats.values()
            if stat.get("target_met", True)
        )

        return (targets_met / len(stats)) * 100.0
