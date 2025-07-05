/**
 * @file DWARFExtractor.hpp
 * @brief Robust DWARF debug information extractor using LLVM libraries
 * @author Trevor Bakker
 * @date 2025
 * 
 * IMPORTANT THREAD-SAFETY WARNING:
 * This class is NOT thread-safe due to LLVM's DWARF library limitations.
 * Do not create multiple DWARFExtractor instances simultaneously or use
 * from different threads. See heimdall-limitations.md for details.
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

#ifdef LLVM_DWARF_AVAILABLE
// Global LLVM objects to prevent premature destruction
extern std::unique_ptr<llvm::MemoryBuffer> g_buffer;
extern std::unique_ptr<llvm::object::ObjectFile> g_objectFile;
extern std::unique_ptr<llvm::DWARFContext> g_context;
#endif

/**
 * @brief Robust DWARF debug information extractor using LLVM libraries
 * 
 * This class provides comprehensive DWARF parsing capabilities using LLVM's
 * DWARF libraries, which are the most mature and standards-compliant
 * implementation available.
 * 
 * @warning This class is NOT thread-safe. Do not use multiple instances
 *          simultaneously or from different threads. See heimdall-limitations.md
 *          for detailed thread-safety information.
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

#ifdef LLVM_DWARF_AVAILABLE
    /**
     * @brief Ensure LLVM is initialized once per process
     */
    static void ensureLLVMInitialized();
#endif

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
     * @return raw pointer to DWARF context if creation succeeded, nullptr otherwise
     */
    llvm::DWARFContext* createDWARFContext(const std::string& filePath);

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
#endif
};

} // namespace heimdall 