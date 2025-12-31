import os
import sys
import platform
import subprocess
import shutil

PROJECT_NAME = "CustomLanguageSwitcher"
OUTPUT_DIR = "out"


def run_command(command, cwd=None):
    try:
        print(f"🔨 Running: {' '.join(command)}")
        result = subprocess.run(command, cwd=cwd, check=True)
        return result
    except subprocess.CalledProcessError as e:
        print(f"❌ Error occurred while running command.")
        sys.exit(1)


def main():
    current_os = platform.system()
    print(f"🖥️  Detected OS: {current_os}")

    project_root = os.getcwd()

    if current_os == "Windows":
        build_dir = os.path.join(project_root, "build", "win")
        executable_name = f"{PROJECT_NAME}.exe"
    elif current_os == "Linux":
        build_dir = os.path.join(project_root, "build", "linux")
        executable_name = PROJECT_NAME
    else:
        print("❌ Unsupported OS")
        sys.exit(1)

    if not os.path.exists(build_dir):
        os.makedirs(build_dir)
        print(f"📁 Created build directory: {build_dir}")

    configure_cmd = ["cmake", "-S", ".", "-B", build_dir]
    if current_os == "Windows":
        configure_cmd.append("-DCMAKE_EXE_LINKER_FLAGS=/ENTRY:mainCRTStartup")
    else:
        configure_cmd.append("-DCMAKE_BUILD_TYPE=Release")

    run_command(configure_cmd)

    build_cmd = ["cmake", "--build", build_dir, "--config", "Release"]
    run_command(build_cmd)

    dist_path = os.path.join(project_root, OUTPUT_DIR)
    if not os.path.exists(dist_path):
        os.makedirs(dist_path)

    found_path = None
    possible_paths = [
        os.path.join(build_dir, "Release", executable_name),
        os.path.join(build_dir, "Debug", executable_name),
        os.path.join(build_dir, executable_name)
    ]

    for p in possible_paths:
        if os.path.exists(p):
            found_path = p
            break

    if found_path:
        dest = os.path.join(dist_path, executable_name)
        shutil.copy2(found_path, dest)
        print(f"\n✅ Build Success! File located at:")
        print(f"👉 {dest}")
    else:
        print("\n⚠️  Build finished, but could not locate executable to move.")


if __name__ == "__main__":
    main()
