#!/usr/bin/env python

import sys
import binascii
import hashlib
from struct import pack

def hashfile(filepath):
    sha1 = hashlib.sha1()
    f = open(filepath, 'rb')
    f.seek(32)
    try:
        sha1.update(f.read(220))
    finally:
        f.close()
    return sha1.hexdigest()

if __name__ == '__main__':
    hashes = []
    files = []
    for filename in sys.argv[1:]:
        if filename.endswith('.mib'):
            hashes.append(hashfile(filename)[:8])
            files.append(int(filename[:-4]))
    f = open('mib_id.dat', 'wb')
    for item in hashes:
        f.write(binascii.a2b_hex(item))
    for item in files:
        f.write(pack('i', item))
    f.close()
    sys.exit(0)
