subroutine gen_ReadSequential_String() &
        bind (C, name="gen_ReadSequential_String")

    implicit none

    integer :: out = 12
    character(len=13) :: hello = "Hello, World!"

    open(out, file='numbers.unf', status='replace', form='unformatted', access='sequential')
    write(out) hello
    close(out)

end subroutine gen_ReadSequential_String


subroutine gen_ReadSequential_ShortIntegers() &
        bind (C, name="gen_ReadSequential_ShortIntegers")

    implicit none

    integer(1) :: int1 = 101
    integer(2) :: int2 = 102
    integer(4) :: int4 = 104
    integer(8) :: int8 = 108
    integer :: out = 12

    open(out, file='numbers.unf', status='replace', form='unformatted', access='sequential')
    write(out) int1, int2, int4, int8
    close(out)

end subroutine gen_ReadSequential_ShortIntegers


subroutine gen_ReadSequential_IntegerReal() &
        bind (C, name="gen_ReadSequential_IntegerReal")

    implicit none

    integer :: out = 12
    integer(4) :: a = 42
    integer(8) :: b = 123
    integer(4) :: c = 404
    integer(8) :: d = 8080
    real(8) :: pi = 3.14d0
    real(8) :: e = 2.72d0

    open(out, file='numbers.unf', status='replace', form='unformatted', access='sequential')
    write(out) a, b
    write(out) c, d, pi
    write(out) e
    close(out)

end subroutine gen_ReadSequential_IntegerReal


subroutine gen_ReadSequential_IntegerRealArrays() &
        bind (C, name="gen_ReadSequential_IntegerRealArrays")

    implicit none

    integer :: out = 12
    character(len=20) :: title = "table of cubes"
    integer(4), parameter :: n = 4
    integer(4), dimension(n) :: x
    real(8), dimension(n) :: y
    integer :: i

    do i = 1, n
        x(i) = i
        y(i) = dble(i)**3
    end do

    open(out, file='numbers.unf', status='replace', form='unformatted', access='sequential')
    write(out) title
    write(out) n
    write(out) x, y
    close(out)

end subroutine gen_ReadSequential_IntegerRealArrays


subroutine gen_ReadSequential_TwoMixedArrays() &
        bind (C, name="gen_ReadSequential_TwoMixedArrays")

    implicit none

    integer :: out = 12
    integer(4), parameter :: dim = 5
    integer(2) :: ints(dim) = (/ -1, -2, -3, -4, -5 /)
    real(8) :: reals(dim) = (/ 1.0d0, 4.0d0, 9.0d0, 16.0d0, 25.0d0 /)
    integer :: i

    open(out, file='numbers.unf', status='replace', form='unformatted', access='sequential')
    write(out) dim, (ints(i), reals(i), i = 1, dim)
    close(out)

end subroutine gen_ReadSequential_TwoMixedArrays


subroutine check_WriteSequential_ShortIntegers(int1, int2, int4, int8) &
        bind (C, name="check_WriteSequential_ShortIntegers")

    implicit none

    integer(1), intent(out) :: int1
    integer(2), intent(out) :: int2
    integer(4), intent(out) :: int4
    integer(8), intent(out) :: int8
    integer :: out = 12

    int1 = 0
    int2 = 0
    int4 = 0
    int8 = 0

    open(out, file='numbers.unf', status='old', form='unformatted', access='sequential')
    read(out) int1, int2, int4, int8
    close(out)

end subroutine check_WriteSequential_ShortIntegers


subroutine check_WriteSequential_NamedConstants(twelve, pi, imag_unit) &
        bind (C, name="check_WriteSequential_NamedConstants")

    implicit none

    integer(4), intent(out) :: twelve
    real(8), intent(out) :: pi
    complex(8), intent(out) :: imag_unit

    integer :: out = 12
    character(len=10) :: buf_twelve
    character(len=10) :: buf_pi
    character(len=10) :: buf_imag_unit

    twelve = 0
    pi = 0.0d0
    imag_unit = cmplx(0.0d0, 0.0d0)

    open(out, file='numbers.unf', status='old', form='unformatted', access='sequential')
    read(out) buf_twelve, twelve
    read(out) buf_pi, pi
    read(out) buf_imag_unit, imag_unit
    close(out)

end subroutine check_WriteSequential_NamedConstants


subroutine check_WriteSequential_IntegerRealArrays(num_elements, int4_array, real8_array) &
        bind (C, name="check_WriteSequential_IntegerRealArrays")

    implicit none

    integer(4), intent(out) :: num_elements
    integer(4), intent(out) :: int4_array(5)
    real(8), intent(out) :: real8_array(5)
    integer :: out = 12
    integer :: i

    num_elements = 0
    int4_array = 0
    real8_array = 0.0d0

    open(out, file='numbers.unf', status='old', form='unformatted', access='sequential')
    read(out) num_elements, (int4_array(i),i=1,num_elements)
    read(out) (real8_array(i),i=1,num_elements)
    close(out)

end subroutine check_WriteSequential_IntegerRealArrays


subroutine gen_ReadDirect_ReadRealMatrix() &
        bind (C, name="gen_ReadDirect_ReadRealMatrix")

    implicit none

    integer :: out = 12
    integer(4), parameter :: dim = 4
    integer(4) :: i, j, rec_count

    open(out, file='numbers.unf', status='replace', form='unformatted', access='direct', recl=16)
    rec_count = 1
    do i = 1, dim
        do j = 1, dim
            write(out,rec=rec_count) i, j, i + j / 10.0d0
            rec_count = rec_count + 1
        end do
    end do
    close(out)

end subroutine gen_ReadDirect_ReadRealMatrix


subroutine check_WriteDirect_WriteRealArrays(npoints, x, sinx, cosx) &
        bind (C, name="check_WriteDirect_WriteRealArrays")

    integer(4), intent(out) :: npoints
    real(8), intent(out) :: x(*)
    real(8), intent(out) :: sinx(*)
    real(8), intent(out) :: cosx(*)
    integer :: out = 12
    integer :: rec_count

    ! let the records be wider than needed
    open(out, file='numbers.unf', status='old', form='unformatted', access='direct', recl=30)

    rec_count = 1
    10 continue
    read(out,rec=rec_count,err=13) x(rec_count), sinx(rec_count), cosx(rec_count)
    rec_count = rec_count + 1
    goto 10

    13 continue
    close(out)
    npoints = rec_count - 1

end subroutine check_WriteDirect_WriteRealArrays


subroutine gen_ReadStream_ReadRealMatrix() &
        bind (C, name="gen_ReadStream_ReadRealMatrix")

    implicit none

    integer :: out = 12
    integer(4), parameter :: dim = 4
    integer(4) :: i, j, rec_count

    open(out, file='numbers.unf', status='replace', form='unformatted', access='stream')
    rec_count = 1
    do i = 1, dim
        do j = 1, dim
            write(out) i, j, i + j / 10.0d0
            rec_count = rec_count + 1
        end do
    end do
    close(out)

end subroutine gen_ReadStream_ReadRealMatrix


subroutine check_WriteStream_WriteRealArrays(npoints, x, sinx, cosx) &
        bind (C, name="check_WriteStream_WriteRealArrays")

    integer(4), intent(out) :: npoints
    real(8), intent(out) :: x(*)
    real(8), intent(out) :: sinx(*)
    real(8), intent(out) :: cosx(*)
    integer :: out = 12
    integer :: rec_count

    open(out, file='numbers.unf', status='old', form='unformatted', access='stream')

    rec_count = 1
    10 continue
    read(out,err=13,end=13) x(rec_count), sinx(rec_count), cosx(rec_count)
    rec_count = rec_count + 1
    goto 10

    13 continue
    close(out)
    npoints = rec_count - 1

end subroutine check_WriteStream_WriteRealArrays


subroutine gen_SequentialSeek_SeekBackward() &
        bind (C, name="gen_SequentialSeek_SeekBackward")

    implicit none

    integer(1) :: int1 = 101
    integer(2) :: int2 = 102
    integer(4) :: int4 = 104
    integer(8) :: int8 = 108
    integer :: out = 12

    open(out, file='numbers.unf', status='replace', form='unformatted', access='sequential')
    write(out) int1
    write(out) int2
    write(out) int4
    write(out) int8
    close(out)

end subroutine gen_SequentialSeek_SeekBackward
