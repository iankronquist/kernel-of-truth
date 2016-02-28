with open('test.h', 'w') as fh:
    fh.write("#ifndef TESTING123_H\n#define TESTING123_H\nchar a[] = {")
    with open('test.o') as f:
        content = f.read()
        for c in content[0x180:]:
            print hex(ord(c))
            fh.write('{}, '.format(hex(ord(c))))
    fh.write("};\n")
    fh.write("#endif")
