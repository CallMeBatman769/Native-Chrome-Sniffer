Native-Chrome-Sniffer is a technical Proof-of-Concept (PoC) written in C++ that demonstrates how Chromium-based browsers can be manipulated via the Remote Debugging Protocol to extract sensitive session data (cookies).

Unlike common Python implementations, this tool is built as a native Windows binary, providing a deeper look into the low-level process management and IPC (Inter-Process Communication) required to interface with the Chrome 
DevTools Protocol (CDP).
This project is developed for Security Research and Educational purposes only.

It demonstrates how threat actors could potentially extract cookies without requiring Administrator privileges.
It highlights the "App-Bound Encryption" bypass through legitimate debugging flags.
Do not use this tool for illegal activities. The author assumes no liability for misuse.

Most "Cookie Stealers" or extraction PoCs use Python or Go for ease of use. This project chooses C++ for:
  Portability: Compiles to a standalone executable without needing a 500MB Python environment.
  
  Low-Level Control: Direct access to CreateProcessA and Windows handle management.
  
  Evasion Research: Understanding how process "handovers" and "zombie handles" look in a native environment.


You need:
- C++20 Standard (or higher)
- OpenSSL (for WebSocket/Curl support)

Libraries:
- nlohmann/json - For CDP command crafting.
- IXWebSocket - For real-time communication with Chrome.
- libcurl - For the initial HTTP handshake to fetch the webSocketDebuggerUrl.

Usage:

1.Ensure Google Chrome is installed in the default directory.

2.Compile the solution in Release | x64.

3.Run the executable.

4.The tool will:
- Terminate existing Chrome instances to release profile locks.
- Launch a hidden (Headless) Chrome instance with --remote-debugging-port=9222.
- Connect via WebSocket and execute Network.getAllCookies.
- Output the raw JSON cookie data to the console.

This project is licensed under the MIT License - see the LICENSE file for details.
