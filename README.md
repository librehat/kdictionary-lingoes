kdictionary-lingoes
===================

A Lingoes Dictionary File LD2/LDX Reader. Written in C++(Qt).

-----------------------

This program is a standalone Lingoes LD2/LDX reader/extracter.

## Usage

Execute `./kdictionary-lingoes -h` to print this usage information.

```
Usage: ./kdictionary-lingoes [options]
Lingoes dictionary file (LD2/LDX) reader/extracter.

Options:
  -h, --help      Displays this help.
  -v, --version   Displays version information.
  -i <input>      Input Lingoes dictionary file (default: input.ld2).
  -o <output>     Output extracted text file (default: output.txt).
  --disable-trim  Disable HTML tag trimming.
```

## Dependecencies

- CMake > 2.8.11
- Qt5Core

#### LICENSED UNDER GPLv3 ######

### THANKS

[Xiaoyun Zhu](https://code.google.com/u/117780958602636136739/): Author of [original Java code](https://code.google.com/p/dict4cn/source/browse/trunk/importer/src/LingoesLd2Reader.java). 

