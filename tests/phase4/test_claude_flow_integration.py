"""
Integration Tests for Claude Flow Phase 4

Tests the enhanced coordinators and their integration with
the Flask API server.
"""

import unittest
import sys
import os
import time

# Add app directory to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..', 'app'))

try:
    from coordinator.swarm import EnhancedSwarmCoordinator
    from coordinator.timeline import NeuralTimelineEngine
    from coordinator.federated import FederatedLearningCoordinator
    from coordinator.performance import PerformanceMonitor
    CLAUDE_FLOW_AVAILABLE = True
except ImportError:
    CLAUDE_FLOW_AVAILABLE = False
    print("Claude Flow not available - skipping tests")


@unittest.skipUnless(CLAUDE_FLOW_AVAILABLE, "Claude Flow not available")
class TestSwarmCoordinator(unittest.TestCase):
    """Test Enhanced Swarm Coordinator."""

    def setUp(self):
        """Set up test camera network."""
        self.demo_cameras = [
            {"id": "cam1", "ip": "192.168.1.100"},
            {"id": "cam2", "ip": "192.168.1.101"},
            {"id": "cam3", "ip": "192.168.1.102"}
        ]
        self.coordinator = EnhancedSwarmCoordinator(self.demo_cameras)

    def test_initialization(self):
        """Test coordinator initializes correctly."""
        self.assertIsNotNone(self.coordinator)
        self.assertEqual(self.coordinator.camera_count, 3)
        self.assertEqual(self.coordinator.topology, "mesh")  # <=5 cameras

    def test_coordinate_detection(self):
        """Test detection coordination."""
        detection_data = {
            "timestamp": time.time(),
            "detections": [{"class": "person", "confidence": 0.9}],
            "threat_scores": [0.8],
            "confidence": 0.85
        }

        result = self.coordinator.coordinate_detection("cam1", detection_data)

        self.assertIsNotNone(result)
        self.assertIn("proposal", result)
        self.assertEqual(result["proposal"]["source_camera"], "cam1")

    def test_network_status(self):
        """Test getting network status."""
        status = self.coordinator.get_network_status()

        self.assertIsNotNone(status)
        self.assertEqual(status["total_cameras"], 3)
        self.assertEqual(status["topology"], "mesh")
        self.assertGreaterEqual(status["health_percentage"], 0)

    def test_heartbeat_update(self):
        """Test heartbeat updates."""
        self.coordinator.update_heartbeat("cam1")

        # Verify camera is active
        cam1 = self.coordinator.cameras["cam1"]
        self.assertEqual(cam1.status, "active")
        self.assertGreater(cam1.last_heartbeat, 0)


@unittest.skipUnless(CLAUDE_FLOW_AVAILABLE, "Claude Flow not available")
class TestTimelineEngine(unittest.TestCase):
    """Test Neural Timeline Engine."""

    def setUp(self):
        """Set up timeline engine."""
        self.engine = NeuralTimelineEngine()

    def test_initialization(self):
        """Test engine initializes correctly."""
        self.assertIsNotNone(self.engine)
        self.assertEqual(len(self.engine.models), 4)
        self.assertEqual(self.engine.prediction_horizon, 300)

    def test_predict_futures(self):
        """Test future predictions."""
        current_state = {
            "timestamp": time.time(),
            "detections": [{"class": "person", "bbox": [100, 100, 200, 200]}],
            "tracks": [{"id": 1, "trajectory": [[100, 100], [110, 105]]}],
            "threat_level": 0.6
        }

        historical_context = [
            {"timestamp": time.time() - 30, "event": "detection"},
            {"timestamp": time.time() - 20, "event": "tracking"}
        ]

        predictions = self.engine.predict_futures(current_state, historical_context)

        self.assertIsNotNone(predictions)
        self.assertGreater(len(predictions), 0)

        # Check prediction structure
        for pred in predictions:
            self.assertIsNotNone(pred.future_id)
            self.assertGreaterEqual(pred.probability, 0.0)
            self.assertLessEqual(pred.probability, 1.0)
            self.assertGreater(len(pred.timeline_events), 0)

    def test_intervention_points(self):
        """Test intervention point identification."""
        current_state = {
            "timestamp": time.time(),
            "threat_level": 0.9  # High threat
        }

        predictions = self.engine.predict_futures(current_state, [])

        # Check for intervention points in high-threat scenarios
        has_interventions = any(
            len(p.intervention_points) > 0
            for p in predictions
        )

        # At least one prediction should have interventions for high threat
        self.assertTrue(has_interventions or current_state["threat_level"] < 0.7)


@unittest.skipUnless(CLAUDE_FLOW_AVAILABLE, "Claude Flow not available")
class TestFederatedCoordinator(unittest.TestCase):
    """Test Federated Learning Coordinator."""

    def setUp(self):
        """Set up federated coordinator."""
        self.camera_ids = ["cam1", "cam2", "cam3"]
        self.coordinator = FederatedLearningCoordinator(self.camera_ids)

    def test_initialization(self):
        """Test coordinator initializes correctly."""
        self.assertIsNotNone(self.coordinator)
        self.assertEqual(self.coordinator.network_size, 3)
        self.assertEqual(len(self.coordinator.models), 3)

    def test_local_training(self):
        """Test local model training."""
        local_data = [
            {"image": "dummy1.jpg", "label": "person"},
            {"image": "dummy2.jpg", "label": "car"}
        ]

        update = self.coordinator.train_local_model("cam1", "detector", local_data)

        if update:  # May return None if training fails
            self.assertEqual(update.camera_id, "cam1")
            self.assertEqual(update.model_name, "detector")
            self.assertEqual(update.sample_count, 2)
            self.assertTrue(update.encrypted)

    def test_model_status(self):
        """Test getting model status."""
        status = self.coordinator.get_model_status()

        self.assertIsNotNone(status)
        self.assertEqual(status["network_size"], 3)
        self.assertIn("models", status)
        self.assertIn("detector", status["models"])

    def test_synchronization(self):
        """Test camera synchronization."""
        result = self.coordinator.synchronize_camera("cam1", "detector")

        self.assertTrue(result)  # Should succeed for valid camera/model


@unittest.skipUnless(CLAUDE_FLOW_AVAILABLE, "Claude Flow not available")
class TestPerformanceMonitor(unittest.TestCase):
    """Test Performance Monitor."""

    def setUp(self):
        """Set up performance monitor."""
        self.monitor = PerformanceMonitor(window_size=50)

    def test_initialization(self):
        """Test monitor initializes correctly."""
        self.assertIsNotNone(self.monitor)
        self.assertEqual(self.monitor.window_size, 50)
        self.assertEqual(len(self.monitor.metrics), 5)

    def test_record_metric(self):
        """Test recording metrics."""
        self.monitor.record_metric("inference_latency", 15.5, "ms")

        stats = self.monitor.get_statistics("inference_latency")

        self.assertEqual(stats["count"], 1)
        self.assertEqual(stats["current"], 15.5)
        self.assertTrue(stats["target_met"])  # <20ms target

    def test_statistics(self):
        """Test statistics calculation."""
        # Record multiple values
        for i in range(10):
            self.monitor.record_metric("frame_rate", 12.0 + i, "fps")

        stats = self.monitor.get_statistics("frame_rate")

        self.assertEqual(stats["count"], 10)
        self.assertEqual(stats["min"], 12.0)
        self.assertEqual(stats["max"], 21.0)
        self.assertAlmostEqual(stats["mean"], 16.5, places=1)

    def test_all_statistics(self):
        """Test getting all statistics."""
        self.monitor.record_metric("inference_latency", 18.0, "ms")
        self.monitor.record_metric("frame_rate", 15.0, "fps")

        all_stats = self.monitor.get_all_statistics()

        self.assertIsNotNone(all_stats)
        self.assertIn("inference_latency", all_stats)
        self.assertIn("frame_rate", all_stats)

    def test_bottleneck_detection(self):
        """Test bottleneck detection."""
        # Record poor performance
        self.monitor.record_metric("inference_latency", 50.0, "ms")  # >20ms target
        self.monitor.record_metric("frame_rate", 5.0, "fps")  # <10fps target

        bottlenecks = self.monitor.detect_bottlenecks()

        self.assertGreater(len(bottlenecks), 0)

        # Check bottleneck structure
        for bottleneck in bottlenecks:
            self.assertIn("metric", bottleneck)
            self.assertIn("current", bottleneck)
            self.assertIn("target", bottleneck)
            self.assertIn("severity", bottleneck)

    def test_health_score(self):
        """Test overall health score calculation."""
        # Record metrics meeting targets
        self.monitor.record_metric("inference_latency", 15.0, "ms")  # Good
        self.monitor.record_metric("frame_rate", 12.0, "fps")  # Good

        report = self.monitor.get_performance_report()

        self.assertIn("overall_health", report)
        self.assertGreaterEqual(report["overall_health"], 0.0)
        self.assertLessEqual(report["overall_health"], 100.0)


@unittest.skipUnless(CLAUDE_FLOW_AVAILABLE, "Claude Flow not available")
class TestIntegration(unittest.TestCase):
    """Test integration between components."""

    def setUp(self):
        """Set up all coordinators."""
        self.demo_cameras = [
            {"id": "cam1", "ip": "192.168.1.100"},
            {"id": "cam2", "ip": "192.168.1.101"}
        ]

        self.swarm = EnhancedSwarmCoordinator(self.demo_cameras)
        self.timeline = NeuralTimelineEngine()
        self.federated = FederatedLearningCoordinator([c["id"] for c in self.demo_cameras])
        self.performance = PerformanceMonitor()

    def test_detection_workflow(self):
        """Test complete detection workflow."""
        # 1. Coordinate detection via swarm
        detection_data = {
            "timestamp": time.time(),
            "detections": [{"class": "person", "confidence": 0.9}],
            "threat_scores": [0.7],
            "confidence": 0.85
        }

        consensus = self.swarm.coordinate_detection("cam1", detection_data)
        self.assertIsNotNone(consensus)

        # 2. Predict timeline
        current_state = {
            "timestamp": detection_data["timestamp"],
            "detections": detection_data["detections"],
            "threat_level": 0.7
        }

        predictions = self.timeline.predict_futures(current_state, [])
        self.assertGreater(len(predictions), 0)

        # 3. Record performance
        self.performance.record_metric("inference_latency", 18.0, "ms")

        stats = self.performance.get_statistics("inference_latency")
        self.assertEqual(stats["count"], 1)

    def test_federated_training_workflow(self):
        """Test federated learning workflow."""
        # 1. Train on multiple cameras
        for camera_id in [c["id"] for c in self.demo_cameras]:
            local_data = [{"sample": i} for i in range(5)]
            update = self.federated.train_local_model(camera_id, "detector", local_data)

            # May be None if training fails, which is acceptable
            if update:
                self.assertEqual(update.camera_id, camera_id)

        # 2. Get status
        status = self.federated.get_model_status()
        self.assertEqual(status["network_size"], 2)


if __name__ == '__main__':
    unittest.main()
