#include "../Mapserver/HigherLayers/WeatherTableReader.h"
#include <cassert>
#include <cstring>

static bool ReadFixture(const char* text, short (&values)[2][1][2])
{
    FILE* file = std::tmpfile();
    assert(file);
    assert(std::fwrite(text, 1, std::strlen(text), file) == std::strlen(text));
    std::rewind(file);
    const bool result = ReadLegacyWeatherTable(file, values, 1);
    std::fclose(file);
    return result;
}

int main(int argc, char** argv)
{
    struct Fixture { unsigned long before; short values[2][1][2]; unsigned long after; };
    Fixture fixture = {0xa5a5a5a5, {}, 0x5a5a5a5a};
    assert(ReadFixture("1 -61 -122 14 -50\n", fixture.values));
    assert(fixture.values[0][0][0] == -61 && fixture.values[1][0][1] == -50);
    assert(fixture.before == 0xa5a5a5a5 && fixture.after == 0x5a5a5a5a);
    assert(!ReadFixture("1 40000 0 0 0\n", fixture.values));
    assert(!ReadFixture("1 99999999999999999999999999 0 0 0\n", fixture.values));
    assert(!ReadFixture("1 0 0 0\n", fixture.values));
    assert(!ReadFixture("2 0 0 0 0\n", fixture.values));
    assert(!ReadFixture("1 0 0 0 0 extra\n", fixture.values));
    assert(!ReadLegacyWeatherTable(static_cast<FILE*>(nullptr), fixture.values, 1));
    if (argc != 3) { std::fprintf(stderr, "Supply rain and temperature resources\n"); return 2; }
    FILE* rainFile = std::fopen(argv[1], "rt");
    FILE* temperatureFile = std::fopen(argv[2], "rt");
    if (!rainFile || !temperatureFile) {
        if (rainFile) std::fclose(rainFile);
        if (temperatureFile) std::fclose(temperatureFile);
        std::fprintf(stderr, "Resource open failed\n"); return 2;
    }
    int rain[12][31][1] = {};
    short temperature[12][31][2] = {};
    const bool valid = ReadLegacyWeatherTable(rainFile, rain, 30) &&
        ReadLegacyWeatherTable(temperatureFile, temperature, 31);
    std::fclose(rainFile); std::fclose(temperatureFile);
    if (!valid) { std::fprintf(stderr, "Real resource validation failed\n"); return 1; }
    assert(rain[4][0][0] == 143 && temperature[0][0][0] == -61 && temperature[0][0][1] == -122);
    std::printf("Weather rows/range/truncation/short guards and real resources passed\n");
    return 0;
}
