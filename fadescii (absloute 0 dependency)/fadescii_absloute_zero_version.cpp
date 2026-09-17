#include <iostream>
#include <cstdint>
#include <string>
#include <unistd.h>
#include <sys/ioctl.h>
#include <deque>



// helper class

class circularBuffer {
public:
    struct Glyph {
        int row;
        int col;
        char character;
    };

private:
    std::deque<Glyph> buffer;
    int count;

public:
    circularBuffer(int size) : count(size) {}

    void push(int row, int col, char character) {
        if (buffer.size() >= count)
            buffer.pop_front();

        buffer.push_back({row, col, character});
    }

    int size() const {
        return buffer.size();
    }

    Glyph& operator[](int index) {
        return buffer[index];
    }
};

// main class


class fadescii {

// important data needed to draw the box on the screen
private:
    bool isInitialized=false;

// box data to simulate the box that will be drawn on the screen
private:
    int originRow, originCol;
    int height, width;

// text metadata needed to draw the text on the screen
private:
    double speed=0.1;
    int glowLength=5;

    struct rgb {
        uint8_t r;
        uint8_t g;
        uint8_t b;
    };
    rgb textColor = {255, 255, 255};
    rgb glowColor = {255, 0, 0};

// helper functions
private:
    rgb convertColorCodesToRGB(std::string colorCode) {
        // hex case
        if(colorCode[0] == '#') {
            if(colorCode.length() != 7) {
                throw std::invalid_argument("Invalid hex color code");
            }
            uint8_t r = std::stoi(colorCode.substr(1, 2), nullptr, 16);
            uint8_t g = std::stoi(colorCode.substr(3, 2), nullptr, 16);
            uint8_t b = std::stoi(colorCode.substr(5, 2), nullptr, 16);
            return {r, g, b};
        }else if(colorCode.substr(0, 3) == "rgb") {
            // rgb case
            size_t start = colorCode.find('(');
            size_t end = colorCode.find(')');
            if(start == std::string::npos || end == std::string::npos || end <= start) {
                throw std::invalid_argument("Invalid rgb color code");
            }
            std::string values = colorCode.substr(start + 1, end - start - 1);
            size_t comma1 = values.find(',');
            size_t comma2 = values.find(',', comma1 + 1);
            if(comma1 == std::string::npos || comma2 == std::string::npos) {
                throw std::invalid_argument("Invalid rgb color code");
            }
            uint8_t r = std::stoi(values.substr(0, comma1));
            uint8_t g = std::stoi(values.substr(comma1 + 1, comma2 - comma1 - 1));
            uint8_t b = std::stoi(values.substr(comma2 + 1));
            return {r, g, b};
        } else {
            throw std::invalid_argument("Unsupported color code format");
        }
        return {0, 0, 0}; // Default return to avoid compiler warning
    }

    rgb interpolateColor(rgb start, rgb end, double t) {
        return {
            static_cast<uint8_t>(start.r + (end.r - start.r) * t),
            static_cast<uint8_t>(start.g + (end.g - start.g) * t),
            static_cast<uint8_t>(start.b + (end.b - start.b) * t)
        };
    }

public:
    int initialize(int row, int col, int h, int w) {
        struct winsize wsize;
        if(ioctl(STDOUT_FILENO, TIOCGWINSZ, &wsize) == -1) {
            throw std::runtime_error("Failed to get terminal size");
        }
        if(row < 0 || col < 0 || h <= 0 || w <= 0 || row + h > wsize.ws_row || col + w > wsize.ws_col) {
            throw std::invalid_argument("Invalid box dimensions or position");
        }
        originRow = row;
        originCol = col+1; // due to issue where col=0 was not behaving accordingly, so added +1 to col to fix it (if any other issue occur then i re-work on it again)
        height = h;
        width = w;
        isInitialized = true;
        return 0;
    }

public:
    int setTextColor(uint8_t r, uint8_t g, uint8_t b) {
        textColor = {r, g, b};
        return 0;
    }
    int setTextColor(std::string colorCode) {
        setTextColor(convertColorCodesToRGB(colorCode).r, convertColorCodesToRGB(colorCode).g, convertColorCodesToRGB(colorCode).b);
        return 0;
    }

    int setGlowColor(uint8_t r, uint8_t g, uint8_t b) {
        glowColor = {r, g, b};
        return 0;
    }
    int setGlowColor(std::string colorCode) {
        setGlowColor(convertColorCodesToRGB(colorCode).r, convertColorCodesToRGB(colorCode).g, convertColorCodesToRGB(colorCode).b);
        return 0;
    }

public:
    int setSpeed(double s) {
        if(s < 0) {
            throw std::invalid_argument("Speed must be non-negative");
        }
        speed = s;
        return 0;
    }

    int setGlowLength(int l) {
        if(l < 0) {
            throw std::invalid_argument("Glow length must be non-negative");
        }
        glowLength = l;
        return 0;
    }

public:
    int drawText(std::string text) {
        if (!isInitialized)
            throw std::runtime_error("TextGlow is not initialized");

        if (height <= 0 || width <= 0 || glowLength < 0)
            return 0;

        std::cout << "\033[?25l" << std::flush;

        std::string buffer;
        int row = originRow;
        int col = originCol;

        const int totalCells = width * height;

        auto drawChar = [&](int r, int c, char ch, rgb color) {
            if (r < originRow || r >= originRow + height ||
                c < originCol || c >= originCol + width)
                return;

            std::cout << "\033[" << r << ";" << c << "H"
                    << "\033[38;2;"
                    << static_cast<int>(color.r) << ";"
                    << static_cast<int>(color.g) << ";"
                    << static_cast<int>(color.b) << "m"
                    << ch << "\033[0m";
        };

        auto getPosition = [&](int index, int &r, int &c) {
            index %= totalCells;

            r = originRow + index / width;
            c = originCol + index % width;
        };

        auto advanceCursor = [&]() {
            int index = (row - originRow) * width + (col - originCol);
            index = (index + 1) % totalCells;

            row = originRow + index / width;
            col = originCol + index % width;
        };

        for (char ch : text) {
            if (ch == '\n') {
                buffer.clear();

                int currentIndex =
                    (row - originRow) * width + (col - originCol);

                currentIndex =
                    ((currentIndex / width) + 1) * width;

                currentIndex %= totalCells;

                row = originRow + currentIndex / width;
                col = originCol + currentIndex % width;

                continue;
            }

            if (glowLength == 0) {
                drawChar(row, col, ch, textColor);
                std::cout << std::flush;

                advanceCursor();

                usleep(static_cast<useconds_t>(speed * 1000000.0));
                continue;
            }

            if (static_cast<int>(buffer.size()) >= glowLength)
                buffer.erase(0, 1);

            buffer.push_back(ch);

            int currentIndex =
                (row - originRow) * width + (col - originCol);

            int activeCount = static_cast<int>(buffer.size());

            for (int j = 0; j < activeCount; j++) {
                int trailIndex =
                    (currentIndex - activeCount + 1 + j + totalCells)
                    % totalCells;

                int drawRow;
                int drawCol;

                getPosition(trailIndex, drawRow, drawCol);

                double t = (activeCount == 1)
                            ? 0.0
                            : static_cast<double>(j) / (activeCount - 1);

                rgb color = interpolateColor(textColor, glowColor, t);

                drawChar(drawRow, drawCol, buffer[j], color);
            }

            std::cout << std::flush;

            advanceCursor();

            usleep(static_cast<useconds_t>(speed * 1000000.0));
        }

        // Remaining glow characters ko text color mein finalize karo
        int currentIndex =
            (row - originRow) * width + (col - originCol);

        for (int j = 0; j < static_cast<int>(buffer.size()); j++) {
            int finalIndex =
                (currentIndex - static_cast<int>(buffer.size()) + j
                + totalCells) % totalCells;

            int drawRow;
            int drawCol;

            getPosition(finalIndex, drawRow, drawCol);

            drawChar(drawRow, drawCol, buffer[j], textColor);
        }

        std::cout << "\033[0m\033[?25h" << std::flush;

        return 0;
    }

    int drawTextShimmer(std::string text) {
        if (!isInitialized)
            throw std::runtime_error("TextGlow is not initialized");

        if (height <= 0 || width <= 0 || glowLength < 0)
            return 0;

        std::cout << "\033[?25l" << std::flush;

        std::string buffer;
        int row = originRow;
        int col = originCol;

        const int totalCells = width * height;

        double shimmerPhase = 0.0;

        auto drawChar = [&](int r, int c, char ch, rgb color) {
            if (r < originRow || r >= originRow + height ||
                c < originCol || c >= originCol + width)
                return;

            std::cout << "\033[" << r << ";" << c << "H"
                    << "\033[38;2;"
                    << static_cast<int>(color.r) << ";"
                    << static_cast<int>(color.g) << ";"
                    << static_cast<int>(color.b) << "m"
                    << ch << "\033[0m";
        };

        auto getPosition = [&](int index, int &r, int &c) {
            index %= totalCells;

            if (index < 0)
                index += totalCells;

            r = originRow + index / width;
            c = originCol + index % width;
        };

        auto advanceCursor = [&]() {
            int index =
                (row - originRow) * width +
                (col - originCol);

            index = (index + 1) % totalCells;

            row = originRow + index / width;
            col = originCol + index % width;
        };

        auto makeDarkColor = [](rgb color) -> rgb {
            return {
                static_cast<uint8_t>(color.r * 0.25),
                static_cast<uint8_t>(color.g * 0.25),
                static_cast<uint8_t>(color.b * 0.25)
            };
        };

        auto makeBrightColor = [](rgb color) -> rgb {
            return {
                static_cast<uint8_t>(
                    color.r + (255 - color.r) * 0.75
                ),
                static_cast<uint8_t>(
                    color.g + (255 - color.g) * 0.75
                ),
                static_cast<uint8_t>(
                    color.b + (255 - color.b) * 0.75
                )
            };
        };

        auto interpolate = [](rgb a, rgb b, double t) -> rgb {
            return {
                static_cast<uint8_t>(a.r + (b.r - a.r) * t),
                static_cast<uint8_t>(a.g + (b.g - a.g) * t),
                static_cast<uint8_t>(a.b + (b.b - a.b) * t)
            };
        };

        // Only glowColor gets two brightness boundaries.
        const rgb darkestGlow = makeDarkColor(glowColor);
        const rgb brightestGlow = makeBrightColor(glowColor);

        for (char ch : text) {
            if (ch == '\n') {
                buffer.clear();

                int currentIndex =
                    (row - originRow) * width +
                    (col - originCol);

                currentIndex =
                    ((currentIndex / width) + 1) * width;

                currentIndex %= totalCells;

                row = originRow + currentIndex / width;
                col = originCol + currentIndex % width;

                continue;
            }

            if (glowLength == 0) {
                drawChar(row, col, ch, textColor);
                std::cout << std::flush;

                advanceCursor();

                usleep(static_cast<useconds_t>(
                    speed * 1000000.0
                ));

                continue;
            }

            if (static_cast<int>(buffer.size()) >= glowLength)
                buffer.erase(0, 1);

            buffer.push_back(ch);

            int currentIndex =
                (row - originRow) * width +
                (col - originCol);

            int activeCount = static_cast<int>(buffer.size());

            for (int j = 0; j < activeCount; j++) {
                int trailIndex =
                    (currentIndex - activeCount + 1 + j + totalCells)
                    % totalCells;

                int drawRow;
                int drawCol;

                getPosition(trailIndex, drawRow, drawCol);

                double t = (activeCount == 1)
                            ? 0.0
                            : static_cast<double>(j) /
                            (activeCount - 1);

                // Shimmer moves from darkestGlow to brightestGlow
                // and then back, without changing textColor.
                double phase = shimmerPhase + t;
                phase -= static_cast<int>(phase);

                double wave;

                if (phase < 0.5)
                    wave = phase * 2.0;
                else
                    wave = 2.0 - phase * 2.0;

                rgb currentGlowColor =
                    interpolate(darkestGlow, brightestGlow, wave);

                // Existing textColor -> glowColor glow behavior.
                // textColor itself is never modified.
                rgb currentColor =
                    interpolate(textColor, currentGlowColor, t);

                drawChar(
                    drawRow,
                    drawCol,
                    buffer[j],
                    currentColor
                );
            }

            std::cout << std::flush;

            shimmerPhase += 0.08;

            if (shimmerPhase >= 1.0)
                shimmerPhase -= 1.0;

            advanceCursor();

            usleep(static_cast<useconds_t>(
                speed * 1000000.0
            ));
        }

        int currentIndex =
            (row - originRow) * width +
            (col - originCol);

        for (int j = 0; j < static_cast<int>(buffer.size()); j++) {
            int finalIndex =
                (currentIndex - static_cast<int>(buffer.size()) + j
                + totalCells) % totalCells;

            int drawRow;
            int drawCol;

            getPosition(finalIndex, drawRow, drawCol);

            drawChar(
                drawRow,
                drawCol,
                buffer[j],
                textColor
            );
        }

        std::cout << "\033[0m\033[?25h" << std::flush;

        return 0;
    }

};