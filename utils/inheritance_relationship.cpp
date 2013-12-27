#include <iostream>


/********** 继承关系判断 **********/
#define PUBLIC public:
#define PROTECTED protected:
#define PRIVATE private:
template <typename base, typename derived>
class IsDerived
{
    /* type为无用模板形参，因为嵌套类模板只能偏特化 */
    PRIVATE template <int n, typename type=void> class CSizeBox
    {
        PRIVATE CSizeBox<n-1, type> m_box1;
        PRIVATE CSizeBox<n-1, type> m_box2;
    };
    PRIVATE template <typename type> class CSizeBox<0, type>
    {
        PRIVATE char m_c;
    };
    PRIVATE CSizeBox<0> Check(base const*) const;
    PRIVATE CSizeBox<1> Check(...) const;

    /* 检测继承关系，真表示继承关系成立 */
    PUBLIC bool operator ()(void) const
    {
        return (sizeof(Check(static_cast<base const*>(NULL))) == \
            sizeof(Check(static_cast<derived const*>(NULL))));
    }
    PUBLIC operator bool(void) const
    {
        return (*this)();
    }
};


class A
{};
class B : public A
{};
class C : public B
{};
class D
{};

int main(int argc, char *argv[])
{
    IsDerived<A, B> ab;
    IsDerived<B, C> bc;
    IsDerived<A, C> ac;
    IsDerived<A, D> ad;

    std::cout << "A <- B: " << ab() << std::endl;
    std::cout << "B <- C: " << bc() << std::endl;
    std::cout << "A <- C: " << ac() << std::endl;
    std::cout << "A <- D: " << ad() << std::endl;

    return 0;
}
