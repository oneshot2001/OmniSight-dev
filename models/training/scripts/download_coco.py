#!/usr/bin/env python3
"""
COCO Dataset Downloader for OMNISIGHT
Downloads and prepares COCO dataset for training
"""

import os
import sys
import argparse
import json
import urllib.request
import zipfile
from pathlib import Path
from tqdm import tqdm
import shutil

# COCO dataset URLs
COCO_URLS = {
    'train2017_images': 'http://images.cocodataset.org/zips/train2017.zip',
    'val2017_images': 'http://images.cocodataset.org/zips/val2017.zip',
    'train2017_annotations': 'http://images.cocodataset.org/annotations/annotations_trainval2017.zip',
}


class COCODownloader:
    """COCO dataset downloader and preprocessor"""

    def __init__(self, data_dir='./data/coco'):
        self.data_dir = Path(data_dir)
        self.data_dir.mkdir(parents=True, exist_ok=True)

    def download_file(self, url, dest_path):
        """Download file with progress bar"""
        print(f"[Download] {url}")

        # Check if already exists
        if dest_path.exists():
            print(f"[Skip] File already exists: {dest_path}")
            return

        # Download with progress
        with tqdm(unit='B', unit_scale=True, unit_divisor=1024) as pbar:
            def report_hook(block_num, block_size, total_size):
                if total_size > 0:
                    pbar.total = total_size
                    pbar.update(block_size)

            urllib.request.urlretrieve(url, dest_path, reporthook=report_hook)

        print(f"[Downloaded] {dest_path}")

    def extract_zip(self, zip_path, extract_dir):
        """Extract zip file"""
        print(f"[Extract] {zip_path}")

        with zipfile.ZipFile(zip_path, 'r') as zip_ref:
            zip_ref.extractall(extract_dir)

        print(f"[Extracted] {extract_dir}")

    def download_coco(self, split='train', download_images=True, download_annotations=True):
        """Download COCO dataset"""
        print(f"\n{'='*60}")
        print(f"COCO Dataset Download - {split}")
        print(f"{'='*60}\n")

        # Download images
        if download_images:
            if split == 'train':
                url = COCO_URLS['train2017_images']
                zip_name = 'train2017.zip'
            elif split == 'val':
                url = COCO_URLS['val2017_images']
                zip_name = 'val2017.zip'
            else:
                raise ValueError(f"Invalid split: {split}")

            zip_path = self.data_dir / zip_name
            self.download_file(url, zip_path)

            # Extract
            self.extract_zip(zip_path, self.data_dir)

            # Cleanup zip
            print(f"[Cleanup] Removing {zip_path}")
            zip_path.unlink()

        # Download annotations
        if download_annotations:
            url = COCO_URLS['train2017_annotations']
            zip_name = 'annotations_trainval2017.zip'
            zip_path = self.data_dir / zip_name

            self.download_file(url, zip_path)
            self.extract_zip(zip_path, self.data_dir)

            # Cleanup
            print(f"[Cleanup] Removing {zip_path}")
            zip_path.unlink()

        print(f"\n[Complete] COCO {split} dataset downloaded to {self.data_dir}")

    def create_subset(self, split='train', subset_size=5000, categories=None):
        """Create a smaller subset for faster training/testing"""
        print(f"\n{'='*60}")
        print(f"Creating COCO Subset - {split} ({subset_size} images)")
        print(f"{'='*60}\n")

        # Default OMNISIGHT categories
        if categories is None:
            categories = ['person', 'car', 'truck', 'bus', 'bicycle', 'motorcycle']

        # Load annotations
        ann_file = self.data_dir / 'annotations' / f'instances_{split}2017.json'
        if not ann_file.exists():
            raise FileNotFoundError(f"Annotations not found: {ann_file}")

        with open(ann_file, 'r') as f:
            coco_data = json.load(f)

        # Filter categories
        cat_ids = [cat['id'] for cat in coco_data['categories'] if cat['name'] in categories]
        print(f"[Filter] Selected categories: {categories}")
        print(f"[Filter] Category IDs: {cat_ids}")

        # Filter annotations
        filtered_annotations = [
            ann for ann in coco_data['annotations']
            if ann['category_id'] in cat_ids
        ]

        # Get unique image IDs
        image_ids = list(set([ann['image_id'] for ann in filtered_annotations]))
        image_ids = image_ids[:subset_size]

        print(f"[Subset] Selected {len(image_ids)} images")

        # Filter images
        filtered_images = [
            img for img in coco_data['images']
            if img['id'] in image_ids
        ]

        # Filter annotations again
        filtered_annotations = [
            ann for ann in filtered_annotations
            if ann['image_id'] in image_ids
        ]

        # Create new annotation file
        subset_data = {
            'images': filtered_images,
            'annotations': filtered_annotations,
            'categories': [cat for cat in coco_data['categories'] if cat['id'] in cat_ids],
            'info': coco_data.get('info', {}),
            'licenses': coco_data.get('licenses', [])
        }

        # Save subset annotations
        subset_dir = self.data_dir / f'subset_{split}2017'
        subset_dir.mkdir(exist_ok=True)

        subset_ann_file = subset_dir / f'annotations.json'
        with open(subset_ann_file, 'w') as f:
            json.dump(subset_data, f, indent=2)

        print(f"[Saved] Subset annotations: {subset_ann_file}")

        # Copy subset images
        images_dir = subset_dir / 'images'
        images_dir.mkdir(exist_ok=True)

        src_images_dir = self.data_dir / f'{split}2017'

        print(f"[Copy] Copying {len(filtered_images)} images...")
        for img in tqdm(filtered_images):
            src = src_images_dir / img['file_name']
            dst = images_dir / img['file_name']
            if src.exists() and not dst.exists():
                shutil.copy2(src, dst)

        print(f"\n[Complete] Subset created: {subset_dir}")
        print(f"  - Images: {len(filtered_images)}")
        print(f"  - Annotations: {len(filtered_annotations)}")
        print(f"  - Categories: {len(subset_data['categories'])}")

        return subset_ann_file, images_dir

    def get_dataset_stats(self):
        """Print dataset statistics"""
        print(f"\n{'='*60}")
        print("COCO Dataset Statistics")
        print(f"{'='*60}\n")

        for split in ['train', 'val']:
            ann_file = self.data_dir / 'annotations' / f'instances_{split}2017.json'
            if not ann_file.exists():
                print(f"[Skip] {split} annotations not found")
                continue

            with open(ann_file, 'r') as f:
                data = json.load(f)

            print(f"{split.upper()}2017:")
            print(f"  - Images: {len(data['images'])}")
            print(f"  - Annotations: {len(data['annotations'])}")
            print(f"  - Categories: {len(data['categories'])}")

            # Category distribution
            cat_counts = {}
            for ann in data['annotations']:
                cat_id = ann['category_id']
                cat_counts[cat_id] = cat_counts.get(cat_id, 0) + 1

            print(f"\n  Top 10 Categories:")
            cat_names = {cat['id']: cat['name'] for cat in data['categories']}
            sorted_cats = sorted(cat_counts.items(), key=lambda x: x[1], reverse=True)[:10]
            for cat_id, count in sorted_cats:
                print(f"    - {cat_names.get(cat_id, 'unknown')}: {count}")
            print()


def main():
    parser = argparse.ArgumentParser(description='Download COCO dataset for OMNISIGHT')
    parser.add_argument('--data-dir', type=str, default='./data/coco',
                        help='Directory to save COCO dataset')
    parser.add_argument('--split', type=str, default='train',
                        choices=['train', 'val', 'both'],
                        help='Dataset split to download')
    parser.add_argument('--images', action='store_true',
                        help='Download images')
    parser.add_argument('--annotations', action='store_true',
                        help='Download annotations')
    parser.add_argument('--subset', type=int, default=None,
                        help='Create subset with N images')
    parser.add_argument('--categories', type=str, nargs='+',
                        default=['person', 'car', 'truck', 'bus', 'bicycle', 'motorcycle'],
                        help='Categories to include in subset')
    parser.add_argument('--stats', action='store_true',
                        help='Print dataset statistics')

    args = parser.parse_args()

    # Create downloader
    downloader = COCODownloader(data_dir=args.data_dir)

    # Download dataset
    if args.images or args.annotations:
        if args.split == 'both':
            for split in ['train', 'val']:
                downloader.download_coco(
                    split=split,
                    download_images=args.images,
                    download_annotations=args.annotations
                )
        else:
            downloader.download_coco(
                split=args.split,
                download_images=args.images,
                download_annotations=args.annotations
            )

    # Create subset
    if args.subset:
        if args.split == 'both':
            for split in ['train', 'val']:
                subset_size = args.subset if split == 'train' else max(args.subset // 5, 500)
                downloader.create_subset(
                    split=split,
                    subset_size=subset_size,
                    categories=args.categories
                )
        else:
            downloader.create_subset(
                split=args.split,
                subset_size=args.subset,
                categories=args.categories
            )

    # Print statistics
    if args.stats:
        downloader.get_dataset_stats()


if __name__ == '__main__':
    main()
