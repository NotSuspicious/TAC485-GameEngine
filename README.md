# TAC 485 Spring 2026 — Final: Q*bert

Name: William Zhao
Email: wzhao029@usc.edu

A Q*bert clone built on a custom C++20 engine using SDL3 GPU. Play as the muskrat, hop around the 28-cube pyramid to flip every tile to the activated color, and dodge the bouncing red balls and the snake that hatches from the SnakeSphere.

## Controls

| Key | Action |
|---|---|
| `W` / `↑` | Jump up-right |
| `S` / `↓` | Jump down-left |
| `A` / `←` | Jump down-right (visually left) |
| `D` / `→` | Jump up-left (visually right) |
| Left-mouse drag | Orbit the follow camera |

Land on every cube to win. Falling off the pyramid respawns you at the top. Getting hit by a ball or the snake respawns you on the block where you died.

---

## 1. Download

```sh
git clone git@github.com:NotSuspicious/TAC485-GameEngine.git
cd TAC485-GameEngine
```

The repo layout is:

```
TAC485-GameEngine/
├── README.md           ← this file
├── Qbert/              ← project root (CMakeLists.txt lives here)
│   ├── Engine/         ← engine static library
│   ├── Game/           ← game executable
│   ├── UnitTest/       ← tests
│   ├── Assets/         ← meshes, materials, animations, levels
│   └── Shaders/        ← HLSL shaders
├── cmake/              ← shared compile-flag include
└── external/           ← bundled deps (SDL3, rapidjson, sse2neon, stb)
```

All external dependencies (SDL3 + SDL3_shadercross, rapidjson, stb, sse2neon) are vendored under `external/`, so no package manager is required.

### Prerequisites

* **Git** (for cloning)
* **CMake 3.30.5 or newer**
* **A C++20 compiler:**
  * **Windows:** Visual Studio 2022 (MSVC v143 or later) with the "Desktop development with C++" workload, *or* MinGW-w64 with GCC ≥ 13.
  * **macOS:** Xcode 15+ command-line tools (`xcode-select --install`), Apple Clang ≥ 15. Both Intel and Apple Silicon are supported.

> **Important:** The working directory when launching the game must contain the `Assets/` and `Shaders/` folders (the executable loads them by relative path). The instructions below all set that up correctly.

Now pick one of the following options to build and run the game:

* **§2** — CLion (Mac and Windows) — *recommended for fastest setup*
* **§3** — Visual Studio 2022 (Windows)
* **§4** — Terminal / command line (Mac, Linux, or Windows)

---

## 2. Build & Run — CLion (Mac and Windows)

1. **Open the project.** Launch CLion → **File → Open…** → select the `Qbert/` folder (the one with the top-level `CMakeLists.txt`, *not* the repo root). CLion auto-detects the CMake project.
2. **Pick a toolchain:**
   * **macOS:** CLion's default Xcode/Clang toolchain is fine.
   * **Windows:** open **Settings → Build, Execution, Deployment → Toolchains** and select **Visual Studio** (amd64). CLion will use MSVC.
3. **Wait for CMake reload.** CLion populates the `Debug` and `Release` profiles automatically (matching the existing `cmake-build-debug/` directory).
4. **Set the run configuration's working directory.** This is the most important step — without it, the game can't find its assets.
   * **Run → Edit Configurations… → Game**
   * Set **Working directory** to `$ProjectFileDir$` (the `Qbert/` folder).
   * Set **Executable** to the `Game` target (CLion may pre-fill this).
5. **Build & Run:** click the green hammer to build, then the green ▶ to run. Use the bug icon to debug with breakpoints.

> If you see "failed to load Assets/…" at startup, double-check that the working directory in your Run config is the `Qbert/` folder.

---

## 3. Build & Run — Visual Studio 2022 (Windows)

Two equally valid workflows. Use whichever you prefer.

### Option A — "Open Folder" mode (uses `CMakeSettings.json`, recommended)

1. **Open Visual Studio 2022 → Open a local folder → select `Qbert/`.** VS reads the included `CMakeSettings.json` and configures the `x64-Debug` Ninja profile automatically.
2. Wait for the "CMake generation finished" message in the Output pane.
3. In the toolbar's **Startup Item** dropdown, choose **`Game.exe`**.
4. **Set the working directory:** right-click `CMakeLists.txt` in Solution Explorer → **Debug and Launch Settings → Game** → in the generated `launch.vs.json`, add:
   ```json
   "currentDir": "${projectDir}"
   ```
   to the `Game.exe` configuration. Save.
5. **Press `F5`** (or click the green ▶) to build and run with the debugger attached. **`Ctrl+F5`** runs without the debugger. **`Ctrl+Shift+B`** builds without running.

### Option B — Generated `.sln` (classic VS solution)

If you'd rather use a normal Visual Studio solution:

1. Open the **x64 Native Tools Command Prompt for VS 2022**.
2. From the repo root:
   ```cmd
   cd Qbert
   cmake -S . -B build-vs -G "Visual Studio 17 2022" -A x64
   ```
3. Open `Qbert\build-vs\Game.sln` in Visual Studio.
4. In **Solution Explorer**, right-click the **Game** project → **Set as Startup Project**.
5. Right-click **Game** → **Properties → Configuration Properties → Debugging**. Set:
   * **Working Directory:** `$(ProjectDir)..\..` (this should resolve to the `Qbert/` folder so `Assets/` is reachable).
   * **Command:** leave as `$(TargetPath)` (default).
6. Press **`F5`** to build and run.

CMake's post-build step copies `SDL3.dll`, `SDL3_shadercross.dll`, and `dxcompiler.dll` next to `Game.exe` automatically, so no manual DLL juggling is required.

---

## 4. Build & Run — Terminal

### macOS / Linux

From the repo root:

```sh
cd Qbert
cmake -S . -B cmake-build-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build cmake-build-debug --target Game -j
```

Run from the `Qbert/` directory so the executable can find `Assets/` and `Shaders/`:

```sh
./cmake-build-debug/Game/Game
```

For a release build, swap `Debug` → `Release` and `cmake-build-debug` → `cmake-build-release`.

### Windows (PowerShell / cmd, MSVC)

From the repo root:

```powershell
cd Qbert
cmake -S . -B out\build\x64-Debug -G "Visual Studio 17 2022" -A x64
cmake --build out\build\x64-Debug --target Game --config Debug
```

CMake's post-build step copies the required `SDL3.dll`, `SDL3_shadercross.dll`, and `dxcompiler.dll` next to the executable on Windows. Then run from `Qbert\`:

```powershell
.\out\build\x64-Debug\Game\Debug\Game.exe
```

### Windows (Ninja, matches `CMakeSettings.json`)

If you'd rather use the Ninja generator (faster incremental builds, and what Visual Studio's "Open Folder" mode uses by default):

```powershell
cd Qbert
cmake -S . -B out\build\x64-Debug -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build out\build\x64-Debug --target Game
.\out\build\x64-Debug\Game\Game.exe
```

---

## Troubleshooting

* **`CMake Error: …version 3.30.5 or higher is required.`** Update CMake (`brew install cmake` on macOS, the installer from cmake.org on Windows).
* **macOS: "Game" is damaged and can't be opened.** The post-build step ad-hoc codesigns the binary; if that fails, run manually:
  ```sh
  codesign -s - -f Qbert/cmake-build-debug/Game/Game
  ```
* **Windows: `SDL3.dll not found` at launch.** Either the DLL copy step failed or you're running the exe from a directory that doesn't have it. Confirm the DLL sits next to `Game.exe`, or copy from `external/SDL3/lib/x64/`.
* **Black screen / "failed to compile shader".** Make sure the working directory contains `Shaders/`. Shaders are loaded at runtime by relative path.
* **"Cannot find Assets/Levels/Qbert.itplevel".** Same fix — the working directory must contain the `Assets/` folder. In IDEs, fix this in the Run/Debug configuration; in a terminal, `cd Qbert` before running.

---

## Project Notes

* **Engine** (`Qbert/Engine/`): renderer, asset manager, animation/skinning, job manager, math, parabolic-curve helper, and shared components (`Character`, `PointLight`, `CollisionBox`).
* **Game** (`Qbert/Game/`): `QbertLevel` (pyramid + spawn manager), `QbertCube`, ball hierarchy (`QbertBall` → `RedBall`, `SnakeSphere`), `Snake` (a `Character` component on a skinned actor), and the `Player`/`FollowCam` components.
* **Shaders** (`Qbert/Shaders/`): HLSL compiled at runtime via SDL3_shadercross. `Phong.hlsl` is the default lit shader; `PhongQbertCube.hlsl` handles the per-face tri-color tiles.
