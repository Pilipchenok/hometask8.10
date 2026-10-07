#include <windows.h>
#include <iostream>
#include <cstring>
#include <cstdint>

char* read_input(size_t& outLen);
bool send_all(HANDLE hPipe, const char* buffer, size_t bytesToWrite);

int main()
{
  std::cout << "Введите строку: ";
  size_t len = 0;
  char* msg = read_input(len);

  SECURITY_ATTRIBUTES saAttr;
  saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
  saAttr.bInheritHandle = TRUE;
  saAttr.lpSecurityDescriptor = nullptr;

  HANDLE hReadPipe = NULL;
  HANDLE hWritePipe = NULL;

  if (!CreatePipe(&hReadPipe, &hWritePipe, &saAttr, 0))
  {
    std::cerr << "Parent: CreatePipe error " << GetLastError() << "\n";
    delete[] msg;
    return 1;
  }

  SetHandleInformation(hWritePipe, HANDLE_FLAG_INHERIT, 0);

  char cmdLine[256];
  snprintf(cmdLine, sizeof(cmdLine), "child.exe %llu", 
    static_cast<unsigned long long>(reinterpret_cast<uintptr_t>(hReadPipe)));

  STARTUPINFOA si = {};
  si.cb = sizeof(si);
  PROCESS_INFORMATION pi = {};

  BOOL success = CreateProcessA(
    nullptr,
    cmdLine,
    nullptr,
    nullptr,
    TRUE,
    0,
    nullptr,
    nullptr,
    &si,
    &pi
  );

  if (!success)
  {
    std::cerr << "Parent: CreateProcess error " << GetLastError() << "\n";
    CloseHandle(hReadPipe);
    CloseHandle(hWritePipe);
    delete[] msg;
    return 1;
  }

  CloseHandle(hReadPipe);

  bool send_ok = true;

  if (!send_all(hWritePipe, reinterpret_cast<const char*>(&len), sizeof(len)))
  {
    std::cerr << "Parent: send length error " << GetLastError() << "\n";
    send_ok = false;
  }

  if (send_ok && len > 0)
  {
    if (!send_all(hWritePipe, msg, len))
    {
      std::cerr << "Parent: send data error " << GetLastError() << "\n";
      send_ok = false;
    }
  }

  delete[] msg;

  CloseHandle(hWritePipe);
  WaitForSingleObject(pi.hProcess, INFINITE);

  DWORD exitCode = 0;
  GetExitCodeProcess(pi.hProcess, &exitCode);

  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);

  return (send_ok && exitCode == 0) ? 0 : 1;
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
