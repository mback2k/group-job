#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <strsafe.h>

LPWSTR SkipToArg( LPWSTR args, LPWSTR arg )
{
	LPWSTR *argv;
	int argc;

	for (int i = 1; arg && args[i]; i++) {
		if (args[i-1] == L' ' || args[i-1] == L'\t') {
			argv = CommandLineToArgvW( &args[i], &argc );
			if (argv) {
				if (argc > 0 && wcscmp(arg, argv[0]) == 0) {
					arg = NULL;
				}
				LocalFree(argv);
				if (!arg) {
					return &args[i];
				}
			}
		}
	}

	return NULL;
}

void _tmain( int argc, TCHAR *argv[] )
{
	JOBOBJECT_EXTENDED_LIMIT_INFORMATION jiJobInfo;
	PROCESS_INFORMATION piProcInfo; 
	STARTUPINFO siStartInfo;
	HANDLE hProcess, hJob;
	LPWSTR sCommandLine;
	DWORD lpExitCode;

	// Check the arguments.
	if (argc < 2) {
		fprintf( stderr, "Usage: group-job <command> <arguments...>\n" );
		return;
	}

	// Get the current process.
	hProcess = GetCurrentProcess();

	// Parse the commandline.
	sCommandLine = GetCommandLine();
	if( !sCommandLine ) {
		fprintf( stderr, "GetCommandLine failed (%d).\n", GetLastError() );
		return;
	}

	// Skip the module name in the commandline.
	sCommandLine = SkipToArg( sCommandLine, argv[1] );
	if( !sCommandLine ) {
		fprintf( stderr, "SkipToArg failed (%d).\n", GetLastError() );
		return;
	}

	// Setup the child process.
	ZeroMemory( &piProcInfo, sizeof(PROCESS_INFORMATION) );
	ZeroMemory( &siStartInfo, sizeof(STARTUPINFO) );
	siStartInfo.cb = sizeof(STARTUPINFO); 
	siStartInfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);
	siStartInfo.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	siStartInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
	siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

	// Setup the job object.
	hJob = CreateJobObject( NULL, NULL );
	if( !hJob ) {
		fprintf( stderr, "CreateJobObject failed (%d).\n", GetLastError() );
		return;
	}

	// Assign this process to the job object.
	if( !AssignProcessToJobObject( hJob, hProcess ) ) {
		fprintf( stderr, "AssignProcessToJobObject failed (%d).\n", GetLastError() );
		return;
	}

	// Update the job object.
	ZeroMemory( &jiJobInfo, sizeof(JOBOBJECT_EXTENDED_LIMIT_INFORMATION) );
	jiJobInfo.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;

	if( !SetInformationJobObject( hJob, JobObjectExtendedLimitInformation,
		&jiJobInfo, sizeof(JOBOBJECT_EXTENDED_LIMIT_INFORMATION) ) ) {
		fprintf( stderr, "SetInformationJobObject failed (%d).\n", GetLastError() );
		return;
	}

	// Start the child process. 
	if( !CreateProcess( NULL,   // No module name (use command line)
		sCommandLine,   // Command line
		NULL,           // Process handle not inheritable
		NULL,           // Thread handle not inheritable
		FALSE,          // Set handle inheritance to FALSE
		0,              // No creation flags
		NULL,           // Use parent's environment block
		NULL,           // Use parent's starting directory 
		&siStartInfo,   // Pointer to STARTUPINFO structure
		&piProcInfo )   // Pointer to PROCESS_INFORMATION structure
	) {
		fprintf( stderr, "CreateProcess failed (%d).\n", GetLastError() );
		return;
	}

	// Wait until child process exits.
	WaitForSingleObject( piProcInfo.hProcess, INFINITE );

	// Get the exit code of child process.
	if( !GetExitCodeProcess( piProcInfo.hProcess, &lpExitCode ) ) {
		fprintf( stderr, "GetExitCodeProcess failed (%d).\n", GetLastError() );
		return;
	}

	// Close process and thread handles. 
	CloseHandle( piProcInfo.hProcess );
	CloseHandle( piProcInfo.hThread );

	// Close job handle.
	CloseHandle( hJob );

	// Exit this process.
	ExitProcess(lpExitCode);
}