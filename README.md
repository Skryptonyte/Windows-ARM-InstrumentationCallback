# Instrumentation Callback PoC for Windows 10/11 ARM

This is a proof of concept which demonstrates instrumentation callbacks on Windows ARM to intercept system calls right
after the kernel-mode to user-mode transition. Every other sample till this point has been exclusively x86 or x64 to my knowledge so I felt the need to make one for AArch64/ARM64.

# Notes

* While the return address is stored in register R10 on x86_64, it's stored in register X16 on ARM64.
* The principles are pretty much the same but the assembly stub had to obviously be completely rewritten.
* I was unable to use the RtlCreateContext/RtlRestoreContext method to save and load registers in the assembly callback like in [1]. For now, I just use LDP/STP to push and pop all registers x0-x31 to/from stack. This could possibly be improved.

# TODO

* Consider using TLS to make it more thread safe.
* Some code cleanup.

# References

1. https://github.com/Deputation/instrumentation_callbacks - Most popular sample for instrumentation callbacks on x86_64.
   I used this to get some references pertaining to information callback structs, symbol function usages, etc.
2. https://github.com/Kitware/CMake/blob/ae78e5510bb907340825faf7f8784e82b51a6caa/Tests/VSMARMASM/foo.asm - Template for MARMASM
   assembly code.
3. https://winternl.com/detecting-manual-syscalls-from-user-mode/ - Breakdown of the internals of instrumentation callbacks
