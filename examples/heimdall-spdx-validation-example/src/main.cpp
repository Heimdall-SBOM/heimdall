#include <iostream>
#include "validation_lib.h"
#include "shared_component.h"
#include "processor.h"
#include "validator.h"

int main() {
    std::cout << "SPDX Validation Test Application" << std::endl;
    ValidationLib lib;
    SharedComponent shared;
    Processor proc;
    Validator val;

    std::cout << "Running library validation: " << lib.validate("test-data") << std::endl;
    std::cout << "Running shared component: " << shared.process("shared-data") << std::endl;
    std::cout << "Running processor: " << proc.processData("proc-data") << std::endl;
    std::cout << "Running validator: " << val.validateInput("val-data") << std::endl;

    std::cout << "All components ran successfully." << std::endl;
    return 0;
} 