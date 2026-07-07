#!/usr/bin/env fish

# Usage:
#   format.sh src include tests

if test (count $argv) -eq 0
    echo "Usage: "(status filename)" <directory> [directory ...]"
    exit 1
end

for dir in $argv
    if not test -d "$dir"
        echo "Warning: '$dir' is not a directory, skipping."
        continue
    end

    find "$dir" -type f \( -name '*.cpp' -o -name '*.hpp' \) | while read -l file
        echo "Formatting $file"
        clang-format -i "$file"
    end
end