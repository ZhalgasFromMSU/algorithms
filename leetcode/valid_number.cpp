#include <iostream>
#include <vector>
#include <string>
#include <sstream>


bool valid_number(const std::string& s) {
    uint8_t exp_count(0), sign_count(0), dot_count(0), conn_comp(0);
    auto it = s.begin();
    while (it != s.end() && isspace(*it)) {
        ++it;
    }
    if (it == s.end()) {
        return false;
    } else if (*it == '+' || *it == '-') {
        ++sign_count;
    } else if (isdigit(*it)){
        ++conn_comp;
    } else if (*it == '.') {
        ++dot_count;
    } else {
        return false;
    }
    ++it;
    for (; it != s.end(); ++it) {
        if (isspace(*it)) {
            break;
        } else if (*it == 'e' && exp_count == 0 && conn_comp != 0) {
            ++exp_count;
        } else if (*it == '.' && exp_count == 0) {
            ++dot_count;
        } else if (!isdigit(*it)) {
            return false;
        }
    }
    while (it != s.end() && isspace(*it)) {
        ++it;
    }
    if (it != s.end()) {
        return false;
    }
    return true;
}

int main() {
    valid_number("-1e-1asd");
    return 0;
}
