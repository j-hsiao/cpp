'''
... my converting sbox into bitwise operations gives correct results
HOWEVER... according to the paper, the number of instructions is like 50 or something
but I'm getting over 1000, if I leave out optimizations, it'd likely be MUCH larger
what optimizations am I missing? or is their sbox just crap?
(if I use the identity sbox, the # of instructions is minimal so... yea...)
if I use shitty sboxes like rotate the identity by some offset, the number
i get 52 instructions, but if I generate a sbox by using an rng
then i get a total number of instructions in the 1000s

Faster and Timing-Attack Resistant AES-GCM
https://cryptojedi.org/papers/aesbs-20090616.pdf

they never even give any details about the sbox they use
is it so fast because the sbox is very simple? cannot determine
because there's no evidence of what the sbox actually is

maybe later if I feel like coming back to investigate this
take a look here
https://eprint.iacr.org/2011/332.pdf

dunno if they start from sbox and create the circuit...

'''
from __future__ import print_function
import sys
from matplotlib import pyplot as plt
import numpy as np
import traceback
import time

l = [
 0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
 0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
 0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
 0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
 0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
 0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
 0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
 0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
 0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
 0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
 0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
 0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
 0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
 0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
 0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
 0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16
]


def sboxgen():
    original = np.arange(256)
    for i, ind in enumerate(np.random.randint(0,256,256, np.uint8)):
        original[i], original[ind] = original[ind], original[i]
    return original

def sboxgen2():
    smat = np.arange(256).astype(np.uint8).reshape(16,16)
    for c, rint in enumerate(np.random.randint(0, 256, 8, np.uint8)):
        shift = rint & 0xF
        row = c * 2
        smat[row] = [smat[row, (i + shift) % 16] for i in range(16)]
        shift = (rint & 0xF0) >> 4
        row += 1
        smat[row] = [smat[row, (i + shift) % 16] for i in range(16)]

    for c, rint in enumerate(np.random.randint(0, 256, 8, np.uint8)):
        shift = rint & 0xF
        col = c * 2
        smat[:, col] = [smat[(i + shift) % 16, col] for i in range(16)]
        shift = (rint & 0xF0) >> 4
        col += 1
        smat[row] = [smat[(i + shift) % 16, col] for i in range(16)]

    return smat.ravel()


def add_not(arg, varc, cache):
    expr = '~{}'.format(arg)
    try:
        return cache[expr], varc
    except:
        symbol = 'x{}'.format(varc)
        print('    {} = {}'.format(symbol, expr))
        cache[expr] = symbol
        return symbol, varc + 1

def add_binary(arg1, op, arg2, varc, cache):
    expr1 = '{} {} {}'.format(arg1, op, arg2)
    expr2 = '{} {} {}'.format(arg2, op, arg1)
    try:
        return cache[expr1], varc
    except:
        try:
            return cache[expr2], varc
        except:
            symbol = 'x{}'.format(varc)
            print('    {} = {}'.format(symbol, expr1))
            cache[expr1] = symbol
            cache[expr2] = symbol
            return symbol, varc + 1
    
            
# assume mux is v1 if switch else v2
# ugh fak... why is the list backwards??
def sboxcodegen2(sbox, f):
    sys.stdout = f
    bitouts = np.zeros((8,256), bool)
    # b0 = least significant, b7 = most significant bits
    for j in range(8):
        mask = 1 << j
        for i in range(256):
            bitouts[j][i] = bool(sbox[i] & mask)


    cache = {}

    inputs = 'b0 b1 b2 b3 b4 b5 b6 b7'.split()

    varcount = 0
    print('def sbox(bitslices):')
    print('    # bitslices: should be least significant to most significant, 8 bitslices')
    print('    {} = bitslices'.format(','.join(inputs)))

    for outind, bitout in enumerate(bitouts):
        for stage in range(8):
            switch = inputs[stage]
            temp = []
            for i in range(0, len(bitout), 2):
                v1 = bitout[i + 1]
                v2 = bitout[i]
                # mux gates:
                # ((v1 ^ v2) & s) ^ v2
                # (v1 & s) | (~s & v2)
                if (v1 == v2):
                    temp.append(v1)
                elif (v1, v2, switch) in cache:
                    # order is important
                    temp.append(cache[(v1,v2,switch)])
                else:
                    if isinstance(v1, str):
                        if isinstance(v2, str):
                            # 2 different symbols
                            # ((v1 ^ v2) & s) ^ v2
                            symbol, varcount = add_binary(v1, '^', v2, varcount, cache)
                            symbol, varcount = add_binary(symbol, '&', switch, varcount, cache)
                            symbol, varcount = add_binary(symbol, '^', v2, varcount, cache)
                            temp.append(symbol)
                        else:
                            # v1 is symbol, v2 is const
                            if v2:
                                # ~s | (v1 & s)
                                # ~s | v1
                                symbol, varcount = add_not(switch, varcount, cache)
                                symbol, varcount = add_binary(symbol, '|', v1, varcount, cache)
                                temp.append(symbol)
                            else:
                                # v1 & s
                                symbol, varcount = add_binary(v1, '&', switch, varcount, cache)
                                temp.append(symbol)
                    else:
                        if isinstance(v2, str):
                            #v1 is const, v2 is symbol
                            if v1:
                                # s | (~s & v2)
                                # s | v2
                                symbol, varcount = add_binary(switch, '|', v2, varcount, cache)
                                temp.append(symbol)
                            else:
                                # ~s & v2
                                symbol, varcount = add_not(switch, varcount, cache)
                                symbol, varcount = add_binary(symbol, '&', v2, varcount, cache)
                                temp.append(symbol)
                        else:
                            # 2 different constants
                            if v1:
                                # is the switch directly, no calcs
                                temp.append(switch)
                            else:
                                # is not switch
                                symbol, varcount = add_not(switch, varcount, cache)
                                temp.append(symbol)
                    cache[(v1, v2, switch)] = temp[-1]
            bitout = temp
        #print('return {} calculated, len(temp): {}'.format(outind, len(temp)), file = sys.stderr)
        print('    ret{} = {}'.format(outind, temp[0]))
    print('    return {}'.format(','.join(['ret{}'.format(_) for _ in range(8)])))
    sys.stdout = sys.__stdout__






def sboxcodegen1(steps, f):
    original = ['b{}'.format(i) for i in range(8)]
    print('def sbox(b):', file = f)
    print(' {} = b'.format(','.join(original)), file = f)

    for step in range(steps):
        ncode = 'x{}'.format(step)
        if np.random.randint(0, 8, 1, np.uint8) < 1:
            ind = np.random.randint(0,8,1, np.uint8)[0]
            print(' {} = ~{}'.format(ncode, original[ind]), file = f)
            original[ind] = ncode
        else:
            ind1 = np.random.randint(0,8,1, np.uint8)[0]
            ind2 = np.random.randint(0,8,1, np.uint8)[0]
            while (ind2 == ind1):
                ind2 = np.random.randint(0,8,1, np.uint8)[0]
            print(' {} = {} ^ {}'.format(ncode, original[ind1], original[ind2]), file = f)
            original[ind1] = ncode
    print(' return ' + ','.join(original), file = f)




if __name__ == '__main__':
    x = np.arange(256).astype(np.uint8)
    out = np.zeros(256, np.uint8)

    # bitslices for each byt evalue
    b = np.zeros((8, 256), bool)
    obits = np.zeros((8, 256), bool)

    #np.set_printoptions(threshold = sys.maxsize)


    for bit in range(8):
        for i in range(len(x)):
            b[bit][i] = 1 if (x[i] & (1 << bit)) else 0

    try:
        with open('sbox.py', 'wb') as f:
            sboxcodegen1(64, f)

        from sbox import sbox
        obits = sbox(b)

        for i in range(256):
            for j in range(8):
                out[i] |= ((obits[j][i] & 1) << j)

        print(len(set(out)))


        plt.scatter(x, out)
        plt.show()

    except:
        traceback.print_exc()
        print('sbox failed')

    try:
        #tempbox = l
        #off = 69
        #tempbox = np.array(list(range(off, 256)) + list(range(off)), np.uint8)
        tempbox = sboxgen()
        #tempbox = sboxgen2()

        with open('sbox2.py', 'wb') as f:
            #try to convert sbox into bit ops
            sboxcodegen2(tempbox, f)
        from sbox2 import sbox
        obits = sbox(b)
        out[...] = 0
        for i in range(256):
            for j in range(8):
                out[i] |= ((obits[j][i] & 1) << j)
        print(out == tempbox)

        reps = 10000
        dat = np.arange(8).astype(np.uint64)
        now = time.time()
        for _ in range(reps):
            sbox(dat)
        delta = time.time() - now
        print('bit:', delta)
        now = time.time()
        for _ in range(reps):
            for i in range(64):
                tempbox[i]
        delta = time.time() - now
        print('lut:', delta)

    except:
        traceback.print_exc()
        print('sbox2 failed')






    plt.scatter(x, l)
    plt.show()
    plt.scatter(x, sboxgen())
    plt.show()
