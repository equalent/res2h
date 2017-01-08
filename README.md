res2h
========

**tl;dr:** Load binary data from arbitrary files and convert it to a raw hex C/C++ array for compiling into your software, or bundle the data from all the files in a binary archive. It is inspired by [bin2h](http://code.google.com/p/bin2h/) with added functionality.

**res2h** can convert binary data from files to a raw hex arrays in .c/.cpp source files which you can then include in your project and compile them into the executable. It can also create a common header that lets you access all the converted arrays with one include. If you don't want your data to be loaded into memory res2h also provides the possiblility to create one binary archive containing all the files which you can then access via the "Res2h" class provided in seperate headers. You can also embed this archive in your executable, so you only have one file, and access it like you would with any other archive on disk.
It should compile and work at least in Windows, Ubuntu and Raspbian.

**res2hdump** is a tool that lets you dump information and/or files from a binary res2h archive or an archive embedded in another file, e.g. executable. It also serves as an example on how to use the "Res2h" class contained in the "res2hinterface" files.

**run_test** is a script that lets you test res2h and res2hdump after compiling. It uses res2h with some options to create C++ files and a binary archive from example files in /test. It then unpacks this archive to /results again.

**unittest** does the same thing in an executable. It is a good example on how to use the re2h functions.

License
========

[BSD-2-Clause](http://opensource.org/licenses/BSD-2-Clause), see LICENSE.md

Building
========

**Use CMake:**
<pre>
cd res2h
cmake .
make
</pre>

Visual Studio 2015, G++ 5.4 or Clang 3.9 (C++11 support) are needed. "res2h" and "res2hdump" use std::filesystem, but not the "res2hinterface" you use in your application. For installing a new G++ version see [here](http://lektiondestages.blogspot.de/2013/05/installing-and-switching-gccg-versions.html).

Usage - res2h
========

```
res2h <infile/indir> <outfile/outdir> [options]
```

**Valid options:**
- -s Recurse into subdirectories below indir.
- -c Use .c files and arrays for storing the data definitions, else uses .cpp files and std::vector/std::map.
- -h <headerfile> Puts all declarations in the file "headerfile" using "extern" and includes that header file in the source files.
- -u <sourcefile> Create utility functions and arrays in a .c/.cpp file. Only makes sense in combination with -h.
- -1 Combine all converted files into one big .c/.cpp file (use together with -u).
- -b Compile binary archive outfile containing all infile(s). For reading in your software include res2hinterface.h/.c/.cpp (depending on -c) and consult the docs.
- -a Append infile to outfile. Can be used to append an archive to an executable.
- -v Be verbose.

**Examples:**
- Convert a single file: ```res2h ./lenna.png ./resources/lenna_png.cpp```
- Convert all files in a directory, create a common header and utilities: ```res2h ./data ./resources -s -h resources.h -u resources.cpp```
- Convert all files in a directory, create a common header and utilities, combine all data in resources.cpp: ```res2h ./data ./resources -s -1 -h resources.h -u resources.cpp```
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
extern const size_t res2hNrOfFiles;
extern const Res2hEntry res2hFiles[];
```

- **bla.cpp:**

```
//this file was auto-generated by res2h

#include "bla.h"

const size_t res2hNrOfFiles = 4;
const Res2hEntry res2hFiles[res2hNrOfFiles] = {
 {":/a.x", a_x_size, a_x_data}
};
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
extern const size_t res2hNrOfFiles;
extern const Res2hEntry res2hFiles[];

//this maps the relative file name of resource to its data
//usage e.g: Res2hEntry resource = res2hMap["a.x"];
typedef const std::map<const std::string, const Res2hEntry> res2hMapType;
extern res2hMapType res2hMap;
```

- **bla.cpp:**

```
//this file was auto-generated by res2h

#include "bla.h"

const size_t res2hNrOfFiles = 4;
const Res2hEntry res2hFiles[res2hNrOfFiles] = {
 {":/a.x", a_x_size, a_x_data}
};

res2hMapType::value_type mapTemp[] = {
    std::make_pair(":/a.x", res2hFiles[0]),
};

res2hMapType res2hMap(mapTemp, mapTemp + sizeof mapTemp / sizeof mapTemp[0]);
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
        <td>16</td><td>uint32_t</td><td>size of whole archive in bytes</td>
    </tr>
    <tr>
        <td>20</td><td>uint32_t</td><td>number of directory and file entries following</td>
    </tr>
    <tr>
        <td colspan="3">Then follows the directory:</td>
    </tr>
    <tr>
        <td>24 + 00</td><td>uint32_t</td><td>file entry #0, size of internal name INCLUDING null-terminating character</td>
    </tr>
    <tr>
        <td>24 + 04</td><td>char[]</td><td>file entry #0, internal name (null-terminated)</td>
    </tr>
    <tr>
        <td>24 + 04 + name</td><td>uint32_t</td><td>file entry #0, format flags for entry (currently 0)</td>
    </tr>
    <tr>
        <td>24 + 08 + name</td><td>uint32_t</td><td>file entry #0, size of data</td>
    </tr>
    <tr>
        <td>24 + 12 + name</td><td>uint32_t</td><td>file entry #0, absolute offset of data in file</td>
    </tr>
    <tr>
        <td>24 + 16 + name</td><td>uint32_t</td><td>file entry #0, Adler-32 (RFC1950) checksum of data</td>
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

You can read an archive from a file on disk, but also embed an archive in another file, e.g. your executable. For that use the "-a" option to append the archive to the executable (Please note that you can only have one embedded archive). For reading archive files or embedded archives include the files "res2hinterface.hpp/.cpp" and "res2hutils.hpp/.cpp" in your project. They provide all functions needed for reading resources from archives.
You can find an example on how to use the functions in "res2hdump.cpp" resp. the "res2hdump" project.

Usage - res2hdump
========

```
res2hdump <archive> [<outdir>] [options]
```

**Valid options:**
- -f Recreate path structure, creating directories as needed.
- -i Display information about the archive and files, but don't extract anything.
- -v Be verbose.

**Examples:**
- Display information about the archive: ```res2hdump ./resources/data.bin -i```
- Extract all files from an archive with subdirectories: ```res2hdump ./resources/data.bin ./resources -f```
- Extract files from embedded archive: ```res2hdump ./resources/program.exe ./resources```

I found a bug or have suggestion
========

The best way to report a bug or suggest something is to post an issue on GitHub. Try to make it simple, but descriptive and add ALL the information needed to REPRODUCE the bug. **"Does not work" is not enough!** If you can not compile, please state your system, compiler version, etc! You can also contact me via email if you want to.
