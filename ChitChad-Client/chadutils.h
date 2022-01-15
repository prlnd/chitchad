#ifndef CHADUTILS_H
#define CHADUTILS_H

#include <QString>
#include <string_view>

namespace chad {
QString stringViewToQString(std::string_view sv);
}

#endif // CHADUTILS_H
