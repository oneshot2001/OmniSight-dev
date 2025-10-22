"""
OMNISIGHT IPC Package

Inter-Process Communication between C core and Python Flask server.
"""

from .ipc_client import OmnisightIPCClient, Detection, TrackedObject, Timeline

__all__ = ['OmnisightIPCClient', 'Detection', 'TrackedObject', 'Timeline']
