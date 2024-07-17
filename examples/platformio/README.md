# Setup environment
## WSL2

### Connect USB devices from WSL2
https://learn.microsoft.com/windows/wsl/connect-usb


In Windows Powershell, install `usbipd`
```
winget install usbipd
```

In Ubuntu (on WSL2), run the following commands with administrator privileges:
```
usbipd list
```
Find the BUSID associated with your dev board, then run
```
usbipd bind --busid=<BUSID>
usbipd attach --wsl --busid=<BUSID>
```
so that we can access to the device from WSL2.

