## || fadescii absolute zero dependency version ||

# changes from - real fadescii to fadescii absolute zero dependency version


## removed library dependencies
removed libraries are <code>unistd.h</code> and <code>sys/ioctl.h</code> which were used to get the terminal size. Now, the user has to provide the terminal size as input parameters to the `initialize` function.

## new variables in initialize function
The `initialize` function now takes two additional parameters: `termHeight` and `termWidth`, which represent the height and width of the terminal, respectively. These parameters are used to validate the box dimensions and position, ensuring that the box fits within the terminal boundaries.

## new libraries to simulate same functionality
The `unistd.h` library was used for the `usleep` function, which is now replaced with `std::this_thread::sleep_for` from the `<thread>` library. The `sys/ioctl.h` library was used to get the terminal size, which is now replaced by user-provided parameters in the `initialize` function. The `<thread>` and `<chrono>` libraries are included to provide the necessary functionality for sleeping and timing.

# still needed requirements
- ansii code capability in the terminal to display colors and effects.
- read readme.md for more information about the project and its usage.

# current cpp libraries used:
```
#include <iostream>
#include <cstdint>
#include <string>
#include <thread>
#include <chrono>
```

all of them are cpp specific libraries and are available in all cpp compilers. so no external dependencies are needed to run this project.