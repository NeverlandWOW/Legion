/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef COMMON_H
#define COMMON_H

#include "Define.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <errno.h>
#include <signal.h>
#include <assert.h>

#if PLATFORM == TC_PLATFORM_WINDOWS
#define STRCASECMP stricmp
#else
#define STRCASECMP strcasecmp
#endif

#include <set>
#include <unordered_set>
#include <list>
#include <string>
#include <map>
#include <unordered_map>
#include <queue>
#include <sstream>
#include <memory>
#include <vector>
#include <array>

#include <boost/optional.hpp>
#include <boost/utility/in_place_factory.hpp>
#include <boost/algorithm/clamp.hpp>
#include "Debugging/Errors.h"

#if PLATFORM == TC_PLATFORM_WINDOWS
#  include <ws2tcpip.h>

#  if defined(__INTEL_COMPILER)
#    if !defined(BOOST_ASIO_HAS_MOVE)
#      define BOOST_ASIO_HAS_MOVE
#    endif // !defined(BOOST_ASIO_HAS_MOVE)
#  endif // if defined(__INTEL_COMPILER)

#else
#  include <sys/types.h>
#  include <sys/ioctl.h>
#  include <sys/socket.h>
#  include <netinet/in.h>
#  include <unistd.h>
#  include <netdb.h>
#endif

#if COMPILER == COMPILER_MICROSOFT

#include <float.h>

#define I32FMT "%08I32X"
#define I64FMT "%016I64X"
#if defined(_MSC_VER) && _MSC_VER < 1500 // VC++ 8.0 and below
#define snprintf _snprintf
#endif
#define atoll _atoi64
#define vsnprintf _vsnprintf
#define finite(X) _finite(X)
#define llabs _abs64

#else

#define stricmp strcasecmp
#define strnicmp strncasecmp
#define I32FMT "%08X"
#define I64FMT "%016llX"

#endif

inline float finiteAlways(float f) { return finite(f) ? f : 0.0f; }

inline unsigned long atoul(char const* str) { return strtoul(str, nullptr, 10); }
inline unsigned long long atoull(char const* str) { return strtoull(str, nullptr, 10); }

#define STRINGIZE(a) #a

enum TimeConstants
{
    ONE_SECOND      = 1,
    THREE_SECONDS   = 3,
    FIVE_SECONDS    = 5,
    HALF_MINUTE     = 30,
    MINUTE          = 60,
    HOUR            = MINUTE*60,
    DAY             = HOUR*24,
    WEEK            = DAY*7,
    MONTH           = DAY*30,
    YEAR            = MONTH*12,
    IN_MILLISECONDS = 1000
};

enum AccountTypes : uint8
{
    SEC_PLAYER                = 0,
    SEC_MODERATOR             = 1,
    SEC_GAMEMASTER            = 2,
    SEC_CONFIRMED_GAMEMASTER  = 3,
    SEC_REALM_LEADER          = 4,
    SEC_GM_LEADER             = 5,
    SEC_ADMINISTRATOR         = 6,
    SEC_CONSOLE               = 7                                  // must be always last in list, accounts must have less security level always also
};

enum LocaleConstant
{
    LOCALE_enUS = 0,
    LOCALE_koKR = 1,
    LOCALE_frFR = 2,
    LOCALE_deDE = 3,
    LOCALE_zhCN = 4,
    LOCALE_zhTW = 5,
    LOCALE_esES = 6,
    LOCALE_esMX = 7,
    LOCALE_ruRU = 8,
    LOCALE_none = 9,
    LOCALE_ptBR = 10,
    LOCALE_itIT = 11,

    MAX_LOCALES
};

const uint8 TOTAL_LOCALES = 11;
const LocaleConstant DEFAULT_LOCALE = LOCALE_enUS;

const uint8 MAX_ACCOUNT_TUTORIAL_VALUES = 8;

extern char const* localeNames[MAX_LOCALES];

LocaleConstant GetLocaleByName(const std::string& name);

typedef std::vector<std::string> StringVector;
typedef std::set<std::string> StringSet;
typedef std::unordered_set<std::string> StringUnorderedSet;

struct LocalizedString
{
    char const* Str[MAX_LOCALES];

    LocalizedString();
    explicit LocalizedString(char const* val);
    char const* Get(uint32 locale) const;
};

// we always use stdlibc++ std::max/std::min, undefine some not C++ standard defines (Win API and some other platforms)
#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

#ifndef M_PI
#define M_PI            3.14159265358979323846f
#endif

#ifndef M_PI_F
#  define M_PI_F        float(M_PI)
#endif

#ifndef M_RAD
#define M_RAD           57.295779513082320876846364344191f
#endif

static uint32 constexpr MAX_QUERY_LEN = 32 * 1024;

//! Optional helper class to wrap optional values within.
template <typename T>
using Optional = boost::optional<T>;

namespace Trinity
{
    // using std::make_unique;
    //! std::make_unique implementation (TODO: remove this once C++14 is supported)
    // Overload for non-array types. Arguments are forwarded to T's constructor.

    template <class T>
    struct _Unique_if {
      typedef std::unique_ptr<T> _Single_object;
    };

    template <class T>
    struct _Unique_if<T[]> {
      typedef std::unique_ptr<T[]> _Unknown_bound;
    };

    template <class T, size_t N>
    struct _Unique_if<T[N]> {
      typedef void _Known_bound;
    };

    template <class T, class... Args>
    typename _Unique_if<T>::_Single_object make_unique(Args&&... args) {
      return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
    }

    template <class T>
    typename _Unique_if<T>::_Unknown_bound make_unique(size_t n) {
      typedef typename std::remove_extent<T>::type U;
      return std::unique_ptr<T>(new U[n]());
    }

    template <class T, class... Args>
    typename _Unique_if<T>::_Known_bound make_unique(Args&&...) = delete;

}

#endif
