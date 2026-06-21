"""Patch freertos/atomic.h to remove 7 unused static functions (IDF 5.5.x).

Called by src/project_include.cmake at cmake configure time — before any
compilation — so the removals are effective on the very first build.

The static shim components/lvgl/freertos/atomic.h handles pragma suppression
for the 2 pointer functions (SwapPointers_p32, CompareAndSwapPointers_p32)
that are used by RISC-V port.c but not detected as used by the compiler.

Idempotent: sentinel presence = already patched.
"""

import sys
from pathlib import Path

atomic_h = Path(sys.argv[1])

if not atomic_h.is_file():
    sys.exit(0)

sentinel = "/* Patched by ESPHome lvgl: unused functions removed */"

content = atomic_h.read_text(encoding="utf-8")

if sentinel in content:
    sys.exit(0)  # already patched

blocks_to_remove = [
    # Atomic_Add_u32
    (
        "/**\n"
        " * Atomic add\n"
        " *\n"
        " * @brief Atomically adds count to the value of the specified pointer points to.\n"
        " *\n"
        " * @param[in,out] pulAddend  Pointer to memory location from where value is to be\n"
        " *                         loaded and written back to.\n"
        " * @param[in] ulCount      Value to be added to *pulAddend.\n"
        " *\n"
        " * @return previous *pulAddend value.\n"
        " */\n"
        "static portFORCE_INLINE uint32_t Atomic_Add_u32( uint32_t volatile * pulAddend,\n"
        "                                                 uint32_t ulCount )\n"
        "{\n"
        "    uint32_t ulCurrent;\n"
        "\n"
        "    ATOMIC_ENTER_CRITICAL();\n"
        "    {\n"
        "        ulCurrent = *pulAddend;\n"
        "        *pulAddend += ulCount;\n"
        "    }\n"
        "    ATOMIC_EXIT_CRITICAL();\n"
        "\n"
        "    return ulCurrent;\n"
        "}\n"
        "/*-----------------------------------------------------------*/\n"
    ),
    # Atomic_Subtract_u32
    (
        "/**\n"
        " * Atomic subtract\n"
        " *\n"
        " * @brief Atomically subtracts count from the value of the specified pointer\n"
        " *        pointers to.\n"
        " *\n"
        " * @param[in,out] pulAddend  Pointer to memory location from where value is to be\n"
        " *                         loaded and written back to.\n"
        " * @param[in] ulCount      Value to be subtract from *pulAddend.\n"
        " *\n"
        " * @return previous *pulAddend value.\n"
        " */\n"
        "static portFORCE_INLINE uint32_t Atomic_Subtract_u32( uint32_t volatile * pulAddend,\n"
        "                                                      uint32_t ulCount )\n"
        "{\n"
        "    uint32_t ulCurrent;\n"
        "\n"
        "    ATOMIC_ENTER_CRITICAL();\n"
        "    {\n"
        "        ulCurrent = *pulAddend;\n"
        "        *pulAddend -= ulCount;\n"
        "    }\n"
        "    ATOMIC_EXIT_CRITICAL();\n"
        "\n"
        "    return ulCurrent;\n"
        "}\n"
        "/*-----------------------------------------------------------*/\n"
    ),
    # Atomic_Decrement_u32
    (
        "/**\n"
        " * Atomic decrement\n"
        " *\n"
        " * @brief Atomically decrements the value of the specified pointer points to\n"
        " *\n"
        " * @param[in,out] pulAddend  Pointer to memory location from where value is to be\n"
        " *                         loaded and written back to.\n"
        " *\n"
        " * @return *pulAddend value before decrement.\n"
        " */\n"
        "static portFORCE_INLINE uint32_t Atomic_Decrement_u32( uint32_t volatile * pulAddend )\n"
        "{\n"
        "    uint32_t ulCurrent;\n"
        "\n"
        "    ATOMIC_ENTER_CRITICAL();\n"
        "    {\n"
        "        ulCurrent = *pulAddend;\n"
        "        *pulAddend -= 1;\n"
        "    }\n"
        "    ATOMIC_EXIT_CRITICAL();\n"
        "\n"
        "    return ulCurrent;\n"
        "}\n"
    ),
    # Atomic_OR_u32
    (
        "/**\n"
        " * Atomic OR\n"
        " *\n"
        " * @brief Performs an atomic OR operation on the specified values.\n"
        " *\n"
        " * @param [in, out] pulDestination  Pointer to memory location from where value is\n"
        " *                                to be loaded and written back to.\n"
        " * @param [in] ulValue            Value to be ORed with *pulDestination.\n"
        " *\n"
        " * @return The original value of *pulDestination.\n"
        " */\n"
        "static portFORCE_INLINE uint32_t Atomic_OR_u32( uint32_t volatile * pulDestination,\n"
        "                                                uint32_t ulValue )\n"
        "{\n"
        "    uint32_t ulCurrent;\n"
        "\n"
        "    ATOMIC_ENTER_CRITICAL();\n"
        "    {\n"
        "        ulCurrent = *pulDestination;\n"
        "        *pulDestination |= ulValue;\n"
        "    }\n"
        "    ATOMIC_EXIT_CRITICAL();\n"
        "\n"
        "    return ulCurrent;\n"
        "}\n"
        "/*-----------------------------------------------------------*/\n"
    ),
    # Atomic_AND_u32
    (
        "/**\n"
        " * Atomic AND\n"
        " *\n"
        " * @brief Performs an atomic AND operation on the specified values.\n"
        " *\n"
        " * @param [in, out] pulDestination  Pointer to memory location from where value is\n"
        " *                                to be loaded and written back to.\n"
        " * @param [in] ulValue            Value to be ANDed with *pulDestination.\n"
        " *\n"
        " * @return The original value of *pulDestination.\n"
        " */\n"
        "static portFORCE_INLINE uint32_t Atomic_AND_u32( uint32_t volatile * pulDestination,\n"
        "                                                 uint32_t ulValue )\n"
        "{\n"
        "    uint32_t ulCurrent;\n"
        "\n"
        "    ATOMIC_ENTER_CRITICAL();\n"
        "    {\n"
        "        ulCurrent = *pulDestination;\n"
        "        *pulDestination &= ulValue;\n"
        "    }\n"
        "    ATOMIC_EXIT_CRITICAL();\n"
        "\n"
        "    return ulCurrent;\n"
        "}\n"
        "/*-----------------------------------------------------------*/\n"
    ),
    # Atomic_NAND_u32
    (
        "/**\n"
        " * Atomic NAND\n"
        " *\n"
        " * @brief Performs an atomic NAND operation on the specified values.\n"
        " *\n"
        " * @param [in, out] pulDestination  Pointer to memory location from where value is\n"
        " *                                to be loaded and written back to.\n"
        " * @param [in] ulValue            Value to be NANDed with *pulDestination.\n"
        " *\n"
        " * @return The original value of *pulDestination.\n"
        " */\n"
        "static portFORCE_INLINE uint32_t Atomic_NAND_u32( uint32_t volatile * pulDestination,\n"
        "                                                  uint32_t ulValue )\n"
        "{\n"
        "    uint32_t ulCurrent;\n"
        "\n"
        "    ATOMIC_ENTER_CRITICAL();\n"
        "    {\n"
        "        ulCurrent = *pulDestination;\n"
        "        *pulDestination = ~( ulCurrent & ulValue );\n"
        "    }\n"
        "    ATOMIC_EXIT_CRITICAL();\n"
        "\n"
        "    return ulCurrent;\n"
        "}\n"
        "/*-----------------------------------------------------------*/\n"
    ),
    # Atomic_XOR_u32
    (
        "/**\n"
        " * Atomic XOR\n"
        " *\n"
        " * @brief Performs an atomic XOR operation on the specified values.\n"
        " *\n"
        " * @param [in, out] pulDestination  Pointer to memory location from where value is\n"
        " *                                to be loaded and written back to.\n"
        " * @param [in] ulValue            Value to be XORed with *pulDestination.\n"
        " *\n"
        " * @return The original value of *pulDestination.\n"
        " */\n"
        "static portFORCE_INLINE uint32_t Atomic_XOR_u32( uint32_t volatile * pulDestination,\n"
        "                                                 uint32_t ulValue )\n"
        "{\n"
        "    uint32_t ulCurrent;\n"
        "\n"
        "    ATOMIC_ENTER_CRITICAL();\n"
        "    {\n"
        "        ulCurrent = *pulDestination;\n"
        "        *pulDestination ^= ulValue;\n"
        "    }\n"
        "    ATOMIC_EXIT_CRITICAL();\n"
        "\n"
        "    return ulCurrent;\n"
        "}\n"
    ),
]

patched = content
removed = 0
for block in blocks_to_remove:
    if block in patched:
        patched = patched.replace(block, "")
        removed += 1

if not removed:
    sys.exit(0)  # nothing matched — different IDF version, leave untouched

patched = patched.replace(
    "/* FreeRTOS Kernel V10.5.1",
    f"{sentinel}\n/* FreeRTOS Kernel V10.5.1",
)
atomic_h.write_text(patched, encoding="utf-8")
print(f"Patched freertos/atomic.h: removed {removed} unused static functions.")
