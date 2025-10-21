#!/usr/bin/env python3
"""
COCO Dataset Loader for OMNISIGHT Training
TensorFlow data pipeline optimized for ARTPEC-8 model training
"""

import os
import json
import numpy as np
import tensorflow as tf
from pathlib import Path
from typing import List, Dict, Tuple
import cv2


class COCODataLoader:
    """COCO dataset loader with ARTPEC-8 optimizations"""

    def __init__(
        self,
        annotations_file: str,
        images_dir: str,
        target_size: Tuple[int, int] = (416, 416),
        num_classes: int = 6,
        batch_size: int = 16,
        shuffle: bool = True
    ):
        self.annotations_file = Path(annotations_file)
        self.images_dir = Path(images_dir)
        self.target_size = target_size
        self.num_classes = num_classes
        self.batch_size = batch_size
        self.shuffle = shuffle

        # Load COCO annotations
        self.coco_data = self._load_annotations()
        self.image_infos = self.coco_data['images']
        self.annotations = self.coco_data['annotations']
        self.categories = self.coco_data['categories']

        # Create category mapping
        self.cat_id_to_idx = self._create_category_mapping()

        # Create image_id to annotations mapping
        self.image_to_anns = self._group_annotations()

        print(f"[COCODataLoader] Loaded {len(self.image_infos)} images")
        print(f"[COCODataLoader] Categories: {len(self.categories)}")
        print(f"[COCODataLoader] Annotations: {len(self.annotations)}")

    def _load_annotations(self) -> Dict:
        """Load COCO annotations JSON"""
        with open(self.annotations_file, 'r') as f:
            return json.load(f)

    def _create_category_mapping(self) -> Dict[int, int]:
        """Map COCO category IDs to model class indices"""
        # OMNISIGHT categories (0-5)
        omnisight_categories = {
            'person': 0,
            'vehicle': 1,  # car, truck, bus
            'face': 2,
            'package': 3,  # backpack, suitcase, handbag
            'animal': 4,   # dog, cat, bird, etc.
            'other': 5
        }

        # COCO to OMNISIGHT mapping
        coco_to_omnisight = {}
        for cat in self.categories:
            name = cat['name']
            cat_id = cat['id']

            if name == 'person':
                coco_to_omnisight[cat_id] = 0
            elif name in ['car', 'truck', 'bus', 'motorcycle', 'bicycle']:
                coco_to_omnisight[cat_id] = 1
            elif name in ['backpack', 'suitcase', 'handbag']:
                coco_to_omnisight[cat_id] = 3
            elif name in ['dog', 'cat', 'bird', 'horse', 'sheep', 'cow', 'elephant', 'bear', 'zebra', 'giraffe']:
                coco_to_omnisight[cat_id] = 4
            else:
                coco_to_omnisight[cat_id] = 5  # other

        return coco_to_omnisight

    def _group_annotations(self) -> Dict[int, List]:
        """Group annotations by image_id"""
        image_to_anns = {}
        for ann in self.annotations:
            image_id = ann['image_id']
            if image_id not in image_to_anns:
                image_to_anns[image_id] = []
            image_to_anns[image_id].append(ann)
        return image_to_anns

    def _parse_annotation(self, image_info: Dict) -> Tuple[np.ndarray, np.ndarray, np.ndarray]:
        """Parse annotations for an image"""
        image_id = image_info['id']
        anns = self.image_to_anns.get(image_id, [])

        if len(anns) == 0:
            return np.zeros((0, 4)), np.zeros((0,), dtype=np.int32), np.zeros((0,))

        boxes = []
        classes = []
        scores = []

        for ann in anns:
            # COCO bbox format: [x, y, width, height]
            x, y, w, h = ann['bbox']

            # Convert to [x1, y1, x2, y2]
            x1 = x / image_info['width']
            y1 = y / image_info['height']
            x2 = (x + w) / image_info['width']
            y2 = (y + h) / image_info['height']

            # Clip to [0, 1]
            x1 = max(0, min(1, x1))
            y1 = max(0, min(1, y1))
            x2 = max(0, min(1, x2))
            y2 = max(0, min(1, y2))

            # Skip invalid boxes
            if x2 <= x1 or y2 <= y1:
                continue

            boxes.append([x1, y1, x2, y2])
            classes.append(self.cat_id_to_idx.get(ann['category_id'], 5))
            scores.append(1.0)  # Ground truth

        return (
            np.array(boxes, dtype=np.float32),
            np.array(classes, dtype=np.int32),
            np.array(scores, dtype=np.float32)
        )

    def _load_and_preprocess_image(self, image_path: str) -> np.ndarray:
        """Load and preprocess image"""
        # Load image
        img = cv2.imread(str(image_path))
        if img is None:
            raise ValueError(f"Failed to load image: {image_path}")

        # Convert BGR to RGB
        img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)

        # Resize to target size
        img = cv2.resize(img, self.target_size)

        # Normalize to [0, 1] (will be quantized to INT8 later)
        img = img.astype(np.float32) / 255.0

        return img

    def _augment_image(self, image: np.ndarray, boxes: np.ndarray) -> Tuple[np.ndarray, np.ndarray]:
        """Apply data augmentation"""
        # Random horizontal flip
        if np.random.rand() < 0.5:
            image = np.fliplr(image)
            if len(boxes) > 0:
                boxes[:, [0, 2]] = 1.0 - boxes[:, [2, 0]]

        # Random brightness
        if np.random.rand() < 0.5:
            factor = np.random.uniform(0.8, 1.2)
            image = np.clip(image * factor, 0, 1)

        # Random contrast
        if np.random.rand() < 0.5:
            factor = np.random.uniform(0.8, 1.2)
            mean = np.mean(image)
            image = np.clip((image - mean) * factor + mean, 0, 1)

        return image, boxes

    def _generator(self):
        """Python generator for dataset"""
        indices = np.arange(len(self.image_infos))
        if self.shuffle:
            np.random.shuffle(indices)

        for idx in indices:
            image_info = self.image_infos[idx]
            image_path = self.images_dir / image_info['file_name']

            try:
                # Load image
                image = self._load_and_preprocess_image(image_path)

                # Get annotations
                boxes, classes, scores = self._parse_annotation(image_info)

                # Augmentation
                if self.shuffle:  # Only augment during training
                    image, boxes = self._augment_image(image, boxes)

                # Encode detections (for SSD-style output)
                detections = self._encode_detections(boxes, classes, scores)

                yield image, detections

            except Exception as e:
                print(f"[Warning] Failed to process {image_path}: {e}")
                continue

    def _encode_detections(self, boxes: np.ndarray, classes: np.ndarray, scores: np.ndarray) -> np.ndarray:
        """Encode detections for training (simplified SSD encoding)"""
        # For simplicity, we'll return boxes, classes, scores in a fixed-size array
        # Shape: [max_detections, 6] where each row is [x1, y1, x2, y2, class, score]
        max_detections = 100
        detections = np.zeros((max_detections, 6), dtype=np.float32)

        num_dets = min(len(boxes), max_detections)
        if num_dets > 0:
            detections[:num_dets, :4] = boxes[:num_dets]
            detections[:num_dets, 4] = classes[:num_dets]
            detections[:num_dets, 5] = scores[:num_dets]

        return detections

    def create_dataset(self) -> tf.data.Dataset:
        """Create TensorFlow dataset"""
        output_signature = (
            tf.TensorSpec(shape=(*self.target_size, 3), dtype=tf.float32),  # image
            tf.TensorSpec(shape=(100, 6), dtype=tf.float32)  # detections
        )

        dataset = tf.data.Dataset.from_generator(
            self._generator,
            output_signature=output_signature
        )

        if self.shuffle:
            dataset = dataset.shuffle(buffer_size=1000)

        dataset = dataset.batch(self.batch_size)
        dataset = dataset.prefetch(tf.data.AUTOTUNE)

        return dataset

    def get_num_batches(self) -> int:
        """Get number of batches per epoch"""
        return len(self.image_infos) // self.batch_size


def create_representative_dataset(annotations_file: str, images_dir: str, num_samples: int = 100):
    """Create representative dataset for INT8 quantization"""
    loader = COCODataLoader(
        annotations_file=annotations_file,
        images_dir=images_dir,
        batch_size=1,
        shuffle=False
    )

    def representative_data_gen():
        dataset = loader.create_dataset()
        for i, (image, _) in enumerate(dataset.take(num_samples)):
            yield [image]

    return representative_data_gen


if __name__ == '__main__':
    # Test data loader
    import argparse

    parser = argparse.ArgumentParser(description='Test COCO data loader')
    parser.add_argument('--annotations', type=str, required=True,
                        help='Path to COCO annotations JSON')
    parser.add_argument('--images', type=str, required=True,
                        help='Path to images directory')
    parser.add_argument('--batch-size', type=int, default=4,
                        help='Batch size')

    args = parser.parse_args()

    # Create loader
    loader = COCODataLoader(
        annotations_file=args.annotations,
        images_dir=args.images,
        batch_size=args.batch_size
    )

    # Create dataset
    dataset = loader.create_dataset()

    # Test first batch
    print("\nTesting dataset...")
    for images, detections in dataset.take(1):
        print(f"Images shape: {images.shape}")
        print(f"Detections shape: {detections.shape}")
        print(f"Image range: [{images.numpy().min():.3f}, {images.numpy().max():.3f}]")
        print(f"Detections sample:\n{detections[0, :5].numpy()}")

    print(f"\nDataset created successfully!")
    print(f"Total batches per epoch: {loader.get_num_batches()}")
