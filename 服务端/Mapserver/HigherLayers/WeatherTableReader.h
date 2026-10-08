#pragma once
#include <cerrno>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <limits>

// Legacy resources contain one numbered day and all monthly values per line.
template<class Value, size_t Months, size_t Days, size_t Fields>
bool ReadLegacyWeatherTable(FILE* file, Value (&table)[Months][Days][Fields], size_t rows)
{
    if (!file || rows > Days) return false;
    for (size_t day = 0; day < rows; ++day)
    {
        char line[2048];
        if (!std::fgets(line, sizeof(line), file)) return false;
        char* cursor = line;
        for (size_t field = 0; field <= Months * Fields; ++field)
        {
            while (std::isspace(static_cast<unsigned char>(*cursor))) ++cursor;
            errno = 0;
            char* end = nullptr;
            const long number = std::strtol(cursor, &end, 10);
            if (end == cursor || errno == ERANGE) return false;
            if (field == 0)
            {
                if (number != static_cast<long>(day + 1)) return false;
            }
            else
            {
                if (number < (std::numeric_limits<Value>::min)() ||
                    number > (std::numeric_limits<Value>::max)()) return false;
                table[(field - 1) / Fields][day][(field - 1) % Fields] = static_cast<Value>(number);
            }
            cursor = end;
        }
        while (std::isspace(static_cast<unsigned char>(*cursor))) ++cursor;
        if (*cursor != '\0') return false;
    }
    return true;
}
