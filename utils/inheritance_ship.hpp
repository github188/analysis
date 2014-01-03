namespace e7
{
    namespace utils
    {
        template <typename base, typename derived> class inheritance_ship;
    }
}

template <typename base, typename derived> class e7::utils::inheritance_ship
{
public:
    /* 检测继承关系，真表示继承关系成立 */
    bool operator ()(void) const
    {
        return (sizeof(check(static_cast<base const*>(0))) == \
            sizeof(check(static_cast<derived const*>(0))));
    }

    operator bool(void) const
    {
        return (*this)();
    }

private:
    /* type为无用模板形参，因为嵌套类模板只能偏特化 */
    template <int n, typename type=void> class __size_box__
    {
    private:
        __size_box__<n-1, type> __box1__;
        __size_box__<n-1, type> __box2__;
    };

    template <typename type> class __size_box__<0, type>
    {
    private:
        char __c__;
    };

private:
    __size_box__<0> check(base const*) const;
    __size_box__<1> check(...) const;

};
