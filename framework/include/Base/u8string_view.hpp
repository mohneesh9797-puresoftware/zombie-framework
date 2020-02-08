#ifndef OBS_BASE_U8STRING_VIEW_HPP
#define OBS_BASE_U8STRING_VIEW_HPP

#include "char8_t.hpp"

#include <string_view>

namespace Obs {

using u8string_view = std::basic_string_view<char8_t>;

}

#endif
