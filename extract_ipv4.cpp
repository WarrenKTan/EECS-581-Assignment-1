// extract_ipv4.cpp
//
// Reads lines of text from the user and extracts a single valid IPv4
// address (optionally followed by ":port") embedded anywhere in the line.
//
// All parsing/validation is done by hand -- no atoi/strtol/sscanf/regex/
// inet_* functions are used anywhere in this file.

//                                  GAI DISCLOSURE
// TOOL/VERSION:        The following code is written by Claude Sonnet 5 on 27/09/2026.
// PROMPT:              The prompt used to generate this code can be found in 'AI_Disclosure.txt'.
// MODIFICATIONS:       There were no modifications to functional lines.
//                      Some comments have been rewritten to increase human readability.
//                      
// VERIFICATION:        I, Warren Tan, have read through this code and understand each line's function.
// TESTING:             Tested inputs are stored in 'test_inputs.txt'. Test results are stored in 'test_output.txt'
//                      Tests resulted in intended behaviour.

#include <iostream>
#include <string>

// ---------------------------------------------------------------------
// Helper: is this character one that can ever be part of a candidate
// token (digits, '.', ':')? Anything else is "garbage" and is skipped.
// ---------------------------------------------------------------------
static bool isTokenChar(char c) {
    return (c >= '0' && c <= '9') || c == '.' || c == ':';
}

// ---------------------------------------------------------------------
// Helper: is this character a digit? (hand-rolled, but isdigit() is
// explicitly allowed too -- this just avoids relying on locale).
// ---------------------------------------------------------------------
static bool isDigitChar(char c) {
    return c >= '0' && c <= '9';
}

// ---------------------------------------------------------------------
// parseNumber
//   Reads a run of digit characters from s starting at pos, by hand
//   accumulating the value one digit at a time (no library conversion
//   functions are used).
//
//   Rules enforced:
//     - Must have at least 1 digit.
//     - Must have no more than maxDigits digits.
//     - If the number has more than one digit, it may not start with '0'
//       (leading zero only allowed when the whole value is exactly "0").
//     - Numeric value must not exceed maxValue.
//
//   On success, pos is advanced past the digits consumed and 'value'
//   holds the parsed number. On failure, returns false (pos may be left
//   at an indeterminate position because the whole candidate is going to
//   be rejected anyway).
// 
//   Comments written by 
// ---------------------------------------------------------------------
static bool parseNumber(const std::string& s, size_t& pos, int maxDigits, int maxValue, int& value) {
    // track the starting position
    size_t start = pos;

    // number of digits read
    int count = 0;

    // tracks the numerical value 
    int val = 0;

    // read all consecutive digits after the start position
    while (pos < s.size() && isDigitChar(s[pos])) {
        // there is another digit beyond the allowed length
        if (count == maxDigits) {
            return false;
        }

        // left shift the current val (in base 10)
        // find the current digit using ASCII subtraction, then add the digit to val
        val = val * 10 + (s[pos] - '0');
        
        // increment count and pos
        count++;
        pos++;
    }

    // no digits at all
    if (count == 0) {
        return false;
    }

    // disallowed leading zero
    // (still allows values of exactly zero)
    if (s[start] == '0' && count > 1) {
        return false;
    }

    // val is out of range
    if (val > maxValue) {
        return false;
    }

    // return the value of the digit 
    value = val;
    return true;
}

// ---------------------------------------------------------------------
// parseCandidate
//   Attempts to match the ENTIRE token string against:
//       octet.octet.octet.octet[:port]
//   where each octet is 1-3 digits, 0-255 (leading-zero rule applies),
//   and port (if present) is 1-5 digits, 0-65535 (same leading-zero rule).
//
//   The whole token must be consumed -- any leftover/unexpected
//   character (extra '.', extra ':', trailing junk, etc.) fails the
//   whole candidate. There is no attempt to salvage a valid substring
//   out of a token that doesn't fully match.
// ---------------------------------------------------------------------
static bool parseCandidate(const std::string& tok, unsigned long& outAddress, int& outPort) {
    size_t pos = 0;
    int octet[4];

    for (int i = 0; i < 4; i++) {
        int val;

        // attempt to parse an octet with...
        //    3 digits maximum
        //    255 value maximum
        if (!parseNumber(tok, pos, 3, 255, val)) {
            // no number was found meeting the requirements. terminate parse. 
            return false;
        }

        // store valid octet in octet array
        octet[i] = val;

        // only check/update if not after the fourth octet
        if (i < 3) {
            // invalid if...
            //    there are not four octets
            //    the following char after parsing is not a period
            if (pos >= tok.size() || tok[pos] != '.') {
                return false;
            }

            // consume the period
            pos++;
        }
    }

    // no port value
    int port = -1;

    // attempt to parse a port value
    if (pos == tok.size()) {
        // no port
        port = -1;
    } else if (tok[pos] == ':') {
        pos++; // consume ':'
        int p;

        // attempt to find port number with...
        //    5 digits maximum
        //    65535 value maximum
        if (!parseNumber(tok, pos, 5, 65535, p)) {
            // Colon present but port invalid -> reject the WHOLE match, address included.
            return false;
        }
        if (pos != tok.size()) {
            // Trailing junk after the port (e.g. a second colon).
            return false;
        }
        port = p;
    } else {
        // Trailing junk after the fourth octet (e.g. a fourth period).
        return false;
    }

    // return the address 
    outAddress = (static_cast<unsigned long>(octet[0]) << 24) |
                 (static_cast<unsigned long>(octet[1]) << 16) |
                 (static_cast<unsigned long>(octet[2]) << 8) |
                 (static_cast<unsigned long>(octet[3]));
    outPort = port;
    return true;
}

// ---------------------------------------------------------------------
// extractIPv4
//   Scans str for maximal runs of "token characters" (digits, '.', ':').
//   Every other character acts purely as a separator/garbage and is
//   skipped. Each run is tried, in left-to-right order, as a whole
//   candidate against parseCandidate(). The first run that fully
//   validates is returned as the answer.
// ---------------------------------------------------------------------
bool extractIPv4(const std::string& str, unsigned long& outAddress, int& outPort) {
    size_t n = str.size();
    size_t i = 0;

    // search entire string for possible IPv4 candidates, then validate them.
    while (i < n) {
        // the first token found is a IPv4 candidate
        if (isTokenChar(str[i])) {
            // find where the IPv4 candidate ends
            size_t start = i;
            while (i < n && isTokenChar(str[i])) {
                i++;
            }
            // 'token' is a IPv4 candidate
            std::string token = str.substr(start, i - start);

            // validate IPv4 candidate
            unsigned long addr;
            int port;
            if (parseCandidate(token, addr, port)) {
                // candidate is valid IPv4 address. Return address, port, and true.
                outAddress = addr;
                outPort = port;
                return true;
            }
            // Candidate failed validation in full -- move on and keep
            // scanning the rest of the line for another candidate.
        } else {
            i++;
        }
    }

    // no address/port was found. return default failure values
    outAddress = 0;
    outPort = -1;
    return false;
}

// ---------------------------------------------------------------------
// main
//   Repeatedly prompts for a line of input until the user types END
//   (case-sensitive), then prints "Program terminated." and exits.
// ---------------------------------------------------------------------
int main() {
    // holds the user input 
    std::string line;

    while (true) {
        std::cout << "Enter a line of text (or END to quit): ";
        
        // end of file condition. terminate program.
        if (!std::getline(std::cin, line)) {
            // EOF on input -- treat the same as END.
            std::cout << std::endl << "Program terminated." << std::endl;
            break;
        }

        // user inputs "END"
        if (line == "END") {
            std::cout << "Program terminated." << std::endl;
            break;
        }

        // attempt to find IPv4 address from user input
        unsigned long address;
        int port;
        bool found = extractIPv4(line, address, port);

        // address is found
        if (found) {
            // print formatted information as stated in the instructions
            int b1 = static_cast<int>((address >> 24) & 0xFF);
            int b2 = static_cast<int>((address >> 16) & 0xFF);
            int b3 = static_cast<int>((address >> 8) & 0xFF);
            int b4 = static_cast<int>(address & 0xFF);

            // IPv4 address
            std::cout << "Extracted IPv4 address: "
                      << b1 << "." << b2 << "." << b3 << "." << b4
                      << " (decimal value: " << address << ", port: ";

            // port number
            if (port == -1) {
                std::cout << "none";
            } else {
                std::cout << port;
            }

            std::cout << ")" << std::endl;
        } else {
            std::cout << "No valid IPv4 address found in input." << std::endl;
        }
    }

    return 0;
}
