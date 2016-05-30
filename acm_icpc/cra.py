#! /usr/bin/python


def calc_factorial(n):
    factorial = 1
    for i in xrange(n):
        factorial *= (i + 1)

    return factorial


# common formula
def cra1(r, a):
    assert(r >= a)

    if r == a:
        return 1

    r_fac = calc_factorial(r)
    a_fac = calc_factorial(a)
    r_a_fac = calc_factorial(r - a)

    return r_fac / a_fac / r_a_fac


# recursive formula
def cra2(r, a):
    assert(r >= a)

    if r == a:
        return 1

    if a == 1:
        return r

    return cra2(r - 1, a) + cra2(r - 1, a - 1)


# e7 formula
def cra_e7(r, a):
    assert(r >= a)

    if r == a:
        return 1

    if a == 1:
        return r

    result = 1

    for i in xrange(a):
        if (r - a < i + 1):
            break

        result += cra_e7(a, i + 1) * cra_e7(r - a, i + 1)

    return result


if __name__ == "__main__":
    print cra1(4, 2)
    print cra1(5, 2)
    print cra1(10, 4)
    print cra1(100, 41)

    print cra2(4, 2)
    print cra2(5, 2)
    print cra2(10, 4)
    print cra1(100, 41)

    print cra_e7(4, 2)
    print cra_e7(5, 2)
    print cra_e7(10, 4)
    print cra1(100, 41)
