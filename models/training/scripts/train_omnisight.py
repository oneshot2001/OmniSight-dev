#!/usr/bin/env python3
"""
OMNISIGHT End-to-End Training Script
Trains object detection model optimized for ARTPEC-8 DLPU
"""

import os
import sys
import argparse
import json
import numpy as np
import tensorflow as tf
from tensorflow import keras
from pathlib import Path
from datetime import datetime
import yaml

# Import COCO data loader
from coco_dataset import COCODataLoader, create_representative_dataset


class OmnisightDetectionModel:
    """Simplified object detection model for ARTPEC-8"""

    def __init__(self, num_classes=6, input_size=(416, 416, 3)):
        self.num_classes = num_classes
        self.input_size = input_size
        self.model = None

    def build_model(self):
        """Build lightweight detection model"""
        print("[Model] Building ARTPEC-8 optimized architecture...")

        inputs = keras.Input(shape=self.input_size, name='input_image')

        # Efficient backbone (MobileNet-like)
        x = self._build_backbone(inputs)

        # Detection head
        x = keras.layers.GlobalAveragePooling2D()(x)
        x = keras.layers.Dense(256, activation='relu')(x)
        x = keras.layers.Dropout(0.3)(x)

        # Output: [max_detections, 6] -> flatten to 600
        # Each detection: [x1, y1, x2, y2, class, score]
        outputs = keras.layers.Dense(600, activation='sigmoid', name='detections')(x)
        outputs = keras.layers.Reshape((100, 6))(outputs)

        self.model = keras.Model(inputs=inputs, outputs=outputs, name='omnisight_detection')

        print(f"[Model] Built with {self.model.count_params():,} parameters")
        return self.model

    def _build_backbone(self, inputs):
        """Build efficient backbone"""
        # Initial convolution
        x = keras.layers.Conv2D(32, 3, strides=2, padding='same', use_bias=False)(inputs)
        x = keras.layers.BatchNormalization()(x)
        x = keras.layers.ReLU()(x)

        # Efficient blocks
        filters = [64, 128, 256, 512]
        for f in filters:
            x = self._efficient_block(x, f, stride=2)

        return x

    def _efficient_block(self, x, filters, stride=1):
        """Efficient conv block (optimized for ARTPEC-8)"""
        # Use regular convolutions (better for DLPU than depthwise)
        shortcut = x

        # Main path
        x = keras.layers.Conv2D(filters, 3, strides=stride, padding='same', use_bias=False)(x)
        x = keras.layers.BatchNormalization()(x)
        x = keras.layers.ReLU()(x)

        x = keras.layers.Conv2D(filters, 3, padding='same', use_bias=False)(x)
        x = keras.layers.BatchNormalization()(x)

        # Residual connection
        if stride != 1 or shortcut.shape[-1] != filters:
            shortcut = keras.layers.Conv2D(filters, 1, strides=stride, padding='same', use_bias=False)(shortcut)
            shortcut = keras.layers.BatchNormalization()(shortcut)

        x = keras.layers.Add()([x, shortcut])
        x = keras.layers.ReLU()(x)

        return x

    def compile_model(self, learning_rate=0.001):
        """Compile model"""
        optimizer = keras.optimizers.Adam(learning_rate=learning_rate)

        # Simple MSE loss for detection outputs
        self.model.compile(
            optimizer=optimizer,
            loss='mse',
            metrics=['mae']
        )

        print(f"[Model] Compiled with lr={learning_rate}")


class OmnisightTrainer:
    """OMNISIGHT model trainer"""

    def __init__(self, config_path=None):
        self.config = self._load_config(config_path) if config_path else self._default_config()
        self.model_builder = None
        self.train_dataset = None
        self.val_dataset = None
        self.history = None

    def _default_config(self):
        """Default training configuration"""
        return {
            'model': {
                'num_classes': 6,
                'input_size': [416, 416, 3]
            },
            'training': {
                'batch_size': 16,
                'epochs': 50,
                'learning_rate': 0.001,
                'lr_decay_factor': 0.5,
                'lr_decay_epochs': 10,
            },
            'data': {
                'train_annotations': 'data/coco/subset_train2017/annotations.json',
                'train_images': 'data/coco/subset_train2017/images',
                'val_annotations': 'data/coco/subset_val2017/annotations.json',
                'val_images': 'data/coco/subset_val2017/images',
            },
            'output': {
                'checkpoint_dir': 'models/training/checkpoints',
                'model_dir': 'models/detection',
            }
        }

    def _load_config(self, config_path):
        """Load configuration from YAML"""
        with open(config_path, 'r') as f:
            return yaml.safe_load(f)

    def prepare_datasets(self):
        """Prepare training and validation datasets"""
        print("\n" + "="*60)
        print("Preparing Datasets")
        print("="*60 + "\n")

        # Training dataset
        train_loader = COCODataLoader(
            annotations_file=self.config['data']['train_annotations'],
            images_dir=self.config['data']['train_images'],
            target_size=tuple(self.config['model']['input_size'][:2]),
            num_classes=self.config['model']['num_classes'],
            batch_size=self.config['training']['batch_size'],
            shuffle=True
        )
        self.train_dataset = train_loader.create_dataset()

        # Validation dataset
        val_loader = COCODataLoader(
            annotations_file=self.config['data']['val_annotations'],
            images_dir=self.config['data']['val_images'],
            target_size=tuple(self.config['model']['input_size'][:2]),
            num_classes=self.config['model']['num_classes'],
            batch_size=self.config['training']['batch_size'],
            shuffle=False
        )
        self.val_dataset = val_loader.create_dataset()

        print(f"[Data] Training batches: {train_loader.get_num_batches()}")
        print(f"[Data] Validation batches: {val_loader.get_num_batches()}")

    def build_model(self):
        """Build and compile model"""
        print("\n" + "="*60)
        print("Building Model")
        print("="*60 + "\n")

        self.model_builder = OmnisightDetectionModel(
            num_classes=self.config['model']['num_classes'],
            input_size=tuple(self.config['model']['input_size'])
        )

        self.model_builder.build_model()
        self.model_builder.compile_model(
            learning_rate=self.config['training']['learning_rate']
        )

    def train(self):
        """Train the model"""
        print("\n" + "="*60)
        print("Training Model")
        print("="*60 + "\n")

        # Create callbacks
        checkpoint_dir = Path(self.config['output']['checkpoint_dir'])
        checkpoint_dir.mkdir(parents=True, exist_ok=True)

        callbacks = [
            keras.callbacks.ModelCheckpoint(
                filepath=str(checkpoint_dir / 'model_epoch_{epoch:02d}.keras'),
                save_best_only=True,
                monitor='val_loss',
                mode='min'
            ),
            keras.callbacks.ReduceLROnPlateau(
                monitor='val_loss',
                factor=self.config['training']['lr_decay_factor'],
                patience=self.config['training']['lr_decay_epochs'],
                min_lr=1e-7,
                verbose=1
            ),
            keras.callbacks.EarlyStopping(
                monitor='val_loss',
                patience=15,
                restore_best_weights=True
            ),
            keras.callbacks.TensorBoard(
                log_dir=str(checkpoint_dir / 'logs' / datetime.now().strftime('%Y%m%d-%H%M%S'))
            )
        ]

        # Train
        self.history = self.model_builder.model.fit(
            self.train_dataset,
            validation_data=self.val_dataset,
            epochs=self.config['training']['epochs'],
            callbacks=callbacks,
            verbose=1
        )

        print("\n[Training] Complete!")

    def save_model(self):
        """Save trained model"""
        print("\n" + "="*60)
        print("Saving Model")
        print("="*60 + "\n")

        model_dir = Path(self.config['output']['model_dir'])
        model_dir.mkdir(parents=True, exist_ok=True)

        # Save as Keras model
        model_path = model_dir / 'omnisight_detection.keras'
        self.model_builder.model.save(str(model_path))
        print(f"[Saved] Keras model: {model_path}")

        # Save as SavedModel (for TFLite conversion)
        saved_model_path = model_dir / 'saved_model'
        tf.saved_model.save(self.model_builder.model, str(saved_model_path))
        print(f"[Saved] SavedModel: {saved_model_path}")

        # Save training history
        history_path = model_dir / 'training_history.json'
        with open(history_path, 'w') as f:
            json.dump({
                'loss': [float(x) for x in self.history.history['loss']],
                'val_loss': [float(x) for x in self.history.history['val_loss']],
                'mae': [float(x) for x in self.history.history['mae']],
                'val_mae': [float(x) for x in self.history.history['val_mae']],
            }, f, indent=2)
        print(f"[Saved] Training history: {history_path}")

        # Save model info
        info = {
            'model_name': 'OMNISIGHT Detection',
            'version': '1.0.0',
            'input_size': self.config['model']['input_size'],
            'num_classes': self.config['model']['num_classes'],
            'num_parameters': int(self.model_builder.model.count_params()),
            'training_date': datetime.now().isoformat(),
            'final_loss': float(self.history.history['loss'][-1]),
            'final_val_loss': float(self.history.history['val_loss'][-1]),
        }

        info_path = model_dir / 'model_info.json'
        with open(info_path, 'w') as f:
            json.dump(info, f, indent=2)
        print(f"[Saved] Model info: {info_path}")


def main():
    parser = argparse.ArgumentParser(description='Train OMNISIGHT detection model')
    parser.add_argument('--config', type=str, default=None,
                        help='Path to config YAML file')
    parser.add_argument('--epochs', type=int, default=None,
                        help='Number of epochs (overrides config)')
    parser.add_argument('--batch-size', type=int, default=None,
                        help='Batch size (overrides config)')
    parser.add_argument('--lr', type=float, default=None,
                        help='Learning rate (overrides config)')

    args = parser.parse_args()

    # Create trainer
    trainer = OmnisightTrainer(config_path=args.config)

    # Override config with CLI args
    if args.epochs:
        trainer.config['training']['epochs'] = args.epochs
    if args.batch_size:
        trainer.config['training']['batch_size'] = args.batch_size
    if args.lr:
        trainer.config['training']['learning_rate'] = args.lr

    # Print configuration
    print("\n" + "="*60)
    print("OMNISIGHT Model Training")
    print("="*60)
    print(f"\nConfiguration:")
    print(json.dumps(trainer.config, indent=2))
    print()

    # Training pipeline
    try:
        trainer.prepare_datasets()
        trainer.build_model()
        trainer.train()
        trainer.save_model()

        print("\n" + "="*60)
        print("✓ Training Complete!")
        print("="*60 + "\n")

    except KeyboardInterrupt:
        print("\n[Training] Interrupted by user")
        if trainer.model_builder and trainer.model_builder.model:
            print("[Training] Saving current model...")
            trainer.save_model()

    except Exception as e:
        print(f"\n[ERROR] Training failed: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)


if __name__ == '__main__':
    main()
