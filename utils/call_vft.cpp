#include <cstdio>

class A
{
public:
    virtual void a(void)
    {
        printf("A:a(%p)\n", this);
    }

private:
    virtual void b(void)
    {
        printf("A:b(%p)\n", this);
    }
};

class B : public A
{
public:
    virtual void c(void)
    {
        printf("B:c(%p)\n", this);
    }

private:
    virtual void d(void)
    {
        printf("B:d(%p)\n", this);
    }
};

class C : public B
{
public:
    virtual void e(void)
    {
        printf("C:e(%p)\n", this);
    }

private:
    virtual void f(void)
    {
        printf("C:f(%p)\n", this);
    }
};

int main(void)
{
    A a;
    B b;
    C c;
    // void ***ppp_avf = (void ***)&a;
    void ***ppp_bvf = (void ***)&b;
    void ***ppp_cvf = (void ***)&c;

    printf("a, b, c: %p, %p, %p\n", &a, &b, &c);

    // for (int i = 0; NULL != (*ppp_avf)[i]; ++i) {
    //     ((void (*)(void))(*ppp_avf)[i])();
    // }
    // printf("\n");

    for (int i = 0; NULL != (*ppp_bvf)[i]; ++i) {
        ((void (*)(void *))(*ppp_bvf)[i])(&b);
    }
    printf("\n");

    for (int i = 0; NULL != (*ppp_cvf)[i]; ++i) {
        ((void (*)(void *))(*ppp_cvf)[i])(&c);
    }
    printf("\n");

    return 0;
}
