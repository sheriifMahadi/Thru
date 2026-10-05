#!/usr/bin/env python3
"""Hash locally and print the 64 hex characters used as instruction data."""
import argparse
import hashlib

parser = argparse.ArgumentParser(description=__doc__)
source = parser.add_mutually_exclusive_group(required=True)
source.add_argument("--text", help="Hash these exact UTF-8 characters, without a newline")
source.add_argument("--file", help="Hash a local file without uploading its contents")
args = parser.parse_args()
if args.text is not None:
    digest = hashlib.sha256(args.text.encode("utf-8")).hexdigest()
else:
    with open(args.file, "rb") as stream:
        digest = hashlib.file_digest(stream, "sha256").hexdigest()
print(digest)
