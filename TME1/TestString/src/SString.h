#include <memory>
#include <iostream>
#include "strutil.h"

namespace pr {

class SString {
private:
    std::shared_ptr<const char[]> data;
    
public:
    //construc 
    SString(const char* s = "") : data(newcopy(s)) {}
    
    //destruct apparement shared_ptr fait tout le taf
    ~SString() = default;
    
    
    SString(const SString& other) = default;
    
    
    SString& operator=(const SString& other) = default;//ça c'est super pratique en vrai
    
    
    SString(SString&& other) noexcept = default;
    SString& operator=(SString&& other) noexcept = default;
    
    //deux trois operateurs
    bool operator<(const SString& other) const {
        if (!data && !other.data) return false;
        if (!data) return true;
        if (!other.data) return false;
        return compare(data.get(), other.data.get()) < 0;
    }
    
    //les amis
    friend std::ostream& operator<<(std::ostream& os, const SString& str);
    friend bool operator==(const SString& a, const SString& b);
    friend SString operator+(const SString& a, const SString& b);
    
};
//j'implemente tout ici parce que j'ai la flemme de faire autre .cpp pour une question
inline std::ostream& operator<<(std::ostream& os, const SString& str) {
    os << (str.data ? str.data.get() : "(null)");
    return os;
}

inline bool operator==(const SString& a, const SString& b) {
    if (!a.data && !b.data) return true;
    if (!a.data || !b.data) return false;
    return compare(a.data.get(), b.data.get()) == 0;
}

inline SString operator+(const SString& a, const SString& b) {
    const char* str_a = a.data ? a.data.get() : "";
    const char* str_b = b.data ? b.data.get() : "";
    char* result = newcat(str_a, str_b);
    SString temp(result);
    delete[] result;
    return temp;
}

} // namespace pr