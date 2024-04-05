//
// Created by Alexander Oleynichenko on 07.01.2024.
//

#include <complex.h>
#include <gtest/gtest.h>

#include "libunf.h"
#include <complex.h>
#include <math.h>
#include <string.h>

extern "C" {
void gen_ReadSequential_String();
void gen_ReadSequential_ShortIntegers();
void gen_ReadSequential_IntegerReal();
void gen_ReadSequential_IntegerRealArrays();
void gen_ReadSequential_TwoMixedArrays();
void check_WriteSequential_ShortIntegers(int8_t *int1, int16_t *int2, int32_t *int4, int64_t *int8);
void check_WriteSequential_NamedConstants(int32_t *twelve, double *pi, double _Complex *imag_unit);
void check_WriteSequential_IntegerRealArrays(int32_t *num_elements, int32_t *int4_array, double *real8_array);
void gen_ReadDirect_ReadRealMatrix();
void check_WriteDirect_WriteRealArrays(int32_t *npoints, double *x, double *sinx, double *cosx);
void gen_ReadStream_ReadRealMatrix();
void check_WriteStream_WriteRealArrays(int32_t *npoints, double *x, double *sinx, double *cosx);
void gen_SequentialSeek_SeekBackward();
}


TEST(ReadSequential, String)
{
    gen_ReadSequential_String();

    unf_file_t *file = unf_open("numbers.unf", "r", UNF_ACCESS_SEQUENTIAL);
    ASSERT_NE(file, (unf_file_t *) NULL);

    char buf[16];
    int n_read = unf_read(file, "c13", buf);
    buf[13] = '\0';

    ASSERT_EQ(n_read, 1);
    ASSERT_STREQ(buf, "Hello, World!");

    unf_close(file);
}


TEST(ReadSequential, ShortIntegers)
{
    gen_ReadSequential_ShortIntegers();

    unf_file_t *file = unf_open("numbers.unf", "r", UNF_ACCESS_SEQUENTIAL);
    ASSERT_NE(file, (unf_file_t *) NULL);

    int8_t int1 = 0;
    int16_t int2 = 0;
    int32_t int4 = 0; // this value will be skipped (NULL in the format string)
    int64_t int8 = 0;
    int n_read = unf_read(file, "i1,i2,i4,i8", &int1, &int2, NULL, &int8);

    ASSERT_EQ(n_read, 4);
    ASSERT_EQ(unf_error(file), 0);
    ASSERT_EQ(int1, 101);
    ASSERT_EQ(int2, 102);
    ASSERT_EQ(int4, 0); // this integer value was skipped
    ASSERT_EQ(int8, 108);

    unf_close(file);
}


TEST(ReadSequential, IntegerReal)
{
    gen_ReadSequential_IntegerReal();

    unf_file_t *file = unf_open("numbers.unf", "r", UNF_ACCESS_SEQUENTIAL);
    ASSERT_NE(file, (unf_file_t *) NULL);

    // record 1
    int32_t a = 0;
    int64_t b = 0;
    int n_read = unf_read(file, "i4,i8", &a, &b);
    ASSERT_EQ(n_read, 2);
    ASSERT_EQ(a, 42);
    ASSERT_EQ(b, 123);

    // record 2
    int32_t c = 0;
    int64_t d = 0;
    double pi = 0.0;
    n_read = unf_read(file, "i4,i8,r8", &c, &d, &pi);
    ASSERT_EQ(n_read, 3);
    ASSERT_EQ(c, 404);
    ASSERT_EQ(d, 8080);
    ASSERT_DOUBLE_EQ(pi, 3.14);

    // record 3
    double e = 0.0;
    n_read = unf_read(file, "r8", &e);
    ASSERT_DOUBLE_EQ(e, 2.72);

    // try to read the last entry again
    n_read = unf_read(file, "r8", &e);
    ASSERT_NE(unf_eof(file), 0);
    ASSERT_EQ(n_read, 0);

    unf_close(file);
}


TEST(ReadSequential, IntegerRealArrays)
{
    gen_ReadSequential_IntegerRealArrays();

    unf_file_t *file = unf_open("numbers.unf", "r", UNF_ACCESS_SEQUENTIAL);
    ASSERT_NE(file, (unf_file_t *) NULL);

    // record 1: title
    char buf[20];
    int n_read = unf_read(file, "c20", buf);
    buf[14] = '\0';

    ASSERT_EQ(n_read, 1);
    ASSERT_STREQ(buf, "table of cubes");

    // record 2: number of entries
    int32_t n = 0;
    n_read = unf_read(file, "i4", &n);
    ASSERT_EQ(n_read, 1);
    ASSERT_EQ(n, 4);

    // record 3: arrays x, y=x^3
    int32_t x[10];
    double y[10];
    n_read = unf_read(file, "i4[i4],r8[i4]", x, &n, y, &n);
    ASSERT_EQ(n_read, 2);
    for (int i = 0; i < n; i++) {
        ASSERT_EQ(x[i], i + 1);
        ASSERT_DOUBLE_EQ(y[i], pow(i + 1, 3));
    }

    unf_close(file);
}


TEST(ReadSequential, TwoMixedArrays)
{
    gen_ReadSequential_TwoMixedArrays();

    // integer(4) :: dim
    // integer(2) :: ints(dim)
    // real(8) :: reals(dim)
    // write(out) dim, (ints(i), reals(i), i = 1, dim)

    // get array dim
    unf_file_t *file = unf_open("numbers.unf", "r", UNF_ACCESS_SEQUENTIAL);
    ASSERT_NE(file, (unf_file_t *) NULL);

    int32_t dim = 0;
    int n_read = unf_read(file, "i4", &dim);
    ASSERT_EQ(n_read, 1);
    ASSERT_EQ(unf_error(file), 0);
    ASSERT_EQ(dim, 5);

    // allocate memory for data
    int16_t *ints = (int16_t *) calloc(dim, sizeof(int16_t));
    double *reals = (double *) calloc(dim, sizeof(double));
    char *raw_data = (char *) calloc(dim, sizeof(int16_t) + sizeof(double));

    // read raw data as bytes
    int status = unf_backspace(file);
    ASSERT_EQ(status, UNF_SUCCESS);
    dim = 0;
    n_read = unf_read(file, "i4,c10[i4]", &dim, raw_data, &dim);
    ASSERT_EQ(n_read, 2);
    ASSERT_EQ(unf_error(file), 0);
    ASSERT_EQ(dim, 5);

    // decode array of raw bytes
    int entry_size = sizeof(int16_t) + sizeof(double);
    for (int i = 0; i < dim; i++) {
        ints[i] = *((int16_t *) (raw_data + entry_size * i));
        reals[i] = *((double *) (raw_data + entry_size * i + sizeof(int16_t)));
    }

    for (int i = 0; i < dim; i++) {
        int n = i + 1;
        ASSERT_EQ(ints[i], -n);
        ASSERT_DOUBLE_EQ(reals[i], n * n);
    }

    free(ints);
    free(reals);
    free(raw_data);
    unf_close(file);
}


TEST(WriteSequential, ShortIntegers)
{
    unf_file_t *file = unf_open("numbers.unf", "w", UNF_ACCESS_SEQUENTIAL);
    ASSERT_NE(file, (unf_file_t *) NULL);

    // write
    int8_t int1 = 101;
    int16_t int2 = 102;
    int32_t int4 = 104;
    int64_t int8 = 108;
    int n_written = unf_write(file, "i1,i2,i4,i8", int1, int2, int4, int8);
    ASSERT_EQ(n_written, 4);
    ASSERT_EQ(unf_error(file), 0);

    // read & compare
    int1 = 0;
    int2 = 0;
    int4 = 0;
    int8 = 0;
    check_WriteSequential_ShortIntegers(&int1, &int2, &int4, &int8);
    ASSERT_EQ(int1, 101);
    ASSERT_EQ(int2, 102);
    ASSERT_EQ(int4, 104);
    ASSERT_EQ(int8, 108);

    unf_close(file);
}


TEST(WriteSequential, NamedConstants)
{
    unf_file_t *file = unf_open("numbers.unf", "w", UNF_ACCESS_SEQUENTIAL);
    ASSERT_NE(file, (unf_file_t *) NULL);

    int n_written = unf_write(file, "c10,i4", "twelve    ", 12);
    ASSERT_EQ(n_written, 2);

    n_written = unf_write(file, "c10,r8", "pi        ", 3.14);
    ASSERT_EQ(n_written, 2);

    n_written = unf_write(file, "c10,z8", "imag unit ", 0.0 + 1.0i);
    ASSERT_EQ(n_written, 2);

    unf_close(file);

    // check results read from the unformatted file by the Fortran code
    int32_t check_twelve;
    double check_pi;
    double _Complex check_imag_unit;
    check_WriteSequential_NamedConstants(&check_twelve, &check_pi, &check_imag_unit);
    ASSERT_EQ(check_twelve, 12);
    ASSERT_DOUBLE_EQ(check_pi, 3.14);
    ASSERT_DOUBLE_EQ(check_imag_unit * check_imag_unit, -1.0);
}


TEST(WriteSequential, IntegerRealArrays)
{
    unf_file_t *file = unf_open("numbers.unf", "w", UNF_ACCESS_SEQUENTIAL);
    ASSERT_NE(file, (unf_file_t *) NULL);

    int32_t n = 5;
    int32_t buf_i[5];
    double buf_sq[5];
    for (int i = 0; i < n; i++) {
        buf_i[i] = i;
        buf_sq[i] = i * i;
    }

    int n_written = unf_write(file, "i4,i4[i4]", n, buf_i, n);
    ASSERT_EQ(n_written, 2);

    n_written = unf_write(file, "r8[i4]", buf_sq, n);
    ASSERT_EQ(n_written, 1);

    unf_close(file);

    n = 0;
    memset(buf_i, 0, sizeof(buf_i));
    memset(buf_sq, 0, sizeof(buf_sq));
    check_WriteSequential_IntegerRealArrays(&n, buf_i, buf_sq);
    ASSERT_EQ(n, 5);

    for (int i = 0; i < n; i++) {
        ASSERT_EQ(buf_i[i], i);
        ASSERT_DOUBLE_EQ(buf_sq[i], i * i);
    }
}


TEST(ReadDirect, ReadRealMatrix)
{
    gen_ReadDirect_ReadRealMatrix();

    int recl = 2 * sizeof(int32_t) + sizeof(double);
    unf_file_t *file = unf_open("numbers.unf", "r", UNF_ACCESS_DIRECT, recl);

    int32_t i, j;
    double val;
    int32_t count = 1;

    while (unf_read_rec(file, count, "2i4,r8", &i, &j, &val) == 3) {
        ASSERT_EQ(unf_error(file), UNF_SUCCESS);
        ASSERT_DOUBLE_EQ(i + j / 10.0, val);
        count++;
    }

    ASSERT_EQ(count - 1, 16);

    unf_close(file);
}


TEST(WriteDirect, WriteRealArrays)
{
    int32_t npoints = 10;
    double x[10];
    double y_sin[10];
    double y_cos[10];

    // generate file (C)
    // let the records be wider than needed
    int32_t recl = 3 * sizeof(double) + 6; // == 30
    unf_file_t *file = unf_open("numbers.unf", "w", UNF_ACCESS_DIRECT, recl);
    for (int i = 0; i < npoints; i++) {
        double x_i = ((double) i) / npoints;
        int n_written = unf_write_rec(file, npoints - i, "3r8", x_i, sin(x_i), cos(x_i));
        ASSERT_EQ(unf_error(file), UNF_SUCCESS);
        ASSERT_EQ(n_written, 3);
    }
    unf_close(file);

    // read file (Fortran)
    npoints = 0;
    memset(x, 0, sizeof(x));
    memset(y_sin, 0, sizeof(y_sin));
    memset(y_cos, 0, sizeof(y_cos));
    check_WriteDirect_WriteRealArrays(&npoints, x, y_sin, y_cos);
    ASSERT_EQ(npoints, 10);

    // check results
    for (int i = 0; i < npoints; i++) {
        double x_i = ((double) npoints - i - 1) / npoints;
        double sinx = sin(x_i);
        double cosx = cos(x_i);
        ASSERT_DOUBLE_EQ(x_i, x[i]);
        ASSERT_DOUBLE_EQ(sinx, y_sin[i]);
        ASSERT_DOUBLE_EQ(cosx, y_cos[i]);
    }
}


TEST(ReadStream, ReadRealMatrix)
{
    gen_ReadStream_ReadRealMatrix();

    unf_file_t *file = unf_open("numbers.unf", "r", UNF_ACCESS_STREAM);

    int32_t i, j;
    double val;
    int32_t count = 0;

    while (unf_read(file, "2i4,r8", &i, &j, &val) == 3) {
        ASSERT_EQ(unf_error(file), 0);
        ASSERT_DOUBLE_EQ(i + j / 10.0, val);
        count++;
    }

    ASSERT_EQ(count, 16);

    unf_close(file);
}


TEST(WriteStream, WriteRealArrays)
{
    int32_t npoints = 10;
    double x[10];
    double y_sin[10];
    double y_cos[10];

    // generate file (C)
    unf_file_t *file = unf_open("numbers.unf", "w", UNF_ACCESS_STREAM);
    for (int i = 0; i < npoints; i++) {
        double x_i = ((double) i) / npoints;
        unf_write(file, "3r8", x_i, sin(x_i), cos(x_i));
    }
    unf_close(file);

    // read file (Fortran)
    npoints = 0;
    memset(x, 0, sizeof(x));
    memset(y_sin, 0, sizeof(y_sin));
    memset(y_cos, 0, sizeof(y_cos));
    check_WriteStream_WriteRealArrays(&npoints, x, y_sin, y_cos);
    ASSERT_EQ(npoints, 10);

    // check results
    for (int i = 0; i < npoints; i++) {
        double x_i = ((double) i) / npoints;
        double sinx = sin(x_i);
        double cosx = cos(x_i);
        ASSERT_DOUBLE_EQ(x_i, x[i]);
        ASSERT_DOUBLE_EQ(sinx, y_sin[i]);
        ASSERT_DOUBLE_EQ(cosx, y_cos[i]);
    }
}


TEST(SequentialSeek, SeekBackward)
{
    gen_SequentialSeek_SeekBackward();

    unf_file_t *file = unf_open("numbers.unf", "r", UNF_ACCESS_SEQUENTIAL);
    ASSERT_NE(file, (unf_file_t *) NULL);

    int8_t int1 = 0;
    int16_t int2 = 0;
    int32_t int4 = 0;
    int64_t int8 = 0;

    int n_read = unf_read(file, "i1", &int1);
    ASSERT_EQ(n_read, 1);
    ASSERT_EQ(int1, 101);
    ASSERT_EQ(unf_error(file), UNF_SUCCESS);

    n_read = unf_read(file, "i2", &int2);
    ASSERT_EQ(n_read, 1);
    ASSERT_EQ(int2, 102);
    ASSERT_EQ(unf_error(file), UNF_SUCCESS);

    n_read = unf_read(file, "i4", &int4);
    ASSERT_EQ(n_read, 1);
    ASSERT_EQ(int4, 104);
    ASSERT_EQ(unf_error(file), UNF_SUCCESS);

    n_read = unf_read(file, "i8", &int8);
    ASSERT_EQ(n_read, 1);
    ASSERT_EQ(int8, 108);
    ASSERT_EQ(unf_error(file), UNF_SUCCESS);

    // rewind the last record
    int status = unf_seek(file, UNF_POS_CURRENT, -1);
    ASSERT_EQ(status, UNF_SUCCESS);

    // try to read the last entry again
    int8 = 0;
    n_read = unf_read(file, "i8", &int8);
    ASSERT_EQ(n_read, 1);
    ASSERT_EQ(int8, 108);
    ASSERT_EQ(unf_error(file), UNF_SUCCESS);

    // go to the beginning of the file
    status = unf_seek(file, UNF_POS_CURRENT, -4);
    ASSERT_EQ(status, UNF_SUCCESS);

    // try to read the first entry again
    int1 = 0;
    n_read = unf_read(file, "i1", &int1);
    ASSERT_EQ(n_read, 1);
    ASSERT_EQ(int1, 101);
    ASSERT_EQ(unf_error(file), UNF_SUCCESS);

    // go to the beginning of the file in the other way
    status = unf_seek(file, UNF_POS_BEGIN, 0);
    ASSERT_EQ(status, UNF_SUCCESS);

    // try to read the first entry again
    int1 = 0;
    n_read = unf_read(file, "i1", &int1);
    ASSERT_EQ(n_read, 1);
    ASSERT_EQ(int1, 101);
    ASSERT_EQ(unf_error(file), UNF_SUCCESS);

    // try to rewind too much
    status = unf_seek(file, UNF_POS_CURRENT, -2);
    ASSERT_EQ(status, UNF_ERROR);

    unf_close(file);
}


TEST(SequentialSeek, SeekForward)
{
    gen_SequentialSeek_SeekBackward();

    unf_file_t *file = unf_open("numbers.unf", "r", UNF_ACCESS_SEQUENTIAL);
    ASSERT_NE(file, (unf_file_t *) NULL);

    int16_t int2 = 0;
    int32_t int4 = 0;
    int64_t int8 = 0;

    // skip the first entry (int8_t number, 8 bytes)
    int status = unf_seek(file, UNF_POS_CURRENT, 1);
    ASSERT_EQ(status, UNF_SUCCESS);

    // begin reading from the second entry
    int n_read = unf_read(file, "i2", &int2);
    ASSERT_EQ(n_read, 1);
    ASSERT_EQ(int2, 102);
    ASSERT_EQ(unf_error(file), UNF_SUCCESS);

    n_read = unf_read(file, "i4", &int4);
    ASSERT_EQ(n_read, 1);
    ASSERT_EQ(int4, 104);
    ASSERT_EQ(unf_error(file), UNF_SUCCESS);

    n_read = unf_read(file, "i8", &int8);
    ASSERT_EQ(n_read, 1);
    ASSERT_EQ(int8, 108);
    ASSERT_EQ(unf_error(file), UNF_SUCCESS);

    // try to seek unexisting record after the end of file
    status = unf_seek(file, UNF_POS_CURRENT, 1);
    ASSERT_EQ(status, UNF_ERROR);
    ASSERT_NE(unf_eof(file), 0);

    unf_close(file);
}
