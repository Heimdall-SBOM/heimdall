/*
Copyright 2025 The Heimdall Authors.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

/**
 * @file HeimdallClangPlugin.cpp
 * @brief Clang compiler plugin for Heimdall SBOM generation
 * @author Trevor Bakker
 * @date 2025
 *
 * This plugin hooks into Clang AST processing and preprocessor callbacks
 * to collect metadata for enhanced SBOM generation, including source files,
 * includes, hashes, and license information.
 */

#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Lex/PPCallbacks.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/SourceManager.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclCXX.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Lex/PreprocessorOptions.h"
#include "clang/Lex/HeaderSearchOptions.h"
#include "clang/Basic/Version.h"
#include "llvm/Config/llvm-config.h"
#include "../common/CompilerMetadata.hpp"
#include <memory>
#include <iostream>

using namespace clang;
using namespace heimdall::compiler;

namespace {

// Plugin configuration
struct ClangPluginConfig {
    std::string output_dir;
    std::string format;
    bool verbose;
    bool include_system_headers;
    
    ClangPluginConfig() : format("json"), verbose(false), include_system_headers(false) {}
};

/**
 * @brief AST visitor for collecting metadata from source code
 */
class HeimdallASTVisitor : public RecursiveASTVisitor<HeimdallASTVisitor> {
private:
    ASTContext &Context;
    CompilerMetadataCollector &Collector;
    ClangPluginConfig &Config;
    
public:
    HeimdallASTVisitor(ASTContext &Context, CompilerInstance &CI,
                      CompilerMetadataCollector &Collector, ClangPluginConfig &Config)
        : Context(Context), Collector(Collector), Config(Config) {}
    
    bool VisitTranslationUnitDecl(TranslationUnitDecl *TUD) {
        // Process the main source file
        SourceManager &SM = Context.getSourceManager();
        FileID MainFileID = SM.getMainFileID();
        OptionalFileEntryRef MainFile = SM.getFileEntryRefForID(MainFileID);
        
        if (MainFile) {
            std::string MainFileName = MainFile->getName().str();
            Collector.setMainSourceFile(MainFileName);
            
            if (Config.verbose) {
                llvm::outs() << "[Heimdall-Clang] Processing main source: " 
                           << MainFileName << "\n";
            }
        }
        
        return true;
    }
    
    bool VisitFunctionDecl(FunctionDecl *FD) {
        if (Context.getSourceManager().isWrittenInMainFile(FD->getLocation())) {
            std::string FunctionName = FD->getNameAsString();
            Collector.addFunction(FunctionName);
            
            if (Config.verbose) {
                llvm::outs() << "[Heimdall-Clang] Found function: " 
                           << FunctionName << "\n";
            }
        }
        return true;
    }
    
    bool VisitVarDecl(VarDecl *VD) {
        if (VD->hasGlobalStorage() && 
            Context.getSourceManager().isWrittenInMainFile(VD->getLocation())) {
            std::string VarName = VD->getNameAsString();
            Collector.addGlobalVariable(VarName);
            
            if (Config.verbose) {
                llvm::outs() << "[Heimdall-Clang] Found global variable: " 
                           << VarName << "\n";
            }
        }
        return true;
    }
    
    bool VisitCXXRecordDecl(CXXRecordDecl *RD) {
        if (RD->isCompleteDefinition() && 
            Context.getSourceManager().isWrittenInMainFile(RD->getLocation())) {
            std::string ClassName = RD->getNameAsString();
            
            if (Config.verbose) {
                llvm::outs() << "[Heimdall-Clang] Found class: " 
                           << ClassName << "\n";
            }
        }
        return true;
    }
    
    bool VisitNamespaceDecl(NamespaceDecl *ND) {
        if (Context.getSourceManager().isWrittenInMainFile(ND->getLocation())) {
            std::string NamespaceName = ND->getNameAsString();
            
            if (Config.verbose) {
                llvm::outs() << "[Heimdall-Clang] Found namespace: " 
                           << NamespaceName << "\n";
            }
        }
        return true;
    }
};

/**
 * @brief Preprocessor callbacks for tracking includes and macros
 */
class HeimdallPPCallbacks : public PPCallbacks {
private:
    CompilerInstance &CI;
    CompilerMetadataCollector &Collector;
    ClangPluginConfig &Config;
    
public:
    HeimdallPPCallbacks(CompilerInstance &CI, CompilerMetadataCollector &Collector,
                       ClangPluginConfig &Config)
        : CI(CI), Collector(Collector), Config(Config) {}
    
#if LLVM_VERSION_MAJOR >= 19
    void InclusionDirective(
        SourceLocation HashLoc,
        const Token &IncludeTok,
        StringRef FileName,
        bool IsAngled,
        CharSourceRange FilenameRange,
        OptionalFileEntryRef File,
        StringRef SearchPath,
        StringRef RelativePath,
        const Module *SuggestedModule,
        bool ModuleImported,
        SrcMgr::CharacteristicKind FileType) override {
#else
    void InclusionDirective(
        SourceLocation HashLoc,
        const Token &IncludeTok,
        StringRef FileName,
        bool IsAngled,
        CharSourceRange FilenameRange,
        OptionalFileEntryRef File,
        StringRef SearchPath,
        StringRef RelativePath,
        const Module *Imported,
        SrcMgr::CharacteristicKind FileType) override {
#endif
        
        SourceManager &SM = CI.getSourceManager();
        
        // Only track includes from main source file
        if (SM.isWrittenInMainFile(HashLoc)) {
            if (File) {
                std::string FullPath = File->getName().str();
                
                // Determine file type based on characteristics
                std::string file_type = "header";
                bool is_system = (FileType == SrcMgr::C_System || 
                                FileType == SrcMgr::C_ExternCSystem);
                
                if (is_system) {
                    file_type = "system_header";
                    
                    // Skip system headers if not configured to include them
                    if (!Config.include_system_headers) {
                        return;
                    }
                }
                
                // Process with enhanced metadata collection
                Collector.processFileComponent(FullPath, file_type);
                
                if (Config.verbose) {
                    llvm::outs() << "[Heimdall-Clang] Processed include: " 
                               << FullPath << " [" << file_type << "]\n";
                }
            }
        }
    }
    
    void MacroDefined(const Token &MacroNameTok, 
                     const MacroDirective *MD) override {
        std::string MacroName = MacroNameTok.getIdentifierInfo()->getName().str();
        Collector.addMacroDefinition(MacroName);
        
        if (Config.verbose) {
            llvm::outs() << "[Heimdall-Clang] Found macro: " 
                       << MacroName << "\n";
        }
    }
    
    void FileChanged(SourceLocation Loc, FileChangeReason Reason,
                     SrcMgr::CharacteristicKind FileType,
                     FileID PrevFID) override {
        // Track file changes during compilation
        SourceManager &SM = CI.getSourceManager();
        FileID FID = SM.getFileID(Loc);
        OptionalFileEntryRef File = SM.getFileEntryRefForID(FID);
        
        if (File && Reason == EnterFile) {
            std::string FileName = File->getName().str();
            
            // Only process non-system files unless configured otherwise
            bool is_system = (FileType == SrcMgr::C_System || 
                            FileType == SrcMgr::C_ExternCSystem);
            
            if (!is_system || Config.include_system_headers) {
                if (Config.verbose) {
                    llvm::outs() << "[Heimdall-Clang] Entered file: " 
                               << FileName << "\n";
                }
            }
        }
    }
};

/**
 * @brief AST Consumer that coordinates metadata collection
 */
class HeimdallASTConsumer : public ASTConsumer {
private:
    std::unique_ptr<HeimdallASTVisitor> Visitor;
    CompilerInstance &CI;
    std::unique_ptr<CompilerMetadataCollector> Collector;
    ClangPluginConfig Config;
    
public:
    HeimdallASTConsumer(CompilerInstance &CI, const ClangPluginConfig &config)
        : CI(CI), Config(config) {
        
        // Initialize metadata collector
        Collector = std::make_unique<CompilerMetadataCollector>();
        Collector->setVerbose(Config.verbose);
        
        if (!Config.output_dir.empty()) {
            Collector->setOutputDirectory(Config.output_dir);
        }
        
        // Set compiler information
        Collector->setCompilerType("clang");
        Collector->setCompilerVersion(CLANG_VERSION_STRING);
        
        // Initialize AST visitor
        Visitor = std::make_unique<HeimdallASTVisitor>(
            CI.getASTContext(), CI, *Collector, Config);
        
        // Add preprocessor callbacks to track includes
        CI.getPreprocessor().addPPCallbacks(
            std::make_unique<HeimdallPPCallbacks>(CI, *Collector, Config));
        
        // Initialize compiler metadata
        initializeCompilerMetadata();
        
        if (Config.verbose) {
            llvm::outs() << "[Heimdall-Clang] AST Consumer initialized\n";
        }
    }
    
    void HandleTranslationUnit(ASTContext &Context) override {
        // Process the entire translation unit
        Visitor->TraverseDecl(Context.getTranslationUnitDecl());
        
        // Write metadata to intermediate file
        Collector->writeMetadata();
        
        if (Config.verbose) {
            size_t file_count = Collector->getProcessedFileCount();
            llvm::outs() << "[Heimdall-Clang] Translation unit processed. "
                       << file_count << " files analyzed\n";
            llvm::outs() << "[Heimdall-Clang] Metadata written to: "
                       << Collector->getMetadataFilePath() << "\n";
        }
    }
    
private:
    void initializeCompilerMetadata() {
        // Capture compilation flags
        captureCompilerFlags();
        
        // Set project root from current working directory
        char* cwd = getcwd(nullptr, 0);
        if (cwd) {
            Collector->setProjectRoot(cwd);
            free(cwd);
        }
        
        if (Config.verbose) {
            llvm::outs() << "[Heimdall-Clang] Compiler metadata initialized\n";
        }
    }
    
    void captureCompilerFlags() {
        CompilerInvocation &Invocation = CI.getInvocation();
        
        // Get preprocessor options
        PreprocessorOptions &PPOpts = Invocation.getPreprocessorOpts();
        for (const auto& define : PPOpts.Macros) {
            // define.first is the macro definition, define.second is a bool indicating if it's undefined
            if (define.second) {
                // This macro should be undefined (-U flag)
                Collector->addCompilerFlag("undefine", define.first);
            } else {
                // This macro should be defined (-D flag)
                Collector->addCompilerFlag("define", define.first);
            }
        }
        
        // Get header search options
        HeaderSearchOptions &HSOpts = Invocation.getHeaderSearchOpts();
        for (const auto& entry : HSOpts.UserEntries) {
            Collector->addCompilerFlag("include_path", entry.Path);
        }
        
        // Get code generation options
        CodeGenOptions &CGOpts = Invocation.getCodeGenOpts();
        Collector->addCompilerFlag("optimization_level", 
                                 std::to_string(CGOpts.OptimizationLevel));
        Collector->addCompilerFlag("debug_info", 
                                 std::to_string(static_cast<int>(CGOpts.getDebugInfo())));
        
        // Get language options
        LangOptions &LangOpts = Invocation.getLangOpts();
        Collector->addCompilerFlag("cpp_standard", std::to_string(LangOpts.CPlusPlus));
        Collector->addCompilerFlag("exceptions_enabled", LangOpts.Exceptions ? "true" : "false");
        Collector->addCompilerFlag("rtti_enabled", LangOpts.RTTI ? "true" : "false");
        
        // Get target options
        TargetOptions &TargetOpts = Invocation.getTargetOpts();
        Collector->setTargetArchitecture(TargetOpts.Triple);
        Collector->addCompilerFlag("target_cpu", TargetOpts.CPU);
        
        if (Config.verbose) {
            llvm::outs() << "[Heimdall-Clang] Captured compiler flags\n";
        }
    }
};

/**
 * @brief Main plugin action that creates the AST consumer
 */
class HeimdallClangAction : public PluginASTAction {
private:
    ClangPluginConfig Config;
    
protected:
    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                   StringRef InFile) override {
        if (Config.verbose) {
            llvm::outs() << "[Heimdall-Clang] Creating AST consumer for: " 
                       << InFile << "\n";
        }
        
        return std::make_unique<HeimdallASTConsumer>(CI, Config);
    }
    
    bool ParseArgs(const CompilerInstance &CI,
                   const std::vector<std::string> &args) override {
        // Parse plugin arguments
        for (const auto& arg : args) {
            if (arg.find("output-dir=") == 0) {
                Config.output_dir = arg.substr(11);
            } else if (arg.find("format=") == 0) {
                Config.format = arg.substr(7);
            } else if (arg == "verbose") {
                Config.verbose = true;
            } else if (arg == "include-system-headers") {
                Config.include_system_headers = true;
            }
            
            if (Config.verbose) {
                llvm::outs() << "[Heimdall-Clang] Plugin arg: " << arg << "\n";
            }
        }
        
        return true;
    }
    
    PluginASTAction::ActionType getActionType() override {
        return AddBeforeMainAction;
    }
};

} // anonymous namespace

// Register the plugin
static FrontendPluginRegistry::Add<HeimdallClangAction>
    X("heimdall-sbom", "Heimdall SBOM Generation Plugin for Clang");