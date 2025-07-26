#include <openssl/evp.h>
#include <openssl/sha.h>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

std::string sha256_hash(const std::string& input)
{
  unsigned char hash[SHA256_DIGEST_LENGTH];
  SHA256_CTX    sha256;

  SHA256_Init(&sha256);
  SHA256_Update(&sha256, input.c_str(), input.length());
  SHA256_Final(hash, &sha256);

  std::stringstream ss;
  for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
  {
    ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
  }
  return ss.str();
}

int main(int argc, char* argv[])
{
  if (argc != 2)
  {
    std::cerr << "Usage: " << argv[0] << " <string_to_hash>" << std::endl;
    return 1;
  }

  std::string input = argv[1];
  std::string hash  = sha256_hash(input);

  std::cout << "SHA256 hash of '" << input << "':" << std::endl;
  std::cout << hash << std::endl;

  return 0;
}