#include <cassert>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <algorithm>
#include <vector>
#include <span>
#include <expected>

/* mygrep ****************************************/
     constexpr std::string_view VERSION = "0.0";
/*                                               */
/*   made by Azkey                               */
/*************************************************/

/* ToDo ********************************
    + Support stdin
    + Support RegEx
    + Support case-ignoreing
    + Add help
    + Deal with unavailable file input
    + Split function (in main())
****************************************/


// Specify the type of error.
enum class ErrorType {
    Unexpected = 1,
    InvalidArgs = 2,
    IOError = 3,
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
std::expected<OptionFlag, ErrorType>
parseLongOption(std::string_view sv_option);
std::expected<OptionFlag, ErrorType>
parseShortOption(std::string_view sv_option);
std::expected<OptionFlag, ErrorType>
parseOption(const std::vector<std::string>& optv);
constexpr bool hasFlag(OptionFlag value, OptionFlag flag);

struct Option {
    char opShort;
    std::string opLong;
    bool opDefault = false;
    OptionFlag opFlag;
    std::string description;
};

Option showIndex = {
    .opShort{'n'},
    .opLong{"line-number"},
    .opFlag{OptionFlag::ShowIndex},
    .description{"print line number with output lines"}
};

Option invert = {
    .opShort{'v'},
    .opLong{"invert-match"},
    .opFlag{OptionFlag::Invert},
    .description{"select non-matching lines"}
};

Option ignoreCase = {
    .opShort{'i'},
    .opLong{"ignore-case"},
    .opFlag{OptionFlag::IgnoreCase},
    .description{"ignore case distinctions in patterns and data"}
};

Option useRegEx = {
    .opShort{'E'},
    .opLong{"extended-regexp"},
    .opFlag{OptionFlag::UseRegEx},
    .description{"PATTERNS are extended regular expressions"}
};

Option printHitNum = {
    .opShort{'c'},
    .opLong{"count"},
    .opFlag{OptionFlag::PrintHitNum},
    .description{"print only a count of selected lines per FILE"}
};

Option printHelp = {
    .opLong{"help"},
    .opFlag{OptionFlag::PrintHelp},
    .description{"display this help text and exit"}
};

Option printVersion = {
    .opShort{'V'},
    .opLong{"version"},
    .opFlag{OptionFlag::PrintVersion},
    .description{"display version information and exit"}
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
    printVersion
};

void callHelp();
void callVersion();


int main( int argc, char* argv[] ) {

    Argument args( argc, argv );

    std::expected<OptionFlag, ErrorType> op = parseOption(args.getOptions());
    if (!op){
        return static_cast<int>(op.error());
    }

    if ( hasFlag(*op, OptionFlag::PrintHelp) ) {
        callHelp();
        return 0;
    }

    if ( hasFlag(*op, OptionFlag::PrintVersion) ) {
        callVersion();
        return 0;
    }

    std::vector<std::string> operands = args.getOperands();
    std::string_view pattern = operands[0];
    std::span<const std::string> files = std::span{operands}.subspan(1);

    bool existMultipleFiles = (files.size() > 1);

    if ( hasFlag(*op, OptionFlag::PrintHitNum) ){
        for (const std::string& file : files) {
            std::ifstream ifs(file);
            std::string line;
            int hitCounter = 0;

            while ( std::getline(ifs, line) ) {
                if ( hasFlag(*op, OptionFlag::Invert) ^ line.contains(pattern) ) {
                    hitCounter++;
                }
            }

            if (existMultipleFiles) {
                std::cout << file << ": ";
            }
            std::cout << hitCounter << "\n";
        }

        return 0;
    }

    for (const std::string& file : files) {
        std::ifstream ifs(file);
        std::string line;
        std::size_t lineIdx = std::size_t{0};
        
        while ( std::getline(ifs, line) ) {
            lineIdx++;

            if ( hasFlag(*op, OptionFlag::Invert) ^ !line.contains(pattern) ) {
                continue;
            }

            if (existMultipleFiles) {
                std::cout << file << ": ";
            }

            if ( hasFlag(*op, OptionFlag::ShowIndex) ) {
                std::cout << std::setw(4) << lineIdx << "| ";
            }

            std::cout << line << "\n";

        }
    }

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

std::expected<OptionFlag, ErrorType>
parseOption( const std::vector<std::string>& optv ) {
    OptionFlag opRes = OptionFlag::None;

    for (std::string_view opt : optv) {
        if (opt == "-") {
            std::cerr << "Undefined option: " << opt << "\n";
            return std::unexpected{ErrorType::InvalidArgs};
        }
        
        /* parse long option */
        if (opt.starts_with("--")) {
            std::expected<OptionFlag, ErrorType>
            parseResult = parseLongOption(opt);
            if ( !parseResult ){
                return std::unexpected{parseResult.error()};
            } else {
                opRes |= *parseResult;
            }
        }

        /* parse short option */
        else if (opt.starts_with('-')) {
            std::expected<OptionFlag, ErrorType>
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

std::expected<OptionFlag, ErrorType>
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
        return std::unexpected{ErrorType::InvalidArgs};
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
        return std::unexpected{ErrorType::InvalidArgs};
    }

}

std::expected<OptionFlag, ErrorType>
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
            return std::unexpected{ErrorType::InvalidArgs};
        }
    }

    return opRes;

}

void callHelp() {

};

void callVersion() {

};