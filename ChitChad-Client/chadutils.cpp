#include "chadutils.h"

QString chad::stringViewToQString(std::string_view sv)
{
    return QString::fromLatin1(sv.data(), sv.size());
}
