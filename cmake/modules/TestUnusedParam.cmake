include(CheckCSourceCompiles)
check_c_source_compiles("int main(int argc __attribute__((unused)), char *argv[] __attribute__((unused))) { return 0; }" UNUSEDPARAM_ATTRIBUTE)
check_c_source_compiles("int main(int, char *[]) { return 0; }" UNUSEDPARAM_OMIT)