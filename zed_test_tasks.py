import argparse
import json
import re
import subprocess
from pathlib import Path


def get_ctest_tests(build_dir: Path):
    result = subprocess.run(
        ["ctest", "-N", "-V", "--test-dir", str(build_dir)],
        check=True,
        text=True,
        capture_output=True,
    )

    tests = []

    for line in result.stdout.splitlines():
        # Matches:
        # Test #1: MySuite.MyTest
        match = re.match(r"\s*Test\s+#\d+:\s*(.+)", line)
        if match:
            tests.append(match.group(1).strip())

    return tests


def create_debug_config(tests, build_dir: Path):
    configs = []

    for test in tests:
        configs.append(
            {
                "label": f"gtest: {test}",
                "adapter": "CodeLLDB",
                "request": "launch",
                "program": "$ZED_WORKTREE_ROOT/" + find_test_binary(build_dir),
                "args": [f"--gtest_filter={test}"],
                "cwd": "$ZED_WORKTREE_ROOT",
            }
        )

    return configs


def find_test_binary(build_dir: Path):
    """
    Try to find a gtest executable automatically.
    Adjust if your project has multiple test binaries.
    """

    candidates = []

    for path in build_dir.rglob("*"):
        if (
            path.is_file()
            and path.stat().st_mode & 0o111
            and "test" in path.name.lower()
        ):
            candidates.append(path)

    if len(candidates) == 1:
        return str(candidates[0].relative_to(Path.cwd()))

    if not candidates:
        raise RuntimeError(
            "Could not find test executable. Specify it manually with --program."
        )

    raise RuntimeError(
        "Multiple possible test executables found:\n"
        + "\n".join(map(str, candidates))
        + "\nSpecify one manually with --program."
    )


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--build-dir",
        default="build",
        help="CMake build directory",
    )
    parser.add_argument(
        "--program",
        help="Relative path to test executable from project root",
    )

    args = parser.parse_args()

    build_dir = Path(args.build_dir).resolve()

    tests = get_ctest_tests(build_dir)

    if not tests:
        raise RuntimeError("No CTest tests found.")

    if args.program:
        program = args.program
        configs = [
            {
                "label": f"gtest: {test}",
                "adapter": "CodeLLDB",
                "request": "launch",
                "program": "$ZED_WORKTREE_ROOT/" + program,
                "args": [f"--gtest_filter={test}"],
                "cwd": "$ZED_WORKTREE_ROOT",
            }
            for test in tests
        ]
    else:
        configs = create_debug_config(tests, build_dir)

    zed_dir = Path(".zed")
    zed_dir.mkdir(exist_ok=True)

    output = zed_dir / "debug.json"
    output.write_text(
        json.dumps(configs, indent=2) + "\n",
        encoding="utf-8",
    )

    print(f"Generated {len(configs)} debug configurations in {output}")


if __name__ == "__main__":
    main()
