#   Copyright 2017 Google Inc.
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.

include(CheckCXXCompilerFlag)

function(sanitizer_test NAME)
    set(${NAME}_ENABLED FALSE PARENT_SCOPE)
    foreach (FLAG ${ARGN})
        set(CMAKE_REQUIRED_QUIET TRUE)
        set(CMAKE_REQUIRED_FLAGS ${FLAG})
        set(FNAME ${NAME}_${CMAKE_CXX_COMPILER_ID}_CXX)
        unset(FNAME CACHE)
        set(OLD_CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS}")
        set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} ${FLAG}")
        check_cxx_compiler_flag(${FLAG} ${FNAME})
        set(CMAKE_REQUIRED_FLAGS "${OLD_CMAKE_REQUIRED_FLAGS}")
        if (${FNAME})
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLAG}" PARENT_SCOPE)
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${FLAG}" PARENT_SCOPE)
            set(${NAME}_ENABLED TRUE PARENT_SCOPE)
            return()
        endif ()
    endforeach ()
endfunction()

option(SANITIZE_ADDRESS "Enable AddressSanitizer for the build." "")
if (SANITIZE_ADDRESS)
    set(ASAN_CANDIDATES "-fsanitize=address -fno-omit-frame-pointer" "-fsanitize=address")
    sanitizer_test(AddressSanitizer ${ASAN_CANDIDATES})
endif (SANITIZE_ADDRESS)

option(SANITIZE_LEAKS "Enable LeakSanitizer for the build." "")
if (SANITIZE_LEAKS)
    set(ASAN_CANDIDATES "-fsanitize=leaks -fno-omit-frame-pointer" "-fsanitize=leaks")
    sanitizer_test(LeaksSanitizer ${ASAN_CANDIDATES})
endif (SANITIZE_LEAKS)

option(SANITIZE_MEMORY "Enable MemorySanitizer for the build." "")
if (SANITIZE_MEMORY)
    if (${AddressSanitizer_ENABLED})
        message(FATAL_ERROR "Cannot enable MemorySanitizer when AddressSanitizer is enabled")
    else()
        set(MSAN_CANDIDATES "-fsanitize=memory -fno-omit-frame-pointer" "-fsanitize=memory")
        sanitizer_test(MemorySanitizer ${MSAN_CANDIDATES})
    endif()
endif (SANITIZE_MEMORY)

option(SANITIZE_THREAD "Enable ThreadSanitizer for the build." "")
if (SANITIZE_THREAD)
    if (${AddressSanitizer_ENABLED})
        message(FATAL_ERROR "Cannot enable ThreadSanitizer when AddressSanitizer is enabled")
    else()
        set(TSAN_CANDIDATES "-fsanitize=thread -fno-omit-frame-pointer" "-fsanitize=thread")
        sanitizer_test(ThreadSanitizer ${TSAN_CANDIDATES})
    endif()
endif (SANITIZE_THREAD)

option(SANITIZE_UNDEFINED "Enable UndefinedBehaviorSanitizer for the build." "")
if (SANITIZE_UNDEFINED)
    set(UBSAN_CANDIDATES "-fsanitize=undefined -fno-omit-frame-pointer" "-fsanitize=undefined")
    sanitizer_test(UndefinedBehaviorSanitizer ${UBSAN_CANDIDATES})
endif (SANITIZE_UNDEFINED)

# The ugly parentheses are because cmake does not support association of
# boolean operators.
if ((${AddressSanitizer_ENABLED} OR ${LeaksSanitizer_ENABLED})
        OR ((${MemorySanitizer_ENABLED} OR ${ThreadSanitizer_ENABLED})
            OR (${UndefinedBehaviorSanitizer_ENABLED})))
    set(SANITIZER_ENABLED TRUE)
endif ()