#include "String.h"

namespace pr
{

// TODO: Implement constructor e.g. using initialization list
String::String (const char *s)
{
  std::cout << "String constructor called for: " << s << std::endl;
  data = newcopy(s);
}

String::~String ()
{
  std::cout << "String destructor called for: " << (data ? data : "(null)")
      << std::endl;
  // TODO
  delete[] data;

}

String::String(const String& other){
    std::cout << "String copy constructor called for: " << other.data << std::endl;
    data = newcopy(other.data);
}

String& String::operator=(const String& other){
  std::cout << "String copy assignment called for: " << other.data << std::endl;
  if (this == &other) return *this; //pour voir si c'est pas lui meme
  delete [] this->data;
  this->data = newcopy(other.data);
  return *this;
} // Copy assign

String::String(String&& other) noexcept{
  std::cout << "String move constructor called for: " << other.data << std::endl;
  this->data = other.data;
  other.data = nullptr;
}

String& String::operator=(String&& other) noexcept{
  std::cout << "String move assignment called for: " << other.data << std::endl;
  if (this != &other) {
    delete[] data;
    data = other.data;
    other.data = nullptr;
  }
  return *this;
}

bool String::operator<(const String& other) const{
  std::cout << "String less-than comparison called for: " << data << " et " << other.data << std::endl;
  for (size_t i = 0; ; ++i) {
    if (data[i] < other.data[i]) return true;
    if (data[i] > other.data[i]) return false;
    if (data[i] == '\0' && other.data[i] == '\0') return false;
    if (data[i] == '\0') return true;
    if (other.data[i] == '\0') return false;
  }
}

String operator+(const String& a, const String& b){
  std::cout << "String addition called for: " << a.data << " et " << b.data << std::endl;
  size_t longueur_a, longueur_b, i, j;
  longueur_a = length(a.data);
  longueur_b = length(b.data);
  char* copie = new char[longueur_a + longueur_b + 1];
  for (i = 0; i < longueur_a; ++i) {
    copie[i] = a.data[i];
  }
  for (j = 0; j < longueur_b; ++j) {
    copie[longueur_a + j] = b.data[j];
  }
  copie[longueur_a + longueur_b] = '\0';

  String result(copie);
  delete[] copie;
  return result;
}

std::ostream& operator<<(std::ostream& os, const String& str) {
  os << (str.data ? str.data : "(null)");
  std::cout << "String output stream called for: " << str.data << std::endl;//au cas ou ça bug je met apres la verification
  return os;
}
    bool operator==(const String& a, const String& b){
      if(a.data == nullptr and b.data == nullptr){
        return true;
      }else if (a.data == nullptr || b.data == nullptr) {
        return false;
      }else if(compare(a.data,b.data) == 0){
        return true;
      }else{
        return false;
      }
    } // Symmetric equality
}// namespace pr

