# Tiny FizzBuzz

This is a fairly small (in terms of final executable size) solution to the FizzBuzz problem, clocking in at 800 bytes for the entire executable.
Compared to a "reference" implementation (see comment in the main.cpp) this is roughly a 90% size reduction. Several tricks were employed to achieve this, which you can read about in the source code.
A smaller executable could be achieved by switching to assembly and messing with the PE header. For example, [Crinkler](https://github.com/runestubbe/Crinkler) manages to further compress the executable to 456 bytes (513 bytes when not using the more dangerous /TINYIMPORT flag).

## Instructions

If you wish to build the source code yourself to verify the claims above, you can simply clone the repository, launch a "Developer Command Prompt for VS 2019" and execute "build.bat" from that command prompt. The executable will be in the "out" folder. You can also observe this being built here on GitHub using GitHub Actions and download the resulting artifact there to verify.
In order to link with Crinkler, you will need to put the Crinkler.exe in the same folder as the "build.bat" file. The crinklerized executable will be called "fizzbuzz_cr.exe", also located in the "out" folder.

## Disclaimer

This is not a useful program, this was a response to a challenge in a discord server to create the smallest FizzBuzz that runs on Windows 10.
