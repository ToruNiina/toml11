#ifndef TOML11_RESULT_H
#define TOML11_RESULT_H
#include <type_traits>
#include <utility>
#include <new>
#include <cassert>

namespace toml
{

template<typename T>
struct success
{
    using value_type = T;
    value_type value;

    explicit success(const value_type& v)
        noexcept(std::is_nothrow_copy_constructible<value_type>::value)
        : value(v)
    {}
    explicit success(value_type&& v)
        noexcept(std::is_nothrow_move_constructible<value_type>::value)
        : value(std::move(v))
    {}

    template<typename U>
    explicit success(U&& v): value(std::forward<U>(v)) {}

    template<typename U>
    explicit success(const success<U>& v): value(v.value) {}
    template<typename U>
    explicit success(success<U>&& v): value(std::move(v.value)) {}

    ~success() = default;
    success(const success&) = default;
    success(success&&)      = default;
    success& operator=(const success&) = default;
    success& operator=(success&&)      = default;
};

template<typename T>
struct failure
{
    using value_type = T;
    value_type value;

    explicit failure(const value_type& v)
        noexcept(std::is_nothrow_copy_constructible<value_type>::value)
        : value(v)
    {}
    explicit failure(value_type&& v)
        noexcept(std::is_nothrow_move_constructible<value_type>::value)
        : value(std::move(v))
    {}

    template<typename U>
    explicit failure(U&& v): value(std::forward<U>(v)) {}

    template<typename U>
    explicit failure(const failure<U>& v): value(v.value) {}
    template<typename U>
    explicit failure(failure<U>&& v): value(std::move(v.value)) {}

    ~failure() = default;
    failure(const failure&) = default;
    failure(failure&&)      = default;
    failure& operator=(const failure&) = default;
    failure& operator=(failure&&)      = default;
};

template<typename T>
success<typename std::remove_cv<typename std::remove_reference<T>::type>::type>
ok(T&& v)
{
    return success<
        typename std::remove_cv<typename std::remove_reference<T>::type>::type
        >(std::forward<T>(v));
}
template<typename T>
failure<typename std::remove_cv<typename std::remove_reference<T>::type>::type>
err(T&& v)
{
    return failure<
        typename std::remove_cv<typename std::remove_reference<T>::type>::type
        >(std::forward<T>(v));
}

inline success<std::string> ok(const char* literal)
{
    return success<std::string>(std::string(literal));
}
inline failure<std::string> err(const char* literal)
{
    return failure<std::string>(std::string(literal));
}


template<typename T, typename E>
struct result
{
    using value_type = T;
    using error_type = E;
    using success_type = success<value_type>;
    using failure_type = failure<error_type>;

    result(const success_type& s): is_ok_(true)
    {
        auto tmp = ::new(std::addressof(this->succ)) success_type(s);
        assert(tmp == std::addressof(this->succ));
    }
    result(const failure_type& f): is_ok_(false)
    {
        auto tmp = ::new(std::addressof(this->fail)) failure_type(f);
        assert(tmp == std::addressof(this->fail));
    }
    result(success_type&& s): is_ok_(true)
    {
        auto tmp = ::new(std::addressof(this->succ)) success_type(std::move(s));
        assert(tmp == std::addressof(this->succ));
    }
    result(failure_type&& f): is_ok_(false)
    {
        auto tmp = ::new(std::addressof(this->fail)) failure_type(std::move(f));
        assert(tmp == std::addressof(this->fail));
    }

    template<typename U>
    result(const success<U>& s): is_ok_(true)
    {
        auto tmp = ::new(std::addressof(this->succ)) success_type(s.value);
        assert(tmp == std::addressof(this->succ));
    }
    template<typename U>
    result(const failure<U>& f): is_ok_(false)
    {
        auto tmp = ::new(std::addressof(this->fail)) failure_type(f.value);
        assert(tmp == std::addressof(this->fail));
    }
    template<typename U>
    result(success<U>&& s): is_ok_(true)
    {
        auto tmp = ::new(std::addressof(this->succ)) success_type(std::move(s.value));
        assert(tmp == std::addressof(this->succ));
    }
    template<typename U>
    result(failure<U>&& f): is_ok_(false)
    {
        auto tmp = ::new(std::addressof(this->fail)) failure_type(std::move(f.value));
        assert(tmp == std::addressof(this->fail));
    }

    result& operator=(const success_type& s)
    {
        this->cleanup();
        this->is_ok_ = true;
        auto tmp = ::new(std::addressof(this->succ)) success_type(s);
        assert(tmp == std::addressof(this->succ));
        return *this;
    }
    result& operator=(const failure_type& f)
    {
        this->cleanup();
        this->is_ok_ = false;
        auto tmp = ::new(std::addressof(this->fail)) failure_type(f);
        assert(tmp == std::addressof(this->fail));
        return *this;
    }
    result& operator=(success_type&& s)
    {
        this->cleanup();
        this->is_ok_ = true;
        auto tmp = ::new(std::addressof(this->succ)) success_type(std::move(s));
        assert(tmp == std::addressof(this->succ));
        return *this;
    }
    result& operator=(failure_type&& f)
    {
        this->cleanup();
        this->is_ok_ = false;
        auto tmp = ::new(std::addressof(this->fail)) failure_type(std::move(f));
        assert(tmp == std::addressof(this->fail));
        return *this;
    }

    template<typename U>
    result& operator=(const success<U>& s)
    {
        this->cleanup();
        this->is_ok_ = true;
        auto tmp = ::new(std::addressof(this->succ)) success_type(s.value);
        assert(tmp == std::addressof(this->succ));
        return *this;
    }
    template<typename U>
    result& operator=(const failure<U>& f)
    {
        this->cleanup();
        this->is_ok_ = false;
        auto tmp = ::new(std::addressof(this->fail)) failure_type(f.value);
        assert(tmp == std::addressof(this->fail));
        return *this;
    }
    template<typename U>
    result& operator=(success<U>&& s)
    {
        this->cleanup();
        this->is_ok_ = true;
        auto tmp = ::new(std::addressof(this->succ)) success_type(std::move(s.value));
        assert(tmp == std::addressof(this->succ));
        return *this;
    }
    template<typename U>
    result& operator=(failure<U>&& f)
    {
        this->cleanup();
        this->is_ok_ = false;
        auto tmp = ::new(std::addressof(this->fail)) failure_type(std::move(f.value));
        assert(tmp == std::addressof(this->fail));
        return *this;
    }

    ~result() noexcept {this->cleanup();}

    result(const result& other): is_ok_(other.is_ok())
    {
        if(other.is_ok())
        {
            auto tmp = ::new(std::addressof(this->succ)) success_type(other.as_ok());
            assert(tmp == std::addressof(this->succ));
        }
        else
        {
            auto tmp = ::new(std::addressof(this->fail)) failure_type(other.as_err());
            assert(tmp == std::addressof(this->fail));
        }
    }
    result(result&& other): is_ok_(other.is_ok())
    {
        if(other.is_ok())
        {
            auto tmp = ::new(std::addressof(this->succ)) success_type(other.as_ok());
            assert(tmp == std::addressof(this->succ));
        }
        else
        {
            auto tmp = ::new(std::addressof(this->fail)) failure_type(other.as_err());
            assert(tmp == std::addressof(this->fail));
        }
    }

    template<typename U, typename F>
    result(const result<U, F>& other): is_ok_(other.is_ok())
    {
        if(other.is_ok())
        {
            auto tmp = ::new(std::addressof(this->succ)) success_type(other.as_ok());
            assert(tmp == std::addressof(this->succ));
        }
        else
        {
            auto tmp = ::new(std::addressof(this->fail)) failure_type(other.as_err());
            assert(tmp == std::addressof(this->fail));
        }
    }
    template<typename U, typename F>
    result(result<U, F>&& other): is_ok_(other.is_ok())
    {
        if(other.is_ok())
        {
            auto tmp = ::new(std::addressof(this->succ)) success_type(other.as_ok());
            assert(tmp == std::addressof(this->succ));
        }
        else
        {
            auto tmp = ::new(std::addressof(this->fail)) failure_type(other.as_err());
            assert(tmp == std::addressof(this->fail));
        }
    }

    result& operator=(const result& other)
    {
        this->cleanup();
        if(other.is_ok())
        {
            auto tmp = ::new(std::addressof(this->succ)) success_type(other.as_ok());
            assert(tmp == std::addressof(this->succ));
        }
        else
        {
            auto tmp = ::new(std::addressof(this->fail)) failure_type(other.as_err());
            assert(tmp == std::addressof(this->fail));
        }
        is_ok_ = other.is_ok();
        return *this;
    }
    result& operator=(result&& other)
    {
        this->cleanup();
        if(other.is_ok())
        {
            auto tmp = ::new(std::addressof(this->succ)) success_type(std::move(other.as_ok()));
            assert(tmp == std::addressof(this->succ));
        }
        else
        {
            auto tmp = ::new(std::addressof(this->fail)) failure_type(std::move(other.as_err()));
            assert(tmp == std::addressof(this->fail));
        }
        is_ok_ = other.is_ok();
        return *this;
    }

    template<typename U, typename F>
    result& operator=(const result<U, F>& other)
    {
        this->cleanup();
        if(other.is_ok())
        {
            auto tmp = ::new(std::addressof(this->succ)) success_type(other.as_ok());
            assert(tmp == std::addressof(this->succ));
        }
        else
        {
            auto tmp = ::new(std::addressof(this->fail)) failure_type(other.as_err());
            assert(tmp == std::addressof(this->fail));
        }
        is_ok_ = other.is_ok();
        return *this;
    }
    template<typename U, typename F>
    result& operator=(result<U, F>&& other)
    {
        this->cleanup();
        if(other.is_ok())
        {
            auto tmp = ::new(std::addressof(this->succ)) success_type(std::move(other.as_ok()));
            assert(tmp == std::addressof(this->succ));
        }
        else
        {
            auto tmp = ::new(std::addressof(this->fail)) failure_type(std::move(other.as_err()));
            assert(tmp == std::addressof(this->fail));
        }
        is_ok_ = other.is_ok();
        return *this;
    }

    bool is_ok()  const noexcept {return is_ok_;}
    bool is_err() const noexcept {return !is_ok_;}

    operator bool() const noexcept {return is_ok_;}

    value_type&       unwrap() &
    {
        if(is_err()) {throw std::runtime_error("result: bad unwrap");}
        return this->succ.value;
    }
    value_type const& unwrap() const&
    {
        if(is_err()) {throw std::runtime_error("result: bad unwrap");}
        return this->succ.value;
    }
    value_type&&      unwrap() &&
    {
        if(is_err()) {throw std::runtime_error("result: bad unwrap");}
        return std::move(this->succ.value);
    }

    error_type&       unwrap_err() &
    {
        if(is_ok()) {throw std::runtime_error("result: bad unwrap_err");}
        return this->fail.value;
    }
    error_type const& unwrap_err() const&
    {
        if(is_ok()) {throw std::runtime_error("result: bad unwrap_err");}
        return this->fail.value;
    }
    error_type&&      unwrap_err() &&
    {
        if(is_ok()) {throw std::runtime_error("result: bad unwrap_err");}
        return std::move(this->fail.value);
    }

    value_type&       as_ok() &      noexcept {return this->succ.value;}
    value_type const& as_ok() const& noexcept {return this->succ.value;}
    value_type&&      as_ok() &&     noexcept {return std::move(this->succ.value);}

    error_type&       as_err() &      noexcept {return this->fail.value;}
    error_type const& as_err() const& noexcept {return this->fail.value;}
    error_type&&      as_err() &&     noexcept {return std::move(this->fail.value);}

  private:

    void cleanup() noexcept
    {
        if(this->is_ok_) {this->succ.~success_type();}
        else             {this->fail.~failure_type();}
        return;
    }

  private:

    bool      is_ok_;
    union
    {
        success_type succ;
        failure_type fail;
    };
};


} // toml11
#endif// TOML11_RESULT_H
