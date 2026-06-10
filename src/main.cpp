#include <cassert>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <format>
#include <string>
#include <algorithm>
#include <vector>
#include <span>
#include <optional>
#include <expected>
#include <variant>
#include <concepts>
#include <type_traits>

/* mygrep ****************************************/
     constexpr std::string_view VERSION = "2.0";
/*                                               */
/*   made by Azkey                               */
/*************************************************/

/* ToDo ********************************
    + Support RegEx
    + Add help

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
    char opShort;
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
        .opLong{"help"},
        .opFlag{OptionFlag::PrintHelp},
        .description{"display this help text and exit"}
    },
    Option{
        .opShort{'V'},
        .opLong{"version"},
        .opFlag{OptionFlag::PrintVersion},
        .description{"display version information and exit"}
    }
};

struct Stdin {};
struct File { std::string name; };

using Target = std::variant<Stdin, File>;

struct ScanVisitor {
    std::string_view pattern;
    OptionFlag op;
    bool doPrintName;

    std::expected<void, ReturnType> operator()( Stdin s );
    std::expected<void, ReturnType> operator()( File  f );
};


void scanStream(
    std::optional<std::string_view> fileName,
    std::istream& is,
    std::string_view pattern,
    OptionFlag op
);

void printResult(
    std::optional<std::string_view> fileName,
    std::size_t lineIndex,
    std::string_view line
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
    std::string_view pattern = operands[0];
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
    Stdin s
)
{
    std::optional<std::string_view> tempName{std::nullopt};
    scanStream(tempName, std::cin, pattern, op);
    return {};
};

std::expected<void, ReturnType> ScanVisitor::operator()(
    File f
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

void scanStream(
    std::optional<std::string_view> fileName,
    std::istream& is,
    std::string_view pattern,
    OptionFlag op
)
{
    std::size_t hitCounter = 0;
    std::size_t lineIndex = 0;
    std::string line;
    bool matched;

    bool doShowIndex = hasFlag(op, OptionFlag::ShowIndex);
    bool doIgnoreCase = hasFlag(op, OptionFlag::IgnoreCase);

    while( std::getline(is, line) ) {
        if( doShowIndex ) {
            lineIndex++;
        }

        // in need, ignore case
        auto searchResult = std::ranges::search(
            line,
            pattern,
            std::equal_to{},
            [doIgnoreCase](char c){
                return doIgnoreCase ? std::tolower(c) : c;
            },
            [doIgnoreCase](char c){
                return doIgnoreCase ? std::tolower(c) : c;
            }
        );

        matched = !searchResult.empty() ^ hasFlag(op, OptionFlag::Invert);
        if( !matched ){
            continue;
        }

        if( hasFlag(op, OptionFlag::PrintHitNum) ){
            hitCounter++;
            continue;
        }
        printResult(fileName, lineIndex, line);
        
    }

    if( hasFlag(op, OptionFlag::PrintHitNum) ){
        printResult(fileName, hitCounter);
    }

    return;
};

void printResult(
    std::optional<std::string_view> fileName,
    std::size_t lineIndex,
    std::string_view line
)
{
    std::string resultString;

    if( fileName ) {
        resultString += std::format("{}: ", *fileName);
    }
    if( lineIndex > 0 ) {
        resultString += std::format("{:>4}| ", lineIndex);
    }
    resultString += line;

    std::cout << resultString << "\n";
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

};

void callVersion() {

};