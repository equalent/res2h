res2h
========

Load plain binary data from arbitrary files and dump them to a raw hex C/C++ array for compiling in your software. Inspired by [bin2h](http://code.google.com/p/bin2h/) with added functionality. It should at least work in Windows, Ubuntu and Raspbian.

License
========

[BSD-3-Clause](http://opensource.org/licenses/BSD-3-Clause), see LICENSE.md

Building
========

**Use CMake:**
<pre>
cd res2h
cmake .
make
</pre>

G++ 4.7 (for C++11), boost-filesystem and boost-system are needed to compile. For installing G++ 4.7 see [here](http://lektiondestages.blogspot.de/2013/05/installing-and-switching-gccg-versions.html).
Install the boost development packages with:
```
sudo apt-get libboost<VERSION>-dev-filesystem
sudo apt-get libboost<VERSION>-dev-system
```

Usage
========
```
res2h <infile/indir> <outfile/outdir> [options]
```

**Valid options:**
- -s Recurse into subdirectories below indir.
- -c Use .c files and arrays for storing the data definitions, else uses .cpp files and std::vector/std::map.
- -h <headerfile> Puts all declarations in the file "headerfile" using "extern" and includes that header file in the source files.
- -u <sourcefile> Create utility functions and arrays in a .c/.cpp file. Only makes sense in combination with -h.
- -b Compile binary archive outfile containing all infile(s). For reading in your software include res2hinterface.h/.c/.cpp (depending on -c) and consult the docs.
- -a Append infile to outfile. Can be used to append an archive to an executable.
- -v Be verbose.

**Examples:**
- Convert a single file: ```res2h ./lenna.png ./resources/lenna_png.cpp -c```
- Convert all files in a directory, create a common header and utilities: ```res2h ./data ./resources -s -h resources.h -u resources.cpp```
- Convert data to a binary archive: ```res2h ./data ./resources/data.bin -b```
- Append an archive to an executable: ```res2h ./resources/data.bin ./program.exe -a```

Output
========
##### e.g. "res2h a.x b_x.cpp -h bla.h" would create those files: #####
- **a_x.cpp:**

```
//this file was auto-generated from a.x by res2h

#include "bla.h"

const size_t a_x_size = 123;
const unsigned char a_x_data[a_x_size] = {
    0x11,0x22,...
};
```

- **bla.h:**

```
//this file was auto-generated by res2h

extern const size_t a_x_size;
extern const unsigned char a_x_data[];
```
========
##### e.g. "res2h a.x b_x.cpp -h bla.h *-u bla.cpp*" would create a_x.cpp too, and: #####
- **bla.h:**

```
//this file was auto-generated by res2h

extern const size_t a_x_size;
extern const unsigned char a_x_data[];

struct Res2hEntry {
    const std::string relativeFileName;
    const size_t size;
    const unsigned char * data;
};

//this contains all the resources with their names and data
extern const std::vector<const Res2hEntry> res2hVector;

//this maps the relative file name of resource to its data
//usage e.g: Res2hEntry resource = res2hMap["a.x"];
extern const std::map<const std::string, const Res2hEntry> res2hMap;
```

- **bla.cpp:**

```
//this file was auto-generated by res2h

#include "bla.h"

const std::vector<const Res2hEntry> res2hVector = {
 {"a.x", a_x_size, a_x_data}
};

const std::map<const std::string, const Res2hEntry> res2hMap = {
 std::pair<const std::string, const Res2hEntry>("a.x", {"a.x", a_x_size, a_x_data})
};
```
========
##### e.g. "res2h a.x b_x.cpp *-c* -h bla.h -u bla.cpp" would create a_x.cpp too, and: #####
- **bla.h:**

```
//this file was auto-generated by res2h

extern const size_t a_x_size;
extern const unsigned char a_x_data[];

struct Res2hEntry {
    const char * relativeFileName;
    const size_t size;
    const unsigned char * data;
};

//this contains all the resources with their names and data
extern const size_t res2hVector_size;
extern const Res2hEntry res2hVector[];
```

- **bla.cpp:**

```
//this file was auto-generated by res2h

#include "bla.h"

const size_t res2hVector_size = 1;
const const Res2hEntry res2hVector[res2hVector_size] = {
 {"a.x", a_x_size, a_x_data}
};
```

Binary archive format
========
<table>
    <tr>
        <th>Offset (decimal)</th><th>Type</th><th>Description</th>
    </tr>
    <tr>
        <td>Start</td><td>char[8]</td><td>magic number string "res2hbin"</td>
    </tr>
    <tr>
        <td>08</td><td>uint32_t</td><td>file format version number (currently 1)</td>
    </tr>
    <tr>
        <td>12</td><td>uint32_t</td><td>format flags or other crap for file (currently 0)</td>
    </tr>
    <tr>
        <td>16</td><td>uint32_t</td><td>number of directory and file entries following</td>
    </tr>
    <tr>
        <td colspan="3">Then follows the directory:</td>
    </tr>
    <tr>
        <td>20 + 00</td><td>uint32_t</td><td>file entry #0, size of internal name INCLUDING null-terminating character</td>
    </tr>
    <tr>
        <td>20 + 04</td><td>char[]</td><td>file entry #0, internal name (null-terminated)</td>
    </tr>
    <tr>
        <td>20 + 04 + name</td><td>uint32_t</td><td>file entry #0, format flags for entry (currently 0)</td>
    </tr>
    <tr>
        <td>20 + 08 + name</td><td>uint32_t</td><td>file entry #0, size of data</td>
    </tr>
    <tr>
        <td>20 + 12 + name</td><td>uint32_t</td><td>file entry #0, absolute offset of data in file</td>
    </tr>
    <tr>
        <td>20 + 16 + name</td><td>uint32_t</td><td>file entry #0, Adler-32 (RFC1950) checksum of data</td>
    </tr>
    <tr>
        <td colspan="3">Then follow the other directory entries</td>
    </tr>
    <tr>
        <td colspan="3">Directly after the directory the data blocks begin</td>
    </tr>
    <tr>
        <td>End - 04</td><td>uint32_t</td><td>Adler-32 (RFC1950) checksum of whole file up to this point</td>
    </tr>
</table>

For reading binary files. Include the files "res2hinterface.hpp/.cpp" or "res2hinterface.h/.c" in your project. They help you with resource handling.

FAQ
========
- Q: Why the duplicate code?
A: I wanted to have monolightic files for C and C++. The idea was that including one header and one source file in your project should be enough to get the whole thing running.
- Q: The C++ interface is much better...
A: Yes. It got more love. C++ makes stuff much easier to implement. I figured the C interface would be used on low-power systems anyway and thus should be slimmer.

I found a bug or have suggestion
========

The best way to report a bug or suggest something is to post an issue on GitHub. Try to make it simple, but descriptive and add ALL the information needed to REPRODUCE the bug. **"Does not work" is not enough!** If you can not compile, please state your system, compiler version, etc! You can also contact me via email if you want to.