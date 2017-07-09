import sys

def is_global_function(symbol_type):
    return symbol_type == 'T'

def write_assembly(symbols, file_name):
    with open(file_name, 'w') as f:

        f.write('.section .data\n')

        for symbol in symbols:
            f.write('.extern {symbol}\n'.format(symbol=symbol))

        f.write('.global kernel_symbol_table_start\n')
        f.write('kernel_symbol_table_start:\n')

        for symbol in symbols:
            f.write('''
            .quad {symbol}
            .quad .symbol_table_{symbol}
            '''.format(symbol=symbol))

        f.write('\n')
        f.write('.global kernel_symbol_table_end\n')
        f.write('kernel_symbol_table_end:\n')

        for symbol in symbols:
            f.write('''
            .symbol_table_{symbol}:
            .asciz "{symbol}"'''.format(symbol=symbol))


if __name__ == '__main__':
    symbols = []
    for line in sys.stdin.readlines():
        _, symbol_type, name = line.split()
        if is_global_function(symbol_type):
            symbols.append(name)

    symbols = []
    write_assembly(symbols, sys.argv[1])
