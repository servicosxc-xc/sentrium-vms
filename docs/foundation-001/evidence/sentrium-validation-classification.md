# SENTRIUM — first Debug validation

The first Debug configure and build succeeded. CTest completed 19 cases: 18 PASS, I-9 FAIL. The original JUnit and full log remain in sentrium-validation; LastTest.log remains in the archived build tree. The invocation returned exit 1 from PowerShell. The native CTest exit code was NOT CAPTURED: the original PowerShell pipeline stopped on stderr before saving the separate exit file. No native exit code is inferred.

Read-only dumpbin /dependents identified SHELL32.dll, ole32.dll, KERNEL32.dll, MSVCP140D.dll, **MSVCP140D_ATOMIC_WAIT.dll**, VCRUNTIME140D.dll, VCRUNTIME140_1D.dll and ucrtbased.dll. The missing allowlist entry was the Microsoft STL atomic-wait runtime, used by C++20 synchronization. The R1 order explicitly permits normal Windows/MSVC runtime dependencies. Its Debug and Release names were added individually, without allowing other DLL families or removing any assertion. The test now prints each name before checking it.

The script's native stderr handling was also corrected to preserve actual exit codes on failing commands. It still stops at nonzero configure/build/test exit codes. No Release build was launched after this failed test run. The corrected source is validated from fresh build directories in sentrium-final.
