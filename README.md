# *SECS-II* Serialization

![C++](docs/badges/C++.svg)
[![CMake](docs/badges/Made-with-CMake.svg)](https://cmake.org)
![GitHub Actions](docs/badges/Made-with-GitHub-Actions.svg)
![License](docs/badges/License-MIT.svg)

## Introduction

A *SECS-II/SEMI E5* serialization library written in *C++23*, supporting:

- Deserializing SECS-II data from bytes.
- Serializing SECS-II data to bytes.
- Formatting SECS-II data to *SML* (*SECS Message Language*) strings.

## Unit Tests

### Prerequisites

- Install *GoogleTest*.
- Install *CMake*.

### Building

Go to the project folder and run:

```bash
mkdir -p build
cd build
cmake -DSECS2_BUILD_TESTS=ON ..
cmake --build .
```

### Running

Go to the `build` folder and run:

```bash
ctest -V
```

## Examples

### Serialization

```c++
List list;
list.push_back(U1 {1, 2});
list.push_back(list);
list.push_back(ASCII {"msg"});
list.push_back(U2 {3, 4});
list.push_back(U1 {});

const auto bytes {Message {list}.ToBytes()};
```

The value of `bytes` is:

```console
0x01 (List, 1 size byte)
0x05 (5 elements)
    0xA5 (U1, 1 size byte)
    0x02 (2 bytes)
        0x01 (1)
        0x02 (2)
    0x01 (List, 1 size byte)
    0x01 (1 element)
        0xA5 (U1, 1 size byte)
        0x02 (2 bytes)
            0x01 (1)
            0x02 (2)
    0x41 (ASCII, 1 size byte)
    0x03 (3 bytes)
        0x6D ('m')
        0x73 ('s')
        0x67 ('g')
    0xA9 (U2, 1 size byte)
    0x02 (4 bytes)
        0x00 0x03 (3)
        0x00 0x04 (4)
    0xA5 (U1, 1 size byte)
    0x00 (0 bytes)
```

### SML Formatting

```c++
List list;
list.push_back(I1 {});
list.push_back(Binary {static_cast<std::byte>(1), static_cast<std::byte>(2)});
list.push_back(list);
list.push_back(ASCII {"hello"});

const auto sml {Message {list}.ToSml()};
```

The value of `sml` is:

```console
<L [4]
    <I1 [0]>
    <B [2] 0x01 0x02>
    <L [2]
        <I1 [0]>
        <B [2] 0x01 0x02>
    >
    <A [5] "hello">
>
```

## License

Distributed under the *MIT License*. See `LICENSE` for more information.