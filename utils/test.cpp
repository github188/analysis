#include <iostream>
#include "smart_pointer.hpp"
#include "inheritance_ship.hpp"
#include "adv_string.h"
#include "stl_string_ex.hpp"


typedef e7::utils::object object_t;

class may : public object_t
{
public:
    may(void) : __data__(0)
    {std::cout << "may cstct:" << this << std::endl;}
    may(intptr_t x) : __data__(x)
    {std::cout << "may cstct:" << this << std::endl;}
    may(may const &other) : __data__(other.__data__)
    {std::cout << "may copy cstct:" << this << std::endl;}
    ~may(void)
    {std::cout << "may dstct:" << this << std::endl;}

    void print_data(void)
    {
        std::cout << __data__ << std::endl;
    }
    void set_data(intptr_t x)
    {
        __data__ = x;
    }

private:
    intptr_t __data__;
};

int main(int argc, char *argv[])
{
#if 0
    e7::utils::smart_pointer<may> p1(new may(2));
    e7::utils::smart_pointer<may> p2(new may(7));
    e7::utils::smart_pointer<may> null_pointer;

    p2.copy(p1);
    p2->print_data();
    p1 = null_pointer;
    p2 = null_pointer;
    std::cout << "end" << std::endl;
#endif

    // string_ex
    std::string_ex s("hello");

    std::list<std::string> l;
    s.to_list(l, "lo");
    std::cout << l.size() << std::endl;
    for (std::list<std::string>::iterator it = l.begin();
         l.end() != it;
         ++it)
    {
        std::cout << *it << std::endl;
    }

    std::string sss = static_cast<std::string>(s);
    static_cast<std::string &>(s).clear();
    std::cout << sss << std::endl;
    std::cout << static_cast<std::string &>(s) << std::endl;

    return 0;
}
