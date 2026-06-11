#include <cassert>
#include <iostream>
#include <fstream>
#include <format>
#include <string>
#include <algorithm>
#include <vector>
#include <span>
#include <optional>
#include <expected>
#include <variant>
#include <regex>

/* mygrep ****************************************/
     constexpr std::string_view VERSION = "3.0";
/*                                               */
/*   made by Azkey                               */
/*************************************************/

/* ToDo ********************************
    ? Target structure can be replaced
      with std::optional
****************************************/


// Specify the type of error.
enum class ReturnType {
    Normal,
    Unexpected,
    InvalidArgs,
    IOError,
};

class Argument {
    private:
        std::vector<std::string> operands;
        std::vector<std::string> options;
    public:
        Argument( int argc, char* argv[] );
        const std::vector<std::string>& getOperands() const;
        const std::vector<std::string>& getOptions() const;
};

enum class OptionFlag {
    None         = 0,
    ShowIndex    = 1 << 0,
    Invert       = 1 << 1,
    IgnoreCase   = 1 << 2,
    UseRegEx     = 1 << 3,
    PrintHitNum  = 1 << 4,
    PrintHelp    = 1 << 5,
    PrintVersion = 1 << 6
};
OptionFlag& operator|=(OptionFlag& L, OptionFlag R);
std::expected<OptionFlag, ReturnType>
parseLongOption(std::string_view sv_option);
std::expected<OptionFlag, ReturnType>
parseShortOption(std::string_view sv_option);
std::expected<OptionFlag, ReturnType>
parseOption(const std::vector<std::string>& optv);
constexpr bool hasFlag(OptionFlag value, OptionFlag flag);

struct Option {
    std::optional<char> opShort;
    std::string_view opLong;
    OptionFlag opFlag;
    std::string_view description;
};

std::vector<Option> definedOptions = {
    Option{
        .opShort{'n'},
        .opLong{"line-number"},
        .opFlag{OptionFlag::ShowIndex},
        .description{"print line number with output lines"}
    },
    Option{
        .opShort{'v'},
        .opLong{"invert-match"},
        .opFlag{OptionFlag::Invert},
        .description{"select non-matching lines"}
    },
    Option{
        .opShort{'i'},
        .opLong{"ignore-case"},
        .opFlag{OptionFlag::IgnoreCase},
        .description{"ignore case distinctions in patterns and data"}
    },
    Option{
        .opShort{'E'},
        .opLong{"extended-regexp"},
        .opFlag{OptionFlag::UseRegEx},
        .description{"PATTERNS are extended regular expressions"}
    },
    Option{
        .opShort{'c'},
        .opLong{"count"},
        .opFlag{OptionFlag::PrintHitNum},
        .description{"print only a count of selected lines per FILE"}
    },
    Option{
        .opShort{'V'},
        .opLong{"version"},
        .opFlag{OptionFlag::PrintVersion},
        .description{"display version information and exit"}
    },
    Option{
        .opLong{"help"},
        .opFlag{OptionFlag::PrintHelp},
        .description{"display this help text and exit"}
    }
};

struct Stdin {};
struct File { std::string name; };

using Target = std::variant<Stdin, File>;

struct ScanVisitor {
    const std::string& pattern;
    OptionFlag op;
    bool doPrintName;

    std::expected<void, ReturnType> operator()( const Stdin& s );
    std::expected<void, ReturnType> operator()( const File&  f );
};

struct Regex { std::regex key; };
struct Plain { std::string key; };

using Key = std::variant<Regex, Plain>;

struct MatchVisitor {
    const std::string& input;
    OptionFlag op;
    bool operator()( const Regex& re );
    bool operator()( const Plain& pl );
};

void scanStream(
    std::optional<std::string_view> fileName,
    std::istream& is,
    const std::string& pattern,
    OptionFlag op
);

void printResult(
    std::optional<std::string_view> fileName,
    std::size_t lineIndex,
    std::string_view line,
    OptionFlag op
);

void printResult(
    std::optional<std::string_view> fileName,
    std::size_t counter
);

void callHelp();
void callVersion();


int main( int argc, char* argv[] ) {

    ReturnType rt = ReturnType::Normal;

    Argument args( argc, argv );

    // reject unknown option
    std::expected<OptionFlag, ReturnType> op = parseOption(args.getOptions());
    if (!op){
        return static_cast<int>(op.error());
    }

    if ( hasFlag(*op, OptionFlag::PrintHelp) ) {
        callHelp();
        return static_cast<int>(rt);
    }

    if ( hasFlag(*op, OptionFlag::PrintVersion) ) {
        callVersion();
        return static_cast<int>(rt);
    }

    std::vector<std::string> operands = args.getOperands();
    if (operands.empty()) {
        std::cout << "Pattern is required.\n";
        return static_cast<int>(ReturnType::InvalidArgs);
    }
    const std::string& pattern = operands[0];
    std::vector<Target> targets;

    if (operands.size() == 1) {
        targets.emplace_back(Stdin{});
    } else {
        auto filenames = std::span{operands}.subspan(1);
        for ( const std::string& filename : filenames ) {
            targets.emplace_back(File{filename});
        }
    }

    bool requireName = targets.size() > 1;

    for ( Target& target : targets ) {
        std::expected<void, ReturnType> result = std::visit(
            ScanVisitor{ pattern, *op, requireName }, target
        );
        if (!result) {
            rt = result.error();
        }

    }

    return static_cast<int>(rt);

}

Argument::Argument( int argc, char* argv[] ) {
    bool oprFlag = false;
    for ( int i = 1; i < argc; i++ ) {
        std::string_view arg = argv[i];

        if ( oprFlag ) {
            operands.emplace_back(arg);
            continue;
        }

        if ( arg == "--" ) {
            oprFlag = true;
            continue;
        }

        if (arg.starts_with('-')) {
            options.emplace_back(arg);
        } else {
            operands.emplace_back(arg);
        }
    }
}

const std::vector<std::string>& Argument::getOperands() const {
    return operands;
}

const std::vector<std::string>& Argument::getOptions() const {
    return options;
}

OptionFlag& operator|=(OptionFlag& L, OptionFlag R) {
    using UT = std::underlying_type_t<OptionFlag>;

    L = static_cast<OptionFlag>(
        static_cast<UT>(L) | static_cast<UT>(R)
    );
    return L;
}

constexpr bool hasFlag(OptionFlag value, OptionFlag flag) {
    using UT = std::underlying_type_t<OptionFlag>;
    return (static_cast<UT>(value) & static_cast<UT>(flag)) != 0;
}

std::expected<OptionFlag, ReturnType>
parseOption( const std::vector<std::string>& optv ) {
    OptionFlag opRes = OptionFlag::None;

    for (std::string_view opt : optv) {
        if (opt == "-") {
            std::cerr << "Undefined option: " << opt << "\n";
            return std::unexpected{ReturnType::InvalidArgs};
        }
        
        /* parse long option */
        if (opt.starts_with("--")) {
            std::expected<OptionFlag, ReturnType>
            parseResult = parseLongOption(opt);
            if ( !parseResult ){
                return std::unexpected{parseResult.error()};
            } else {
                opRes |= *parseResult;
            }
        }

        /* parse short option */
        else if (opt.starts_with('-')) {
            std::expected<OptionFlag, ReturnType>
            parseResult = parseShortOption(opt);
            if ( !parseResult ){
                return std::unexpected{parseResult.error()};
            } else {
                opRes |= *parseResult;
            }
        }

    }
    return opRes;
}

std::expected<OptionFlag, ReturnType>
parseLongOption( std::string_view sv_option ){
    // Use with `if (sv_option.starts_with("--"))` state.
    assert( sv_option.starts_with("--") );

    sv_option.remove_prefix(2); // remove "--"

    // check each defined long option
    auto findIfRes = std::ranges::find_if(
        definedOptions,
        [&sv_option](const Option& o) {
            return o.opLong == sv_option;
        }
    );

    // when option found in defined long options,
    // return earlily with its option flag.
    if ( findIfRes != definedOptions.end() ) {
        return findIfRes->opFlag;
    }

    // when not found in long options,
    // search as short option (not combined).
    // therefore, reject options consisting of multi-chars.
    if (sv_option.size() != 1) {
        std::cerr << "Undefined option: --" << sv_option << "\n";
        return std::unexpected{ReturnType::InvalidArgs};
    }

    findIfRes = std::ranges::find_if(
        definedOptions,
        [&sv_option](const Option& o) {
            return o.opShort == sv_option.front();
        }
    );

    if ( findIfRes != definedOptions.end() ) {
        return findIfRes->opFlag;
    } else {
        std::cerr << "Undefined option: --" << sv_option << "\n";
        return std::unexpected{ReturnType::InvalidArgs};
    }

}

std::expected<OptionFlag, ReturnType>
parseShortOption( std::string_view sv_option ){
    // Use with `if (sv_option.find_first_not_of('-') == 1)` state.
    assert(sv_option.find_first_not_of('-') == 1);

    OptionFlag opRes = OptionFlag::None;

    sv_option.remove_prefix(1); // remove "-"

    // check each defined short option by character
    for( char c : sv_option ) {
        auto findIfRes = std::ranges::find_if(
            definedOptions,
            [c](const Option& o) {
                return o.opShort == c;
            }
        );

        if ( findIfRes != definedOptions.end() ) {
            opRes |= (findIfRes->opFlag);
        } else {
            std::cerr << "Undefined option: " << sv_option << "\n";
            return std::unexpected{ReturnType::InvalidArgs};
        }
    }

    return opRes;

}

std::expected<void, ReturnType> ScanVisitor::operator()(
    const Stdin& s
)
{
    std::optional<std::string_view> tempName{std::nullopt};
    scanStream(tempName, std::cin, pattern, op);
    return {};
};

std::expected<void, ReturnType> ScanVisitor::operator()(
    const File& f
)
{
    std::ifstream ifs(f.name);
    if(!ifs.is_open()){
        std::cerr << "Invalid file name: " << f.name << "\n";
        return std::unexpected(ReturnType::IOError);
    }
    std::optional<std::string_view> thrownName;
    if ( doPrintName ) { thrownName = f.name; }
    scanStream(thrownName, ifs, pattern, op);
    return {};
};

bool MatchVisitor::operator()( const Regex& re ){
    namespace re_const = std::regex_constants;
    bool invert = hasFlag(op, OptionFlag::Invert);

    return std::regex_search(input, re.key) ^ invert;
}

bool MatchVisitor::operator()( const Plain& pl ){
    bool icase = hasFlag(op, OptionFlag::IgnoreCase);
    bool invert = hasFlag(op, OptionFlag::Invert);
    auto searchResult = std::ranges::search(
        input,
        pl.key,
        std::equal_to{},
        [icase](char c){
            unsigned char uc = static_cast<unsigned char>(c);
            return icase ? std::tolower(uc) : c;
        },
        [icase](char c){
            unsigned char uc = static_cast<unsigned char>(c);
            return icase ? std::tolower(uc) : c;
        }
    );

    return !searchResult.empty() ^ invert;
}

void scanStream(
    std::optional<std::string_view> fileName,
    std::istream& is,
    const std::string& pattern,
    OptionFlag op
)
{
    namespace re_const = std::regex_constants;
    std::size_t hitCounter = 0;
    std::size_t lineIndex = 0;
    std::string line;
    bool matched;

    Key key;
    if( hasFlag(op, OptionFlag::UseRegEx) ){
        auto rec = hasFlag(op, OptionFlag::IgnoreCase)
            ? re_const::extended | re_const::icase
            : re_const::extended;
        key = Regex{std::regex(pattern, rec)};
    } else {
        key = Plain{pattern};
    }

    while( std::getline(is, line) ) {
        
        lineIndex++;

        // process regex and invert together
        matched = std::visit( MatchVisitor{ line, op }, key );
        
        if( !matched ){
            continue;
        }

        if( hasFlag(op, OptionFlag::PrintHitNum) ){
            hitCounter++;
            continue;
        }
        printResult(fileName, lineIndex, line, op);
        
    }

    if( hasFlag(op, OptionFlag::PrintHitNum) ){
        printResult(fileName, hitCounter);
    }

    return;
};

void printResult(
    std::optional<std::string_view> fileName,
    std::size_t lineIndex,
    std::string_view line,
    OptionFlag op
)
{
    if( fileName ) {
        std::cout << std::format("{}: ", *fileName);
    }
    if( hasFlag(op, OptionFlag::ShowIndex) ) {
        std::cout << std::format("{:>4}| ", lineIndex);
    }
    std::cout << line << "\n";
    return;
};

void printResult(
    std::optional<std::string_view> fileName,
    std::size_t counter
)
{
    std::string resultString;

    if( fileName ) {
        resultString += std::format("{}: ", *fileName);
    }
    resultString += std::to_string(counter);

    std::cout << resultString << "\n";
    return;
};

void callHelp() {
    std::cout
    << "Usage: mygrep [OPTION]... PATTERN [FILE]...\n"
    << "Search for PATTERN in each FILE.\n"
    << "Example: grep -i 'hello world' menu.h main.c\n"
    << "\n"
    << "OPTIONS:\n";
    for (const Option& defo : definedOptions){
        std::string_view shownShort =
            defo.opShort
                ? std::format("-{},", *defo.opShort)
                : "   ";
            
        std::cout << std::format(
            "  {} --{:<20}{}\n",
            shownShort,
            defo.opLong,
            defo.description
        );
    }
};

void callVersion() {
    std::cout << "mygrep " << VERSION << "\n";
};