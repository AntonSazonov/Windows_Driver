# Simple Windows driver example
 
In order to communicate with driver, first you need to register driver in system.
Before registering it, you need to sign it with valid certificate or sign it with any test certificate and boot Windows in 'testsigning' mode.

To enable test-signing mode, run following command as Administrator and reboot your PC (note: you also need to disable Secure Boot in BIOS if it's enabled):
```bash
bcdedit.exe -set TESTSIGNING ON
```

To disable test-signing, use:
```bash
bcdedit.exe -set TESTSIGNING OFF
```

Registering driver with Service Control Manager also needs Administrator rights.

After registering and starting driver, you can communicate with it.


You can view debug messages from kernel driver with [DebugView](https://learn.microsoft.com/en-us/sysinternals/downloads/debugview) tool by Mark Russinovich.
