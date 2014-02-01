tstring
=======
The unpretentious True Way to implement strings.

The tstring is the string used by the Truth Kernel. The goal is to improve 
upon C-style strings by adding built in length data. Along the way a variety of
mistakes will surely be made.
tstrings are mutable, fixed length strings. Like C-style strings they are at 
their core contiguous arrays of `char`s. They do not end in a null byte.
The first several characters contain data about the string length. 
Do not manipulate tstrings directly, use the methods in the tstring library.

The first byte represents the length of the string, except for the highest bit.
Should the string be longer than that number the value of the next byte, except
for the highest bit. To get the length of the string, add the value of the 
first and second chars (except their highest bits). If this string is larger
than the value which can be held in two chars, another char will be added,
and so on. 
The maximum size of a tstring is the maximum size of an unsigned long long. 

The following diagram assumes MSB bit ordering.  
00000001 01100001  
[length]   'a'  


00001101 01001000 01100101 01101100 01101100 01101111 00101100  
[length]   'H'       'e'      'l'     'l'      'o'      ','      
00100000 01010111 01101111 01110010 01101100 01100100 00010000  
<space>    'W'       'o'     'r'       'l'     'd'      '!'  

11111111 00000001 01000001 01000010 010000101  ...
[length] [length]   'A'      'B'       'C'     ...

Functions:

