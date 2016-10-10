# lua_stackdump

C/C++ header only helper to print lua stackdumps

There are still a few TODOs in the code - pull requests are welcome

Public Domain

Example output:
``
--------------------start-of-stacktrace----------------
index | details (4 entries)
-1    | [string ""]:10: attempt to concatenate a nil value (local 'deltaMillis') (string)
-2    | userdata [0x7d1000000750] (no metatable)
-3    | userdata [0x7d0c00000cb8] (no metatable)
-4    | table: 0x7d1000000640 (size: 5)
       \-- __newindex (string) = function [0x980c90]
       \-- __name (string)  = node (string)
       \-- __tostring (string) = function [0x980aa0]  (bad argument #1 to '?' (node expected, got no value))
       \-- __index (string) = table: 0x7d1000000640 (size: 5)
           \-- __newindex (string) = function [0x980c90]
           \-- __name (string) = node (string)
           \-- __tostring (string) = function [0x980aa0]  (bad argument #1 to '?' (node expected, got no value))
           \-- __index (string) = table: 0x7d1000000640 (size: 5)
           \-- execute (string) = function [0x7d0c00000c60]
       \-- execute (string) = function [0x7d0c00000c60]
----------------------end-of-stacktrace----------------
``
