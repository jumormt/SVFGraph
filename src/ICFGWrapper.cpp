//
// Created by chengxiao on 26/09/22.
//
#include "ICFGWrapper.h"

using namespace SVF;
using namespace SVFUtil;

std::unique_ptr<ICFGWrapper> ICFGWrapper::_icfgWrapper = nullptr;

void ICFGWrapper::addICFGNodeWrapperFromICFGNode(const ICFGNode *src) {

    if (!hasICFGNodeWrapper(src->getId()))
        addICFGNodeWrapper(new ICFGNodeWrapper(src));
    ICFGNodeWrapper *curICFGNodeWrapper = getGNode(src->getId());
    for (const auto &e: src->getOutEdges()) {
        if (!hasICFGNodeWrapper(e->getDstID()))
            addICFGNodeWrapper(new ICFGNodeWrapper(e->getDstNode()));
        ICFGNodeWrapper *dstNodeWrapper = getGNode(e->getDstID());
        if (!hasICFGEdgeWrapper(curICFGNodeWrapper, dstNodeWrapper, e)) {
            ICFGEdgeWrapper *pEdge = new ICFGEdgeWrapper(curICFGNodeWrapper, dstNodeWrapper, e);
            addICFGEdgeWrapper(pEdge);
        }
    }
}

void ICFGWrapper::addICFGNodeWrapperFromICFGNode(const ICFGNode *src, Set<u32_t> &nodeIDs) {
    if (!nodeIDs.count(src->getId())) return;
    if (!hasICFGNodeWrapper(src->getId())) {
        addICFGNodeWrapper(new ICFGNodeWrapper(src));
    }
    ICFGNodeWrapper *curICFGNodeWrapper = getGNode(src->getId());
    for (const auto &e: src->getOutEdges()) {
        bool addEdge = nodeIDs.count(e->getDstID());
        if (!addEdge) {
            if (const IntraCFGEdge *intraEdge = dyn_cast<IntraCFGEdge>(e)) {
                if (intraEdge->getCondition()) {
                    addEdge = true;
                }
            }
        }
        if (!addEdge) continue;
        if (!hasICFGNodeWrapper(e->getDstID())) {
            addICFGNodeWrapper(new ICFGNodeWrapper(e->getDstNode()));
        }
        ICFGNodeWrapper *dstNodeWrapper = getGNode(e->getDstID());
        if (!hasICFGEdgeWrapper(curICFGNodeWrapper, dstNodeWrapper, e)) {
            ICFGEdgeWrapper *pEdge = new ICFGEdgeWrapper(curICFGNodeWrapper, dstNodeWrapper, e);
            addICFGEdgeWrapper(pEdge);
        }
    }
}

void ICFGWrapperBuilder::build(ICFG *icfg) {
    ICFGWrapper::releaseICFGWrapper();
    const std::unique_ptr<ICFGWrapper> &icfgWrapper = ICFGWrapper::getICFGWrapper(icfg);
    for (const auto &i: *icfg) {
        icfgWrapper->addICFGNodeWrapperFromICFGNode(i.second);
    }
}

void ICFGWrapperBuilder::build(ICFG *icfg, Set<u32_t> &nodeIDs) {
    ICFGWrapper::releaseICFGWrapper();
    const std::unique_ptr<ICFGWrapper> &icfgWrapper = ICFGWrapper::getICFGWrapper(icfg);
    for (const auto &i: *icfg) {
        icfgWrapper->addICFGNodeWrapperFromICFGNode(i.second, nodeIDs);
    }
}