#include <windows.h>
#include <iostream>
#include <string>
#include <cstdint>

bool recv_all(HANDLE hPipe, char* buffer, size_t bytesToRead);

int main(int argc, char** argv)
{
  if (argc != 2)
  {
    std::cerr << "Child: not enough arguments\n";
    return 1;
  }

  unsigned long long handleVal = 0;
  try
  {
    handleVal = std::stoull(argv[1]);
  }
  catch (...)
  {
    std::cerr << "Child: invalid argument\n";
    return 1;
  }

  HANDLE hReadPipe = reinterpret_cast<HANDLE>(static_cast<uintptr_t>(handleVal));

  size_t len = 0;
  if (!recv_all(hReadPipe, reinterpret_cast<char*>(&len), sizeof(len)))
  {
    std::cerr << "Child: could not read length, error: " << GetLastError() << "\n";
    CloseHandle(hReadPipe);
    return 1;
  }

  char* buffer = new char[len + 1];

  bool has_error = false;
  if (len > 0)
  {
    if (!recv_all(hReadPipe, buffer, len))
    {
      std::cerr << "Child: could not read data, error: " << GetLastError() << "\n";
      has_error = true;
    }
  }
  buffer[len] = '\0';

  CloseHandle(hReadPipe);

  if (!has_error)
  {
    std::cout << buffer << "\n";
  }

  delete[] buffer;
  return has_error ? 1 : 0;
}

bool recv_all(HANDLE hPipe, char* buffer, size_t bytesToRead)
{
  size_t totalRead = 0;
  while (totalRead < bytesToRead)
  {
    DWORD chunk = static_cast<DWORD>(
      (bytesToRead - totalRead > MAXDWORD) ? MAXDWORD : (bytesToRead - totalRead)
    );
    DWORD bytesReadNow = 0;
    if (!ReadFile(hPipe, buffer + totalRead, chunk, &bytesReadNow, nullptr) || bytesReadNow == 0)
    {
      return false;
    }
    totalRead += bytesReadNow;
  }
  return true;
}
