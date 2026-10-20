#include "options.hpp"

#include <charconv>
#include <format>
#include <system_error>

namespace pwgen {

namespace {

std::size_t parseNumber(std::string_view flag, std::string_view text) {
    std::size_t value = 0;
    const auto* const end = text.data() + text.size();
    const auto [parsedUntil, errorCode] = std::from_chars(text.data(), end, value);
    if (text.empty() || errorCode != std::errc{} || parsedUntil != end) {
        throw Error(std::format("invalid number for {}: '{}'", flag, text));
    }
    return value;
}

class ArgumentCursor {
  public:
    explicit ArgumentCursor(std::span<const std::string_view> args) : args_(args) {}

    [[nodiscard]] bool done() const noexcept { return position_ >= args_.size(); }

    std::string_view next() noexcept { return args_[position_++]; }

    std::string_view valueFor(std::string_view flag) {
        if (done()) {
            throw Error(std::format("missing value for {}", flag));
        }
        return next();
    }

  private:
    std::span<const std::string_view> args_;
    std::size_t position_ = 0;
};

}

Options parseOptions(std::span<const std::string_view> args) {
    Options options;
    ArgumentCursor cursor(args);

    while (!cursor.done()) {
        const std::string_view flag = cursor.next();

        if (flag == "-h" || flag == "--help") {
            options.help = true;
        } else if (flag == "-l" || flag == "--length") {
            options.length = parseNumber(flag, cursor.valueFor(flag));
            if (options.length < minLength || options.length > maxLength) {
                throw Error(std::format("length must be between {} and {}", minLength, maxLength));
            }
        } else if (flag == "-c" || flag == "--count") {
            options.count = parseNumber(flag, cursor.valueFor(flag));
            if (options.count == 0) {
                throw Error("count must be at least 1");
            }
        } else if (flag == "--no-lower") {
            options.lower = false;
        } else if (flag == "--no-upper") {
            options.upper = false;
        } else if (flag == "--no-digits") {
            options.digits = false;
        } else if (flag == "--symbols") {
            options.symbols = true;
        } else if (flag == "--exclude-ambiguous") {
            options.excludeAmbiguous = true;
        } else if (flag == "--exclude") {
            options.exclude = cursor.valueFor(flag);
        } else if (flag == "--require-each") {
            options.requireEach = true;
        } else if (flag == "--entropy") {
            options.showEntropy = true;
        } else {
            throw Error(std::format("unknown option '{}'", flag));
        }
    }

    return options;
}

std::string_view usage() noexcept {
    return R"(Usage: pwgen [options]

Options:
  -l, --length N        password length (default 20, range 1-1024)
  -c, --count N         number of passwords (default 1)
      --no-lower        exclude lowercase letters
      --no-upper        exclude uppercase letters
      --no-digits       exclude digits
      --symbols         include symbols
      --exclude-ambiguous
                        exclude 0 O o 1 l I
      --exclude CHARS   exclude the given characters
      --require-each    use at least one character from every enabled set
      --entropy         print entropy in bits and a strength label
  -h, --help            show this help
)";
}

}
