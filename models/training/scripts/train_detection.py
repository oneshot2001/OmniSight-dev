#!/usr/bin/env python3
"""
OMNISIGHT Object Detection Model Training
Trains EfficientNet-B4 for ARTPEC-8 DLPU deployment
"""

import os
import sys
import argparse
import yaml
import numpy as np
import tensorflow as tf
from tensorflow import keras
from tensorflow.keras import layers, models
from pathlib import Path
import json
from datetime import datetime

# Add project root to path
sys.path.append(str(Path(__file__).parent.parent.parent))


class OmnisightDetectionModel:
    """Object detection model optimized for ARTPEC-8"""

    def __init__(self, config):
        self.config = config
        self.model = None
        self.history = None

    def build_model(self):
        """Build EfficientNet-B4 detection model"""
        print("[Train] Building model architecture...")

        input_shape = (
            self.config['model']['input']['height'],
            self.config['model']['input']['width'],
            self.config['model']['input']['channels']
        )

        # Input layer
        inputs = layers.Input(shape=input_shape, name='input_image')

        # EfficientNet-B4 backbone (with modifications for ARTPEC-8)
        backbone = self._build_artpec8_backbone(inputs)

        # Detection head (SSD-style)
        detections = self._build_detection_head(backbone)

        self.model = models.Model(inputs=inputs, outputs=detections, name='omnisight_detection')

        print(f"[Train] Model built: {self.model.count_params():,} parameters")
        return self.model

    def _build_artpec8_backbone(self, inputs):
        """Build backbone optimized for ARTPEC-8"""
        config = self.config['model']['artpec8']

        x = inputs

        # Initial convolution (stride 2 for downsampling)
        x = layers.Conv2D(
            filters=48,  # Multiple of 6
            kernel_size=3,
            strides=2,
            padding='same',
            use_bias=False,
            name='stem_conv'
        )(x)
        x = layers.BatchNormalization(name='stem_bn')(x)
        x = layers.ReLU(name='stem_relu')(x)

        # MBConv blocks (modified for ARTPEC-8)
        # Use regular convolutions instead of depthwise for better DLPU performance
        block_configs = [
            # (filters, repeats, stride)
            (96, 2, 2),   # Stage 1
            (144, 4, 2),  # Stage 2
            (240, 4, 2),  # Stage 3
            (384, 6, 1),  # Stage 4
        ]

        for stage_idx, (filters, repeats, stride) in enumerate(block_configs):
            for block_idx in range(repeats):
                block_stride = stride if block_idx == 0 else 1
                x = self._mbconv_block_artpec8(
                    x,
                    filters=filters,
                    stride=block_stride,
                    name=f'block{stage_idx}_{block_idx}'
                )

        return x

    def _mbconv_block_artpec8(self, inputs, filters, stride, name):
        """Mobile Inverted Bottleneck Conv block optimized for ARTPEC-8"""
        # Use regular convolutions (not depthwise) for ARTPEC-8 efficiency

        # Expansion
        expanded = layers.Conv2D(
            filters=filters,
            kernel_size=1,
            padding='same',
            use_bias=False,
            name=f'{name}_expand'
        )(inputs)
        expanded = layers.BatchNormalization(name=f'{name}_expand_bn')(expanded)
        expanded = layers.ReLU(name=f'{name}_expand_relu')(expanded)

        # 3x3 convolution (optimal for ARTPEC-8)
        x = layers.Conv2D(
            filters=filters,
            kernel_size=3,
            strides=stride,
            padding='same',
            use_bias=False,
            name=f'{name}_conv'
        )(expanded)
        x = layers.BatchNormalization(name=f'{name}_bn')(x)
        x = layers.ReLU(name=f'{name}_relu')(x)

        # Projection
        x = layers.Conv2D(
            filters=filters,
            kernel_size=1,
            padding='same',
            use_bias=False,
            name=f'{name}_project'
        )(x)
        x = layers.BatchNormalization(name=f'{name}_project_bn')(x)

        # Residual connection
        if stride == 1 and inputs.shape[-1] == filters:
            x = layers.Add(name=f'{name}_add')([inputs, x])

        return x

    def _build_detection_head(self, backbone):
        """Build SSD detection head"""
        num_classes = self.config['model']['output']['num_classes']
        num_anchors = self.config['model']['head']['num_anchors']

        # Multi-scale feature maps
        # For simplicity, using single scale - extend for multi-scale SSD
        x = backbone

        # Classification head
        cls_head = layers.Conv2D(
            filters=num_anchors * num_classes,
            kernel_size=3,
            padding='same',
            name='cls_head'
        )(x)
        cls_head = layers.Reshape((-1, num_classes), name='cls_reshape')(cls_head)
        cls_head = layers.Activation('softmax', name='cls_output')(cls_head)

        # Localization head (bounding boxes)
        loc_head = layers.Conv2D(
            filters=num_anchors * 4,  # 4 coordinates per box
            kernel_size=3,
            padding='same',
            name='loc_head'
        )(x)
        loc_head = layers.Reshape((-1, 4), name='loc_reshape')(loc_head)

        # Confidence head
        conf_head = layers.Conv2D(
            filters=num_anchors * 1,
            kernel_size=3,
            padding='same',
            name='conf_head'
        )(x)
        conf_head = layers.Reshape((-1, 1), name='conf_reshape')(conf_head)
        conf_head = layers.Activation('sigmoid', name='conf_output')(conf_head)

        # Combine outputs
        outputs = {
            'boxes': loc_head,
            'classes': cls_head,
            'scores': conf_head
        }

        return outputs

    def compile_model(self):
        """Compile model with optimizer and losses"""
        print("[Train] Compiling model...")

        train_config = self.config['training']

        # Optimizer
        optimizer = keras.optimizers.Adam(
            learning_rate=train_config['learning_rate']
        )

        # Losses
        losses = {
            'boxes': keras.losses.Huber(),
            'classes': keras.losses.CategoricalCrossentropy(),
            'scores': keras.losses.BinaryCrossentropy()
        }

        # Loss weights
        loss_weights = {
            'boxes': 1.0,
            'classes': 1.0,
            'scores': 1.0
        }

        self.model.compile(
            optimizer=optimizer,
            loss=losses,
            loss_weights=loss_weights,
            metrics=['accuracy']
        )

        print("[Train] Model compiled")

    def train(self, train_dataset, val_dataset, checkpoint_dir):
        """Train the model"""
        print("[Train] Starting training...")

        train_config = self.config['training']

        # Callbacks
        callbacks = [
            keras.callbacks.ModelCheckpoint(
                os.path.join(checkpoint_dir, 'best_model.h5'),
                save_best_only=True,
                monitor='val_loss',
                mode='min',
                verbose=1
            ),
            keras.callbacks.EarlyStopping(
                monitor='val_loss',
                patience=10,
                restore_best_weights=True,
                verbose=1
            ),
            keras.callbacks.ReduceLROnPlateau(
                monitor='val_loss',
                factor=0.5,
                patience=5,
                min_lr=train_config['lr_schedule']['min_lr'],
                verbose=1
            ),
            keras.callbacks.TensorBoard(
                log_dir=os.path.join(checkpoint_dir, 'logs'),
                histogram_freq=1
            )
        ]

        # Train
        self.history = self.model.fit(
            train_dataset,
            validation_data=val_dataset,
            epochs=train_config['epochs'],
            callbacks=callbacks,
            verbose=1
        )

        print("[Train] Training complete")

        return self.history

    def save_model(self, output_path):
        """Save trained model"""
        self.model.save(output_path)
        print(f"[Train] Model saved to {output_path}")


def load_config(config_path):
    """Load training configuration"""
    with open(config_path, 'r') as f:
        config = yaml.safe_load(f)
    return config


def create_mock_dataset(config, num_samples=1000):
    """Create mock dataset for testing (replace with real data loader)"""
    print(f"[Train] Creating mock dataset ({num_samples} samples)...")

    input_shape = (
        config['model']['input']['height'],
        config['model']['input']['width'],
        config['model']['input']['channels']
    )

    def generator():
        for _ in range(num_samples):
            # Mock image
            image = np.random.randint(0, 256, input_shape, dtype=np.uint8)

            # Mock detections
            num_detections = 5
            boxes = np.random.rand(num_detections, 4).astype(np.float32)
            classes = np.random.randint(0, config['model']['output']['num_classes'],
                                       (num_detections, config['model']['output']['num_classes']))
            scores = np.random.rand(num_detections, 1).astype(np.float32)

            yield (
                image,
                {
                    'boxes': boxes,
                    'classes': classes,
                    'scores': scores
                }
            )

    output_signature = (
        tf.TensorSpec(shape=input_shape, dtype=tf.uint8),
        {
            'boxes': tf.TensorSpec(shape=(None, 4), dtype=tf.float32),
            'classes': tf.TensorSpec(shape=(None, config['model']['output']['num_classes']),
                                    dtype=tf.int32),
            'scores': tf.TensorSpec(shape=(None, 1), dtype=tf.float32)
        }
    )

    dataset = tf.data.Dataset.from_generator(
        generator,
        output_signature=output_signature
    )

    batch_size = config['training']['batch_size']
    dataset = dataset.batch(batch_size).prefetch(tf.data.AUTOTUNE)

    return dataset


def main():
    parser = argparse.ArgumentParser(description='Train OMNISIGHT detection model')
    parser.add_argument('--config', type=str, required=True,
                       help='Path to model configuration YAML')
    parser.add_argument('--data-dir', type=str, default='../data/processed',
                       help='Path to processed training data')
    parser.add_argument('--checkpoint-dir', type=str, default='../checkpoints/detection',
                       help='Directory to save checkpoints')
    parser.add_argument('--epochs', type=int, default=None,
                       help='Number of training epochs (overrides config)')
    parser.add_argument('--batch-size', type=int, default=None,
                       help='Batch size (overrides config)')
    parser.add_argument('--mock-data', action='store_true',
                       help='Use mock data for testing')

    args = parser.parse_args()

    # Load configuration
    config = load_config(args.config)

    # Override config with command line args
    if args.epochs:
        config['training']['epochs'] = args.epochs
    if args.batch_size:
        config['training']['batch_size'] = args.batch_size

    # Create checkpoint directory
    os.makedirs(args.checkpoint_dir, exist_ok=True)

    # Save config to checkpoint dir
    config_save_path = os.path.join(args.checkpoint_dir, 'config.yaml')
    with open(config_save_path, 'w') as f:
        yaml.dump(config, f)

    print("=" * 60)
    print("OMNISIGHT Object Detection Model Training")
    print("=" * 60)
    print(f"Model: {config['model']['name']}")
    print(f"Input: {config['model']['input']['height']}x{config['model']['input']['width']}")
    print(f"Classes: {config['model']['output']['num_classes']}")
    print(f"Epochs: {config['training']['epochs']}")
    print(f"Batch size: {config['training']['batch_size']}")
    print(f"Checkpoint dir: {args.checkpoint_dir}")
    print("=" * 60)

    # Create datasets
    if args.mock_data:
        print("[Train] Using mock data for testing...")
        train_dataset = create_mock_dataset(config, num_samples=1000)
        val_dataset = create_mock_dataset(config, num_samples=200)
    else:
        # TODO: Load real dataset
        print("[Train] Real dataset loading not yet implemented")
        print("[Train] Use --mock-data flag for testing")
        return

    # Build model
    model_builder = OmnisightDetectionModel(config)
    model_builder.build_model()
    model_builder.compile_model()

    # Print model summary
    model_builder.model.summary()

    # Train
    history = model_builder.train(train_dataset, val_dataset, args.checkpoint_dir)

    # Save final model
    final_model_path = os.path.join(args.checkpoint_dir, 'final_model.h5')
    model_builder.save_model(final_model_path)

    # Save training history
    history_path = os.path.join(args.checkpoint_dir, 'history.json')
    with open(history_path, 'w') as f:
        json.dump(history.history, f, indent=2)

    print("\n" + "=" * 60)
    print("Training Complete!")
    print("=" * 60)
    print(f"Best model: {os.path.join(args.checkpoint_dir, 'best_model.h5')}")
    print(f"Final model: {final_model_path}")
    print(f"History: {history_path}")
    print("\nNext steps:")
    print("1. Convert model to TFLite:")
    print(f"   python convert_to_tflite.py --model {final_model_path}")
    print("2. Validate on test set:")
    print(f"   python validate_model.py --model converted.tflite")
    print("=" * 60)


if __name__ == '__main__':
    main()
