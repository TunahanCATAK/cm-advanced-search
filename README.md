# Fresh C++ (CMake + GCC-15 + VS Code on macOS Intel)

A minimal C++23 template that builds with **Homebrew GCC 15** and debugs with **LLDB** in VS Code — **no CMake Tools extension required**.

---

## ✅ Requirements

### System tools (macOS Intel)
```bash
# 1) Apple Command Line Tools (SDK headers & lldb)
xcode-select --install
sudo xcode-select -switch /Library/Developer/CommandLineTools
xcrun --sdk macosx --show-sdk-path   # should print an SDK path

# 2) Homebrew (if missing)
#/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# 3) GCC 15 & CMake
brew install gcc cmake
which g++-15    # expect /usr/local/bin/g++-15
which cmake     # expect /usr/local/bin/cmake
```

### VS Code & extensions
- **Visual Studio Code** — <https://code.visualstudio.com/>
- **Extensions**
  - **C/C++** — `ms-vscode.cpptools`
  - **CodeLLDB** — `vadimcn.vscode-lldb`
  - *(Optional)* **CMake Tools** — `ms-vscode.cmake-tools` (not required)

> If VS Code can’t see tools in `/usr/local/bin`, start VS Code from a shell (`code .`) or add to **Settings (JSON)**:
> ```json
> "terminal.integrated.env.osx": { "PATH": "/usr/local/bin:${env:PATH}" }
> ```

---

## 📁 Project layout

```
.
├─ CMakeLists.txt
├─ include/
│  └─ math_utils.hpp
├─ src/
│  ├─ main.cpp
│  └─ math_utils.cpp
└─ .vscode/
   ├─ settings.json
   ├─ tasks.json
   └─ launch.json
```

- **C++ standard:** C++23  
- **Compiler:** `/usr/local/bin/g++-15`  
- **Build dir:** `build/`  
- **Binary:** `build/fresh_app`

---

## 🔧 VS Code configuration (included in repo)

### `.vscode/settings.json`
```json
{
  "C_Cpp.default.compilerPath": "/usr/local/bin/g++-15",
  "C_Cpp.default.intelliSenseMode": "macos-gcc-x64",
  "C_Cpp.errorSquiggles": "Enabled"
}
```

### `.vscode/tasks.json`
```json
{
  "version": "2.0.0",
  "tasks": [
    {
      "label": "Configure (cmake)",
      "type": "shell",
      "command": "bash",
      "args": [
        "-lc",
        "SDK=$(xcrun --sdk macosx --show-sdk-path) && cmake -S \"${workspaceFolder}\" -B \"${workspaceFolder}/build\" -G \"Unix Makefiles\" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=/usr/local/bin/gcc-15 -DCMAKE_CXX_COMPILER=/usr/local/bin/g++-15 -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_C_FLAGS=\"-isysroot $SDK\" -DCMAKE_CXX_FLAGS=\"-isysroot $SDK\""
      ],
      "problemMatcher": []
    },
    {
      "label": "Build (cmake)",
      "type": "shell",
      "command": "cmake",
      "args": ["--build", "${workspaceFolder}/build", "--parallel"],
      "dependsOn": "Configure (cmake)",
      "group": { "kind": "build", "isDefault": true },
      "problemMatcher": "$gcc"
    }
  ]
}
```

### `.vscode/launch.json`
```json
{
  "version": "0.2.0",
  "configurations": [
    {
      "name": "Debug (LLDB) – fresh_app",
      "type": "lldb",
      "request": "launch",
      "program": "${workspaceFolder}/build/fresh_app",
      "cwd": "${workspaceFolder}",
      "preLaunchTask": "Build (cmake)",
      "stopOnEntry": false,
      "terminal": "integrated"
    }
  ]
}
```

---

## ▶️ Build & Debug (VS Code)

1. Open the folder in VS Code.  
2. **Build:** press **Cmd+Shift+B** → choose **Build (cmake)**.  
3. **Debug:** press **F5** → choose **Debug (LLDB) – fresh_app**.

---

## 🧪 Build & Run (Terminal)

```bash
rm -rf build
SDK=$(xcrun --sdk macosx --show-sdk-path)

cmake -S . -B build -G "Unix Makefiles"   -DCMAKE_BUILD_TYPE=Debug   -DCMAKE_C_COMPILER=/usr/local/bin/gcc-15   -DCMAKE_CXX_COMPILER=/usr/local/bin/g++-15   -DCMAKE_EXPORT_COMPILE_COMMANDS=ON   -DCMAKE_C_FLAGS="-isysroot ${SDK}"   -DCMAKE_CXX_FLAGS="-isysroot ${SDK}"

cmake --build build -j
./build/fresh_app
```

Expected:
```
__cplusplus = 202302
sum = 15
```

---

## 🚀 Optional: Release build tasks

Add to `.vscode/tasks.json`:
```json
{
  "label": "Configure (cmake) – Release",
  "type": "shell",
  "command": "bash",
  "args": [
    "-lc",
    "SDK=$(xcrun --sdk macosx --show-sdk-path) && cmake -S \"${workspaceFolder}\" -B \"${workspaceFolder}/build-release\" -G \"Unix Makefiles\" -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=/usr/local/bin/gcc-15 -DCMAKE_CXX_COMPILER=/usr/local/bin/g++-15 -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_C_FLAGS=\"-isysroot $SDK\" -DCMAKE_CXX_FLAGS=\"-isysroot $SDK\""
  ]
},
{
  "label": "Build (cmake) – Release",
  "type": "shell",
  "command": "cmake",
  "args": ["--build", "${workspaceFolder}/build-release", "--parallel"],
  "dependsOn": "Configure (cmake) – Release",
  "group": "build",
  "problemMatcher": "$gcc"
}
```

Optional launch entry:
```json
{
  "name": "Run (LLDB) – fresh_app Release",
  "type": "lldb",
  "request": "launch",
  "program": "${workspaceFolder}/build-release/fresh_app",
  "cwd": "${workspaceFolder}",
  "preLaunchTask": "Build (cmake) – Release"
}
```

---

## 🛠️ Troubleshooting

- **“Configured debug type 'lldb' is not supported.”**  
  Install **CodeLLDB** (`vadimcn.vscode-lldb`) and reload VS Code.

- **`stdio.h: 'FILE' does not name a type` during compile**  
  Fix SDK/CLT and use sysroot:
  ```bash
  xcode-select --install
  sudo xcode-select -switch /Library/Developer/CommandLineTools
  ```
  (Tasks already add `-isysroot $(xcrun …)`.)

- **VS Code can’t find `cmake` or `g++-15`**  
  Start VS Code via `code .` or set:
  ```json
  "terminal.integrated.env.osx": { "PATH": "/usr/local/bin:${env:PATH}" }
  ```

- **No breakpoints hit / variables optimized out**  
  Ensure **Debug** build (`-DCMAKE_BUILD_TYPE=Debug`).

---



Index {
  file_name  : "club.dat"
  id         : 0
  table_size : 10580
  offset     : 0
  version    : 2
}
Index {
  file_name  : "nat_club.dat"
  id         : 1
  table_size : 426
  offset     : 0
  version    : 2
}
Index {
  file_name  : "colour.dat"
  id         : 2
  table_size : 34
  offset     : 0
  version    : 1
}
Index {
  file_name  : "continent.dat"
  id         : 3
  table_size : 6
  offset     : 0
  version    : 1
}
Index {
  file_name  : "nation.dat"
  id         : 4
  table_size : 213
  offset     : 0
  version    : 2
}
Index {
  file_name  : "officials.dat"
  id         : 7
  table_size : 3124
  offset     : 0
  version    : 1
}
Index {
  file_name  : "stadium.dat"
  id         : 5
  table_size : 7099
  offset     : 0
  version    : 1
}
Index {
  file_name  : "staff.dat"
  id         : 6
  table_size : 132722
  offset     : 0
  version    : 1
}
Index {
  file_name  : "staff.dat"
  id         : 8
  table_size : 0
  offset     : 20837354
  version    : 1
}
Index {
  file_name  : "staff.dat"
  id         : 9
  table_size : 23785
  offset     : 20837354
  version    : 2
}
Index {
  file_name  : "staff.dat"
  id         : 10
  table_size : 109940
  offset     : 22454734
  version    : 2
}
Index {
  file_name  : "staff_comp.dat"
  id         : 11
  table_size : 542
  offset     : 0
  version    : 2
}
Index {
  file_name  : "city.dat"
  id         : 21
  table_size : 5418
  offset     : 0
  version    : 1
}
Index {
  file_name  : "club_comp.dat"
  id         : 12
  table_size : 390
  offset     : 0
  version    : 2
}
Index {
  file_name  : "nation_comp.dat"
  id         : 16
  table_size : 19
  offset     : 0
  version    : 2
}
Index {
  file_name  : "first_names.dat"
  id         : 13
  table_size : 34363
  offset     : 0
  version    : 1
}
Index {
  file_name  : "second_names.dat"
  id         : 14
  table_size : 82338
  offset     : 0
  version    : 1
}
Index {
  file_name  : "common_names.dat"
  id         : 15
  table_size : 8493
  offset     : 0
  version    : 1
}
Index {
  file_name  : "staff_history.dat"
  id         : 17
  table_size : 270654
  offset     : 0
  version    : 1
}
Index {
  file_name  : "staff_comp_history.dat"
  id         : 18
  table_size : 1998
  offset     : 0
  version    : 1
}
Index {
  file_name  : "club_comp_history.dat"
  id         : 19
  table_size : 7194
  offset     : 0
  version    : 1
}
Index {
  file_name  : "nation_comp_history.dat"
  id         : 20
  table_size : 143
  offset     : 0
  version    : 1
}
===============

Before added Tuni

Squads (counts exclude -1):
  playing_squad count: 48
  current_squad count: 0

Playing squad (staff IDs):
  72688 100471 46000 89867 46998 8432 54180 100603 90151 5978 
  89827 89856 12336 91296 89935 89017 90096 50839 89175 6995 
  2358 61842 40437 89037 126896 33070 41467 131039 88750 89417 
  8723 10980 6873 11280 89078 8630 10970 89795 89103 53243 
  89486 61140 89225 23415 89297 88599 42893 89666 

Playing squad (staff IDs):
  8432 88599 91296 100471 2358 42893 88750 53243 46000 54180 
  89017 89037 89078 41467 8630 89103 8723 89175 89225 61140 
  100603 89297 72688 126896 5978 10970 10980 23415 89417 33070 
  61842 11280 89486 6873 89666 50839 12336 89795 89827 89856 
  89867 89935 40437 90096 131039 46998 90151 6995 132722 