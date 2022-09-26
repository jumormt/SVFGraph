//
// Created by Xiao on 2022/6/15.
//

#include "SVF-FE/LLVMUtil.h"
#include "WPA/Andersen.h"
#include "SVF-FE/SVFIRBuilder.h"
#include "Util/Options.h"
#include "ICFGWrapper.h"
#include "Util/SVFUtil.h"

using namespace llvm;
using namespace std;
using namespace SVF;

static llvm::cl::opt<std::string> InputFilename(llvm::cl::Positional,
                                                llvm::cl::desc("<input bitcode>"), llvm::cl::init("-"));

static llvm::cl::opt<std::string> IDS("ids", llvm::cl::init(""),
                                             llvm::cl::desc("node ids"));

int main(int argc, char **argv) {

    int arg_num = 0;
    char **arg_value = new char *[argc];
    std::vector<std::string> moduleNameVec;
    LLVMUtil::processArguments(argc, argv, arg_num, arg_value, moduleNameVec);
    cl::ParseCommandLineOptions(arg_num, arg_value,
                                "Whole Program Points-to Analysis\n");

    if (Options::WriteAnder == "ir_annotator") {
        LLVMModuleSet::getLLVMModuleSet()->preProcessBCs(moduleNameVec);
    }

    SVFModule *svfModule = LLVMModuleSet::getLLVMModuleSet()->buildSVFModule(moduleNameVec);
    svfModule->buildSymbolTableInfo();

    /// Build Program Assignment Graph (SVFIR)
    SVFIRBuilder builder;
    SVFIR *pag = builder.build(svfModule);

    PTACallGraph *callgraph = AndersenWaveDiff::createAndersenWaveDiff(pag)->getPTACallGraph();
    builder.updateCallGraph(callgraph);


    /// ICFG
    ICFG *icfg = pag->getICFG();
    icfg->updateCallGraph(callgraph);

    ICFGWrapperBuilder icfgWrapperBuilder;

    string space_delimiter = "_";
    Set<u32_t> nodeIDs;

    size_t pos = 0;
    while ((pos = IDS.find(space_delimiter)) != string::npos) {
        nodeIDs.insert(stoi(IDS.substr(0, pos)));
        IDS.erase(0, pos + space_delimiter.length());
    }

    icfgWrapperBuilder.build(icfg, nodeIDs);
    ICFGWrapper::getICFGWrapper()->dump("ICFGWrapper");

    AndersenWaveDiff::releaseAndersenWaveDiff();
    SVFIR::releaseSVFIR();

//    LLVMModuleSet::getLLVMModuleSet()->dumpModulesToFile(".svf.bc");
    SVF::LLVMModuleSet::releaseLLVMModuleSet();

    llvm::llvm_shutdown();
    return 0;
}


