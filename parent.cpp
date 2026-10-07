#include <windows.h>
#include <iostream>
#include <cstring>
#include <cstdint>

char* read_input(size_t& outLen);
bool send_all(HANDLE hPipe, const char* buffer, size_t bytesToWrite);

int main()
{

}

bool send_all(HANDLE hPipe, const char* buffer, size_t bytesToWrite)
{
  size_t totalWritten = 0;
  while (totalWritten < bytesToWrite)
  {
    DWORD chunk = static_cast<DWORD>(
      (bytesToWrite - totalWritten > MAXDWORD) ? MAXDWORD : (bytesToWrite - totalWritten)
    );
    DWORD written = 0;
    if (!WriteFile(hPipe, buffer + totalWritten, chunk, &written, nullptr) || written == 0)
    {
      return false;
    }
    totalWritten += written;
  }
  return true;
}

char* read_input(size_t& outLen)
{
  size_t capacity = 128;
  size_t length = 0;
  char* buffer = new char[capacity];

  int c = 0;
  while ((c = std::cin.get()) != EOF && c != '\n')
  {
    if (length + 1 >= capacity)
    {
      size_t new_capacity = capacity * 2;
      char* new_buffer = new char[new_capacity];

      std::memcpy(new_buffer, buffer, length);
      delete[] buffer;

      buffer = new_buffer;
      capacity = new_capacity;
    }
    buffer[length++] = static_cast<char>(c);
  }

  buffer[length] = '\0';
  outLen = length;
  return buffer;
}
