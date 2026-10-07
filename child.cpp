#include <windows.h>
#include <iostream>
#include <string>
#include <cstdint>

bool recv_all(HANDLE hPipe, char* buffer, size_t bytesToRead);

int main(int argc, char** argv)
{

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
