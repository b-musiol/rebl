# rebl
Library for handling and analysis of reliability block diagrams (RBDs).

# Reliability Block Diagram definition
Reliability Block Diagrams are passed through a JSON format. It is always encapsulated in a list. So the simplest (nonfunct) block diagram is encoded as this:
```json
[]
```

This reliability block diagram JSON is deposited in the `valText` column of row `rbd` in table `input_rbd` in the accompanying SQLite Database (which of which `rebl` can generate a template to fill out). This database carries both the input data (the RBD) and the output data. It does not carry input parametrization that limits the failure combinations, as these are arguments passed to `rebl`, which are then documented. Consequently, any database can carry many different runs. In `output_result_summary` you find the aggregated result while `output_detail_fc` and `output_result_fc` show the failure combinations used and their individual results respectively.

There are many SQLite frontends to interact with the database. I like the [DB Browser for SQLite](https://github.com/sqlitebrowser/sqlitebrowser) the most.

## Blocks

A block is defined by its component name. That way a component can appear at different spots in the diagram and fail simultaneously. The name should not contain the `§` character, as this is used internally for some purposes, but if your name happens to not clash with any of the internally generated ones, it won't fail. Otherwise, anything UTF-8 should work, but it is not that extensively tested beyond what is provided in ASCII and common additions like Umlauts `äöüß` etc.

Reliability data is not embedded into the JSON and rather into the surrounding SQLite Database. This is done explicitly per name of each component in the table `input_components`. This table can reference data types from `input_component_types` through the column `type`. For an example check out `rbd_mid_simple.db` from the `tests` folder. If a component is not found in `input_components` but appears in the RBD, then it is considered to be ideal (So H=0.0 and T=0.0).

## Defining Topologies

If you have your RBD drawn, you need a way to encode it into JSON.

### Blocks in Series

To define two blocks in series, e.g.:

```
   -----    -----   
--|  A  |--|  B  |--
   -----    -----
```

the corresponding JSON needs to have them listed in sequence:

```json
["A", "B"]
```

If you need an anonymous, ideal block, this can be done by stating `null` in the JSON.

```
   -----   -   -----   
--|  A  |-| |-|  B  |--
   -----   -   -----
```

is represented as:

```json
["A", null, "B"]
```

This chain can be as long as you need to.

### Blocks in Parallel

To define blocks in parallel, you state consecutive sub-lists in JSON.

```
     -----    -----   
 ┌--|  A  |--|  B  |--┐
 |   -----    -----   |
-┤                    ├-
 |   -----    -----   |
 └--|  C  |--|  D  |--┘
     -----    -----
```


```json
[
["A", "B"],
["C", "D"]
]
```

Essentially, as long as a sub-list is closed and a new sub-list is opened, all sub-lists share the same fork- and join nodes in the underlying graph. So you can use as many parallel branches as you need.

If this is needed to be integrated with blocks in series, you just put then before or after the group as needed:


```
              -----    -----   
          ┌--|  A  |--|  B  |--┐
  -----   |   -----    -----   |   -----
-|  E  |--┤                    ├--|  F  |-
  -----   |   -----    -----   |   -----
          └--|  C  |--|  D  |--┘
              -----    -----
```

```json
[
"E",
["A", "B"],
["C", "D"],
"F"
]
```

If parallel branches must be rejoined early to then go to new parallel branches, you need to put a series block in between groups of `[]`. Either this is a real named block, or a `null` block. If you don't do that, then the interpretation is that more branches are set in parallel, which is not what you want in this case.

```
     -----    -----                 -----    -----          -----    -----     
 ┌--|  A  |--|  B  |--┐         ┌--|  E  |--|  G  |--┐  ┌--|  I  |--|  J  |--┐ 
 |   -----    -----   |  -----  |   -----    -----   |  |   -----    -----   | 
-┤                    ├-|  F  |-┤                    ├--┤                    ├-
 |   -----    -----   |  -----  |   -----    -----   |  |   -----    -----   | 
 └--|  C  |--|  D  |--┘         └--|  H  |--|  I  |--┘  └--|  K  |--|  L  |--┘ 
     -----    -----                 -----    -----          -----    -----     
```

```json
[
["A", "B"],
["C", "D"],
"F",
["E", "G"],
["H", "I"],
null,
["I", "J"],
["K", "L"]
]
```

### Nesting all the above

Of course, this can be used to create any RBD now, no matter how complex.

```
     -----    -----                 -----    -----          -----    -----     
 ┌--|  A  |--|  B  |--┐         ┌--|  E  |--|  G  |--┐  ┌--|  I  |--|  J  |--┐ 
 |   -----    -----   |  -----  |   -----    -----   |  |   -----    -----   | 
-┤                    ├-|  F  |-┤                    ├--┤                    ├-
 |   -----    -----   |  -----  |   -----    -----   |  |   -----    -----   | 
 ├--|  C  |--|  D  |--┘         └--|  H  |--|  I  |--┘  └--|  K  |--|  L  |--┤ 
 |   -----    -----                 -----    -----          -----    -----   | 
 |   -----    -----                 -----    -----                           |
 └--|  M  |--|  N  |------------┬--|  E  |--|  G  |-┬------------------------┘
     -----    -----             |   -----    -----  |
                                |   -----           | 
                                ├--|  F  |----------┤ 
                                |   -----           | 
                                |   -----           | 
                                └--|  A  |----------┘ 
                                    -----             
```

```json
[
    [
        ["A", "B"],
        ["C", "D"],
        "F",
        ["E", "G"],
        ["H", "I"],
        null,
        ["I", "J"],
        ["K", "L"]
    ],
    [
        "M", "N",
        ["E","G"],
        ["F"],
        ["A"]
    ]
]
```

# reblapp
The `reblapp` is a simple executable wrapper around `rebl` to demonstrate its capabilities.

To build the `reblapp` you need:
- cmake
- gcc/g++ Compiler
- Linux (Tested on CachyOS)/ Windows (Tested on Win11) / MacOS (tested on Mac Mini M4 MacOS Tahoe)

For CachyOS:
- `sudo pacman -Syu git gcc cmake` (if those are not preinstalled on your system)
- clone repository

For Windows:
- Install git
- Download MinGW64 (https://www.mingw-w64.org/) (Tested with MSYS2)
- Open MinGW64 UCRT shell (or adjust the environment variables to point to the appropriate binaries)
- clone repository

For MacOS:
- Install Homebrew (https://brew.sh/)
- `brew install cmake gcc git`
- clone repository
- If you have XCode installed, you need to `export CC=/opt/homebrew/bin/gcc-<your_installed_gcc_version>` and `export CXX=/opt/homebrew/bin/g++-<your_installed_gcc_version>` before compiling. Otherwise cmake will find AppleClang instead, which has broken tzdb support in `<chrono>`. Check with `ls /opt/homebrew/bin/` which gcc version is installed.

Then on either OS:
- `cd` into the cloned repository
- `cmake -S . -B build -DCMAKE_BUILD_TYPE=Release`
- `cmake --build build --target reblapp`
- You can now copy the `reblapp` (or `reblapp.exe` on Windows) wherever you want
- Launch the `reblapp` without arguments or with `-h` to get the documentation displayed.
- `reblapp` requires to be using a specific SQLite database format, for which a template can be generated out of `reblapp`.

# scvapp

The `scvapp` is an aid to create reliability block diagrams through a CLI.
After launching, use the `help` command to print an in depth help for the usage
of this tool.
You can also run the `-h` or `--help` argument to only print the helptext and
not launch `scvapp` fully.

For building, use the same instructions as for `reblapp`, but replace all
occurrences of `reblapp` with `scvapp`.

Note: The `scvapp` inserts a `null` block after each join, which is optional
when there are no early joins. However it is functionally equivalent to place
it.
