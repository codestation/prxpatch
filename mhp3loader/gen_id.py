#!/usr/bin/env python
import os
import sys
import hashlib
from struct import pack, unpack

def hashfile(filepath):
    sha1 = hashlib.sha1()
    f = open(filepath, 'rb')
    f.seek(32)
    val = unpack('I', f.read(4))[0]
    f.seek(32)
    if val != 148:
        print('You need to use a decrypted mib file, error in ' + filepath)
        f.close()
        sys.exit(1)
    try:
        sha1.update(f.read(220))
    finally:
        f.close()
    return sha1.hexdigest()

if __name__ == '__main__':
    list = []
    for filename in sys.argv[1:]:
        basename = os.path.basename(filename)
        print(basename + ": " + hashfile(filename)[:8])
        val = pack('>I', int(hashfile(filename)[:8], 16))
        hash = unpack('I', val)[0]
        list.append((hash, int(basename[:4])))
    f = open('mib_id.dat', 'wb')
    list.sort()

    for item in list:
        f.write(pack('I', item[0]))
    for item in list:
        f.write(pack('I', item[1]))

    f.close()
    sys.exit(0)
