#ifndef TOML11_DATETIME
#define TOML11_DATETIME
#include <chrono>
#include <iomanip>
#include <ctime>

namespace toml
{

template<typename unsignedT, typename intT>
struct basic_datetime
{
    typedef unsignedT number_type;
    typedef intT      offset_type;
    constexpr static unsignedT undef    = std::numeric_limits<unsignedT>::max();
    constexpr static intT      nooffset = std::numeric_limits<intT>::max();

    unsignedT year;
    unsignedT month;
    unsignedT day;
    unsignedT hour;
    unsignedT minute;
    unsignedT second;
    unsignedT millisecond;
    unsignedT microsecond;
    intT      offset_hour;
    intT      offset_minute;

    basic_datetime() = default;
    ~basic_datetime() = default;
    basic_datetime(const basic_datetime&) = default;
    basic_datetime(basic_datetime&&) = default;
    basic_datetime& operator=(const basic_datetime&) = default;
    basic_datetime& operator=(basic_datetime&&) = default;

    basic_datetime(unsignedT y, unsignedT m, unsignedT d)
        : year(y), month(m), day(d), hour(undef), minute(undef), second(undef),
          millisecond(undef), microsecond(undef),
          offset_hour(nooffset), offset_minute(nooffset)
    {}
    basic_datetime(unsignedT h, unsignedT m, unsignedT s,
                   unsignedT ms, unsignedT us)
        : year(undef), month(undef), day(undef), hour(h), minute(m), second(s),
          millisecond(ms), microsecond(us),
          offset_hour(nooffset), offset_minute(nooffset)
    {}
    basic_datetime(unsignedT y, unsignedT mth, unsignedT d,
                   unsignedT h, unsignedT min, unsignedT s,
                   unsignedT ms, unsignedT us)
        : year(y), month(mth), day(d), hour(h), minute(min), second(s),
          millisecond(ms), microsecond(us),
          offset_hour(nooffset), offset_minute(nooffset)
    {}
    basic_datetime(unsignedT y, unsignedT mth, unsignedT d,
                   unsignedT h, unsignedT min, unsignedT s,
                   unsignedT ss, unsignedT us, intT oh, intT om)
        : year(y), month(mth), day(d), hour(h), minute(min), second(s),
          millisecond(ss), microsecond(us), offset_hour(oh), offset_minute(om)
    {}

    basic_datetime(std::chrono::system_clock::time_point tp);
    basic_datetime(std::time_t t);

    operator std::chrono::system_clock::time_point() const
    {
        std::tm time;
        if(this->year == undef || this->month == undef || this->day == undef)
        {
            const auto now = std::chrono::system_clock::now();
            const auto t = std::chrono::system_clock::to_time_t(now);
            std::tm* t_ = std::localtime(&t);
            time.tm_year = t_->tm_year;
            time.tm_mon  = t_->tm_mon;
            time.tm_mday = t_->tm_mday;
        }
        else
        {
            time.tm_year = this->year - 1900;
            time.tm_mon  = this->month - 1;
            time.tm_mday = this->day;
        }
        time.tm_hour = (this->hour   == undef) ? 0 : this->hour;
        time.tm_min  = (this->minute == undef) ? 0 : this->minute;
        time.tm_sec  = (this->second == undef) ? 0 : this->second;

        auto tp = std::chrono::system_clock::from_time_t(std::mktime(&time));
        tp += std::chrono::milliseconds(this->millisecond);
        tp += std::chrono::microseconds(this->microsecond);
        // mktime regards the tm struct as localtime. so adding offset is not needed.

        return tp;
    }
    operator std::time_t() const
    {
        return std::chrono::system_clock::to_time_t(
               std::chrono::system_clock::time_point(*this));
    }
};

template<typename uT, typename iT>
basic_datetime<uT, iT>::basic_datetime(std::chrono::system_clock::time_point tp)
{
    const auto t = std::chrono::system_clock::to_time_t(tp);
    std::tm *time = std::localtime(&t);
    this->year   = time->tm_year + 1900;
    this->month  = time->tm_mon + 1;
    this->day    = time->tm_mday;
    this->hour   = time->tm_hour;
    this->minute = time->tm_min;
    this->second = time->tm_sec;
    auto t_ = std::chrono::system_clock::from_time_t(std::mktime(time));
    auto diff = tp - t_;
    this->millisecond = std::chrono::duration_cast<std::chrono::milliseconds
        >(diff).count() % 1000;
    this->microsecond = std::chrono::duration_cast<std::chrono::microseconds
        >(diff).count() % 1000;

    std::tm *utc = std::gmtime(&t);
    int total_offset = (this->hour   - utc->tm_hour) * 60 +
                       (this->minute - utc->tm_min);
         if(total_offset >  720) total_offset -= 1440;
    else if(total_offset < -720) total_offset += 1440;
    offset_hour   = total_offset / 60;
    offset_minute = total_offset - (offset_hour * 60);
}

template<typename uT, typename iT>
basic_datetime<uT, iT>::basic_datetime(std::time_t t)
{
    *this = basic_datetime(std::chrono::system_clock::from_time_t(t));
}


template<typename charT, typename traits, typename uT, typename iT>
std::basic_ostream<charT, traits>&
operator<<(std::basic_ostream<charT, traits>& os, basic_datetime<uT, iT> const& dt)
{
    bool date = false;
    if(dt.year  != basic_datetime<uT, iT>::undef &&
       dt.month != basic_datetime<uT, iT>::undef &&
       dt.day   != basic_datetime<uT, iT>::undef)
    {
        os << std::setfill('0') << std::setw(4) << dt.year << '-'
           << std::setfill('0') << std::setw(2) << dt.month << '-'
           << std::setfill('0') << std::setw(2) << dt.day;
        date = true;
    }
    if(dt.hour   != basic_datetime<uT, iT>::undef &&
       dt.minute != basic_datetime<uT, iT>::undef &&
       dt.second != basic_datetime<uT, iT>::undef)
    {
        if(date) os << 'T';
        os << std::setfill('0') << std::setw(2) << dt.hour   << ':'
           << std::setfill('0') << std::setw(2) << dt.minute << ':'
           << std::setfill('0') << std::setw(2) << dt.second << '.'
           << std::setfill('0') << std::setw(3) << dt.millisecond
           << std::setfill('0') << std::setw(3) << dt.microsecond;
    }
    if(dt.offset_hour   != basic_datetime<uT, iT>::nooffset &&
       dt.offset_minute != basic_datetime<uT, iT>::nooffset)
    {
        if(dt.offset_hour == 0 && dt.offset_minute == 0)
        {
            os << 'Z';
        }
        else
        {
            char sign = ' ';
            iT oh = dt.offset_hour;
            iT om = dt.offset_minute;
            om += oh * 60;
            if(om > 0) sign = '+'; else sign='-';
            oh =  om / 60;
            om -= oh * 60;
            os << sign << std::setfill('0') << std::setw(2) << std::abs(oh) << ':'
                       << std::setfill('0') << std::setw(2) << std::abs(om);
        }
    }
    return os;
}

template<typename uT, typename iT>
inline bool
operator==(basic_datetime<uT, iT> const& lhs, basic_datetime<uT, iT> const& rhs)
{
    return lhs.year   == rhs.year   && lhs.month  == rhs.month  &&
           lhs.day    == rhs.day    && lhs.minute == rhs.minute &&
           lhs.second == rhs.second && lhs.millisecond == rhs.millisecond &&
           lhs.microsecond == rhs.microsecond &&
           lhs.offset_hour == rhs.offset_hour &&
           lhs.offset_minute == rhs.offset_minute;
}

template<typename uT, typename iT>
inline bool
operator!=(basic_datetime<uT, iT> const& lhs, basic_datetime<uT, iT> const& rhs)
{
    return !(lhs == rhs);
}

template<typename uT, typename iT>
inline bool
operator<(basic_datetime<uT, iT> const& lhs, basic_datetime<uT, iT> const& rhs)
{
    return std::time_t(lhs) < std::time_t(rhs);
}

template<typename uT, typename iT>
inline bool
operator<=(basic_datetime<uT, iT> const& lhs, basic_datetime<uT, iT> const& rhs)
{
    return std::time_t(lhs) <= std::time_t(rhs);
}

template<typename uT, typename iT>
inline bool
operator>(basic_datetime<uT, iT> const& lhs, basic_datetime<uT, iT> const& rhs)
{
    return std::time_t(lhs) > std::time_t(rhs);
}

template<typename uT, typename iT>
inline bool
operator>=(basic_datetime<uT, iT> const& lhs, basic_datetime<uT, iT> const& rhs)
{
    return std::time_t(lhs) >= std::time_t(rhs);
}


}//toml
#endif// TOML11_DATETIME
