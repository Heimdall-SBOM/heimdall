/**
 * @file DWARFExtractor.hpp
 * @brief Robust DWARF debug information extractor using LLVM libraries
 * @author Trevor Bakker
 * @date 2025
 */

#pragma once

#include <string>
#include <vector>
#include <memory>

#ifdef LLVM_DWARF_AVAILABLE
#include <llvm/DebugInfo/DWARF/DWARFContext.h>
#include <llvm/DebugInfo/DWARF/DWARFUnit.h>
#include <llvm/DebugInfo/DWARF/DWARFDie.h>
#include <llvm/Object/ObjectFile.h>
#include <llvm/Support/Error.h>
#include <llvm/Support/MemoryBuffer.h>
#endif

namespace heimdall
{

/**
 * @brief Robust DWARF debug information extractor using LLVM libraries
 * 
 * This class provides comprehensive DWARF parsing capabilities using LLVM's
 * DWARF libraries, which are the most mature and standards-compliant
 * implementation available.
 */
class DWARFExtractor
{
public:
    /**
     * @brief Default constructor
     */
    DWARFExtractor();
    
    /**
     * @brief Destructor
     */
    ~DWARFExtractor();

    /**
     * @brief Extract source files from DWARF debug information
     * 
     * @param filePath Path to the ELF file containing DWARF info
     * @param sourceFiles Output vector to store extracted source file paths
     * @return true if extraction was successful, false otherwise
     */
    bool extractSourceFiles(const std::string& filePath, std::vector<std::string>& sourceFiles);

    /**
     * @brief Extract compile units from DWARF debug information
     * 
     * @param filePath Path to the ELF file containing DWARF info
     * @param compileUnits Output vector to store extracted compile unit names
     * @return true if extraction was successful, false otherwise
     */
    bool extractCompileUnits(const std::string& filePath, std::vector<std::string>& compileUnits);

    /**
     * @brief Extract function names from DWARF debug information
     * 
     * @param filePath Path to the ELF file containing DWARF info
     * @param functions Output vector to store extracted function names
     * @return true if extraction was successful, false otherwise
     */
    bool extractFunctions(const std::string& filePath, std::vector<std::string>& functions);

    /**
     * @brief Extract line number information from DWARF debug information
     * 
     * @param filePath Path to the ELF file containing DWARF info
     * @param lineInfo Output vector to store line number information
     * @return true if extraction was successful, false otherwise
     */
    bool extractLineInfo(const std::string& filePath, std::vector<std::string>& lineInfo);

    /**
     * @brief Check if DWARF information is available in the file
     * 
     * @param filePath Path to the ELF file to check
     * @return true if DWARF info is present, false otherwise
     */
    bool hasDWARFInfo(const std::string& filePath);

private:
    /**
     * @brief Fallback heuristic DWARF parsing when LLVM DWARF fails
     * 
     * @param filePath Path to the ELF file
     * @param sourceFiles Output vector for source files
     * @return true if any source files were found, false otherwise
     */
    bool extractSourceFilesHeuristic(const std::string& filePath, std::vector<std::string>& sourceFiles);

private:
#ifdef LLVM_DWARF_AVAILABLE
    /**
     * @brief Initialize LLVM DWARF context for a file
     * 
     * @param filePath Path to the ELF file
     * @return true if context creation succeeded, false otherwise
     */
    bool createDWARFContext(const std::string& filePath);

    /**
     * @brief Extract source files from a DWARF compile unit
     * 
     * @param die DWARF DIE (Debugging Information Entry)
     * @param sourceFiles Output vector for source files
     */
    void extractSourceFilesFromDie(const llvm::DWARFDie& die, std::vector<std::string>& sourceFiles);

    /**
     * @brief Extract compile unit information from a DWARF DIE
     * 
     * @param die DWARF DIE (Debugging Information Entry)
     * @param compileUnits Output vector for compile units
     */
    void extractCompileUnitFromDie(const llvm::DWARFDie& die, std::vector<std::string>& compileUnits);

    /**
     * @brief Extract function information from a DWARF DIE
     * 
     * @param die DWARF DIE (Debugging Information Entry)
     * @param functions Output vector for functions
     */
    void extractFunctionsFromDie(const llvm::DWARFDie& die, std::vector<std::string>& functions);

    // Store these as members to ensure their lifetime matches the DWARFContext
    std::unique_ptr<llvm::MemoryBuffer> buffer_;
    std::unique_ptr<llvm::object::ObjectFile> objectFile_;
    std::unique_ptr<llvm::DWARFContext> context_;
#endif
};

} // namespace heimdall 