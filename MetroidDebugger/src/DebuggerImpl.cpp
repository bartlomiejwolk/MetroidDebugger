#include <iostream>
#include <tchar.h>
#include <Windows.h>
#include <string>
#include <map>
#include <Psapi.h>
#include "Defines.h"
#include "DebuggerImpl.h"
#include <memory>
#include <iosfwd>

#define BUFSIZE 512

// TODO Consider creating a namespace
// This function is optimized!
static std::wstring GetFileNameFromHandle(HANDLE hFile)
{
	BOOL bSuccess = FALSE;
	TCHAR pszFilename[MAX_PATH + 1];
	HANDLE hFileMap;

	std::wstring strFilename;

	// Get the file size.
	DWORD dwFileSizeHi = 0;
	DWORD dwFileSizeLo = GetFileSize(hFile, &dwFileSizeHi);

	if (dwFileSizeLo == 0 && dwFileSizeHi == 0)
	{
		return strFilename;
	}

	// Create a file mapping object.
	hFileMap = CreateFileMapping(hFile,
		NULL,
		PAGE_READONLY,
		0,
		1,
		NULL);

	if (hFileMap)
	{
		// Create a file mapping to get the file name.
		void* pMem = MapViewOfFile(hFileMap, FILE_MAP_READ, 0, 0, 1);

		if (pMem)
		{
			if (GetMappedFileName(GetCurrentProcess(),
				pMem,
				pszFilename,
				MAX_PATH))
			{

				// Translate path with device name to drive letters.
				TCHAR szTemp[BUFSIZE];
				szTemp[0] = '\0';

				if (GetLogicalDriveStrings(BUFSIZE - 1, szTemp))
				{
					TCHAR szName[MAX_PATH];
					TCHAR szDrive[3] = TEXT(" :");
					BOOL bFound = FALSE;
					TCHAR* p = szTemp;

					do
					{
						// Copy the drive letter to the template string
						*szDrive = *p;

						// Look up each device name
						if (QueryDosDevice(szDrive, szName, MAX_PATH))
						{
							size_t uNameLen = _tcslen(szName);

							if (uNameLen < MAX_PATH)
							{
								bFound = _tcsnicmp(pszFilename, szName,
									uNameLen) == 0;

								if (bFound)
								{
									// TODO guard against buffer overflow
									wchar_t buff[100];
									swprintf_s(buff, L"%s%s", szDrive, pszFilename + uNameLen);
									strFilename = buff;

								}
							}
						}

						// Go to the next NULL character.
						while (*p++);
					} while (!bFound && *p); // end of string
				}
			}
			bSuccess = TRUE;
			UnmapViewOfFile(pMem);
		}

		CloseHandle(hFileMap);
	}

	return(strFilename);
}

static DWORD GetStartAddress(HANDLE hProcess, HANDLE hThread)
{
	// TODO implement
	//(LPVOID)debugEvent.u.CreateProcessInfo.lpStartAddress;
	return 0;
}

DebuggerImpl::DebuggerImpl(LPCTSTR debugeePath, HWND dialogHandle)
	: DebuggeePath(debugeePath), DialogHandle(dialogHandle)
{
}

void DebuggerImpl::DebuggerThreadProc()
{
	// Create debuggee process
	PROCESS_INFORMATION processInfo;
	{
		STARTUPINFO startupInfo;

		ZeroMemory(&startupInfo, sizeof(startupInfo));
		startupInfo.cb = sizeof(startupInfo);
		ZeroMemory(&processInfo, sizeof(processInfo));

		CreateProcess(
			DebuggeePath,
			NULL,
			NULL,
			NULL,
			false,
			DEBUG_ONLY_THIS_PROCESS,
			NULL,
			NULL,
			&startupInfo,
			&processInfo);
	}

	// Debugger loop
	{
		// debug event data from the OS
		DEBUG_EVENT debugEvent = {};
		// message displayed to the user
		std::wstring eventMessage = {};
		// used by `ContinueDebugEvent()` in case of exception
		DWORD continueStatus = DBG_CONTINUE;
		// DLL info cache, used to report about unloaded DLLs
		std::map<LPVOID, std::string> DLLNameMap;

		bool continueDebugging = true;
		while (continueDebugging)
		{
			WaitForDebugEvent(&debugEvent, INFINITE);

			switch (debugEvent.dwDebugEventCode)
			{
			case CREATE_PROCESS_DEBUG_EVENT:
			{
				eventMessage = GetFileNameFromHandle(debugEvent.u.CreateProcessInfo.hFile);

				// insert break point instruction at program start
				{
					DWORD dwStartAddress = GetStartAddress(processInfo.hProcess, processInfo.hThread);
					BYTE cInstruction;
					DWORD dwReadBytes;
					ReadProcessMemory(processInfo.hProcess, (void*)dwStartAddress, &cInstruction, 1, &dwReadBytes);
					BYTE originalInstruction = cInstruction;
					cInstruction = 0xCC;
					WriteProcessMemory(processInfo.hProcess, (void*)dwStartAddress, &cInstruction, 1, &dwReadBytes);
					FlushInstructionCache(processInfo.hProcess, (void*)dwStartAddress, 1);
				}
			}
			break;

			case CREATE_THREAD_DEBUG_EVENT:
				break;

			case EXIT_THREAD_DEBUG_EVENT:
				break;

			case EXIT_PROCESS_DEBUG_EVENT:
				break;

			case LOAD_DLL_DEBUG_EVENT:
				break;

			case UNLOAD_DLL_DEBUG_EVENT:
				break;

			case OUTPUT_DEBUG_STRING_EVENT:
				break;

			case EXCEPTION_DEBUG_EVENT:
				break;
			}

			SendMessage(DialogHandle, DEBUG_EVENT_MESSAGE, (WPARAM)&eventMessage, debugEvent.dwDebugEventCode);
			ContinueDebugEvent(debugEvent.dwProcessId, debugEvent.dwThreadId, continueStatus);
			continueStatus = DBG_CONTINUE;
		}
	}
}