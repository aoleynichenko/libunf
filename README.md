# libunf
Tools for accessing Fortran binary **unf**ormatted files from projects written in the C or C++ programming languages.

## Introduction
Fortran unformatted binary files are very common in scientific applications. The **libunf** library provides simple tools for reading and writing such files from C code. To access the library's subroutines, simply put the **libunf.c** and **libunf.h** files into your project.

## How to compile and test
    mkdir build
    cd build
    cmake .. && make
    make test

## General notes
Interface of the **libunf** library resembles that of **stdio.h** from the C standard library. Functions and constants start with the **unf_** and **UNF_** prefixes.

Unformatted files are accessed via the pointer to the **unf_file_t** data structure:

    // sequential mode; two other possibilities are direct (UNF_ACCESS_DIRECT)
    // and stream (UNF_ACCESS_STREAM) modes
    
    unf_file_t *file = unf_open("numbers.unf", "r", UNF_ACCESS_SEQUENTIAL);

Read/write operations are implemented using the C variadic functions driven by format strings. For example, one can read the entry consisting of an array size (of type **integer(4)**) and an array itself (of type **real(8), dimension(:)**). Imagine that the Fortran 90 code used to write this record was

    integer(4) :: dim
    real(8), dimension(10) :: x

    write(luout) dim, (x(i),i=1,dim)

Then the C code needed to read such an entry would be as follows:

    int32_t dim = 0;
    double x[10]; // must be preallocated
    unf_read(file, "i4,r8[i4]", &n, x, &n);  // 'i4' for 'integer(4)', 'r8' for 'real(8)

Note that the fixed width types must be used at the C side (and their widths are chosen to match those used in the corresponding Fortran read/write operation). It is highly recommended to check possible errors. The **unf_read()** function returns number of arguments read and sets the error flag of the **unf_file_t** object if something went wrong. More information on the error occured can be extracted from the **errno** global variable:

    int n_read = unf_read(file, "i4,r8[i4]", &n, x, &n);
    if (n_read != 2 || unf_error(file)) {
        perror("in unf_read()");
    }

One exit **unf_close()** should be called to close the file and deallocate related objects:

    unf_close(file);

Detailed documentation for all library functions is provided below.

## Detailed documentation

- [unf_open](#unf_open)
- [unf_close](#unf_close)
- [unf_write](#unf_write)
- [unf_write_rec](#unf_write_rec)
- [unf_read](#unf_read)
- [unf_read_rec](#unf_read_rec)
- [unf_next_rec_size](#unf_next_rec_size)
- [unf_seek](#unf_seek)
- [unf_rewind](#unf_rewind)
- [unf_backspace](#unf_backspace)
- [unf_skip](#unf_skip)
- [unf_eof](#unf_eof)
- [unf_error](#unf_error)

### unf_open
    unf_file_t *unf_open(const char *path, const char *mode, unf_access_t access, ...);

Opens an unformatted file indicated by filename and returns a file stream
associated with that file.

'mode' is used to determine the file access mode:
 * "r" - open a file for reading
 * "w" - create a file for writing
 * "a" - append to a file

'access':
 * UNF_ACCESS_SEQUENTIAL
 * UNF_ACCESS_DIRECT
 * UNF_ACCESS_STREAM

If successful, returns a pointer to the object that controls the opened file stream.
On error, returns a null pointer (and errno is set in this case).

### unf_close
    int unf_close(unf_file_t *file);

Closes the given unformatted file.
Whether or not the operation succeeds, the stream is no longer associated with a file.
The behavior is undefined if the value of the pointer stream is used after
**unf_close()** returns.

Returns UNF_SUCCESS upon success, UNF_ERROR otherwise.

### unf_write
    int unf_write(unf_file_t *file, char *fmt, ...);

Writes data (the next record) to sequential and stream access files.
Arrays must be passed to the function by pointer, scalars and array dimensions
must be passed by value.
Format string is used to specify argument data types.
Format string consists of type specifiers separated by commas.

Available type specifiers are:
 * "c" - character
 * "c<N>" - Fortran string (not null-terminated), N stands for the lenght in bytes
 * "i1", "i2", "i4", "i8" - integer numbers from integer(1) = int8_t to integer(8) = int64_t
 * "r4", "r8" - real numbers, real(4) = float and real(8) = double, respectively
 * "z4", "z8" - complex numbers, complex(4) = float _Complex and complex(8) = double _Complex, respectively

Arrays are specified by addition of the "[i4]" or "[i8]" formats,
where "i4" (or "i8") denotes the type of the variable used to specify array length.

Returns the number of receiving arguments successfully written
(which may be zero in case a matching failure occurred before
the first receiving argument was assigned).

The [unf_eof()](#unf_eof) and [unf_error()](#unf_error) functions and the errno variable are to be used to handle errors.

### unf_write_rec
    int unf_write_rec(unf_file_t *file, int rec, char *fmt, ...);

Writes data to the record number 'rec' of a direct access file.
Arrays must be passed to the function by pointer, scalars and array dimensions
must be passed by value.
Format string is used to specify argument data types; see the description of the
[unf_write()](#unf_write) function for details.

Returns the number of receiving arguments successfully written
(which may be zero in case a matching failure occurred before
the first receiving argument was assigned).

The [unf_eof()](#unf_eof) and [unf_error()](#unf_error) functions and the errno variable are to be used to handle errors.

### unf_read
    int unf_read(unf_file_t *file, char *fmt, ...);

Reads data (the next record) from sequential and stream access files.
All arguments, both arrays and scalars, must be passed to the function by pointer.
Format string is used to specify argument data types.
Format string consists of type specifiers separated by commas.

Available type specifiers are:
 * "c" - character
 * "c<N>" - Fortran string (not null-terminated), N stands for the lenght in bytes
 * "i1", "i2", "i4", "i8" - integer numbers from integer(1) = int8_t to integer(8) = int64_t
 * "r4", "r8" - real numbers, real(4) = float and real(8) = double, respectively
 * "z4", "z8" - complex numbers, complex(4) = float _Complex and complex(8) = double _Complex, respectively

Arrays are specified by addition of the "[i4]" or "[i8]" formats,
where "i4" (or "i8") denotes the type of the variable used to specify array length.

Returns the number of receiving arguments successfully assigned
(which may be zero in case a matching failure occurred before
the first receiving argument was assigned).

The [unf_eof()](#unf_eof) and [unf_error()](#unf_error) functions and the errno variable are to be used to handle errors.

### unf_read_rec
    int unf_read_rec(unf_file_t *file, int rec, char *fmt, ...);

Reads data from record number 'rec'.
For direct access files only.
All arguments, both arrays and scalars, must be passed to the function by pointer.
Format string is used to specify argument data types; see the description of the
[unf_read()](#unf_read) function for details.

Returns the number of receiving arguments successfully assigned
(which may be zero in case a matching failure occurred before
the first receiving argument was assigned).

The [unf_eof()](#unf_eof) and [unf_error()](#unf_error) functions and the errno variable are to be used to handle errors.

### unf_next_rec_size
    int unf_next_rec_size(unf_file_t *file);

Returns size of the next record (in bytes).
For sequential access files only.

### unf_seek
    int unf_seek(unf_file_t *file, unf_position_t pos, int offset);

Sets the record position indicator for the sequential unformatted file
to the value pointed to by offset.
This function is similar to [fseek()](https://en.cppreference.com/w/c/io/fseek), but positions of records are used
rather than position of bytes.

Position 'pos' to which offset is added can have one of the following values:
 * UNF_POS_BEGIN     - from beginning of the file
 * UNF_POS_CURRENT   - the current file position
 * UNF_POS_END       - end of the file

Offset is a number of records to shift the position relative to origin.

Returns UNF_SUCCESS upon success, UNF_ERROR otherwise.

### unf_rewind
    int unf_rewind(unf_file_t *file);

Positions the unformatted file to its initial point.
The next [unf_read()](#unf_read) call will read the first record.
For sequential files only.

Returns UNF_SUCCESS upon success, UNF_ERROR otherwise.

### unf_backspace
    int unf_backspace(unf_file_t *file);

Positions the specified file to just before the preceding record.
For sequential files only.

Returns UNF_SUCCESS upon success, UNF_ERROR otherwise.

### unf_skip
    int unf_skip(unf_file_t *file);

Skips the next record.
For sequential files only.

Returns UNF_SUCCESS upon success, UNF_ERROR otherwise.

### unf_eof
    int unf_eof(unf_file_t *file);

Checks if the end of the given unformatted file has been reached.

Returns nonzero value if the end of the file has been reached, otherwise 0.

### unf_error
    int unf_error(unf_file_t *file);

Checks the given unformatted file for errors.

Returns nonzero value if the file stream has errors occurred, 0 otherwise.

## Useful links

- [FORTRAN 77 Language Reference. Unformatted I/O](https://docs.oracle.com/cd/E19957-01/805-4939/6j4m0vnc4/index.html)
- [File format of unformatted sequential files](https://gcc.gnu.org/onlinedocs/gfortran/File-format-of-unformatted-sequential-files.html)
- [Dealing with Binary Data Files](https://atmos.washington.edu/~salathe/osx_unix/endian.html)
- [Unformatted Direct-access Files](http://rsusu1.rnd.runnet.ru/develop/fortran/prof77/node166.html)
- [Stream Input/Output in Fortran](https://fortranwiki.org/fortran/show/Stream+Input+Output)





