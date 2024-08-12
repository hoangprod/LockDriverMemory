# LockDriverMemory

**LockDriverMemory** is a simple kernel-mode driver designed to lock memory regions of a target driver to prevent them from being paged out. This ensures the memory remains valid for snapshot-taking and enables effective snapshot fuzzing, particularly useful for scenarios like fuzzing NVIDIA's display driver, `nvlddmkm.sys`.

## Overview

While attempting to fuzz NVIDIA's display driver using an emulator like BochsCPU through WTFuzz, I encountered an issue where disabling paging in Windows settings did not prevent the driver's memory from being paged out. As a result, the memory pages became invalid for VA->PA translation, causing the emulator to crash when accessing paged-out addresses. This driver aims to solve that problem by locking the memory of the target driver, keeping it resident in memory.

**LockDriverMemory** is a kernel-mode variation of the [lockmem](https://github.com/0vercl0k/lockmem) tool, which is used to lock a process's memory rather than a driver's memory.

## Requirements

- Visual Studio
- Windows Driver Kit (WDK)

## Configuration

Before compiling, modify the `TargetDriver` variable in `Global.h` to specify the driver you want to lock:

```
namespace G
{
    inline const char* TargetDriver = "nvlddmkm.sys"; // Replace with your target driver
}
```

## Compilation

    Open the solution in Visual Studio.
    Build the driver project.

## Usage

Once compiled, you can load the driver either by signing it with a certificate or by booting Windows in Debug mode to allow unsigned drivers. Follow these steps to start the driver:

### Open an administrative command prompt.
### Create the driver service:
 ```
 sc create lockmem binPath=%PATH_TO_DRIVER.SYS% type=kernel
 ```
### Start the driver:
```
sc start lockmem
```
### Take your snapshots or set breakpoints in the target driver as needed.
### Stop the driver when finished:
```
sc stop lockmem
```

When the driver is stopped, all locked memory regions are automatically unlocked, leaving no traces behind.
## Disclaimer

This driver is intended for research and educational purposes. Use it responsibly and ensure compliance with all applicable laws and regulations.
