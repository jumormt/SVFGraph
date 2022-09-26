//
// Created by chengxiao on 26/09/22.
//

#ifndef Z3_EXAMPLE_ICFGWRAPPER_H
#define Z3_EXAMPLE_ICFGWRAPPER_H

#include "MemoryModel/SVFIR.h"

namespace SVF {
    class ICFGNodeWrapper;

    typedef GenericEdge<ICFGNodeWrapper> GenericICFGWrapperEdgeTy;

    class ICFGEdgeWrapper : public GenericICFGWrapperEdgeTy {
    public:
        typedef GenericNode<ICFGNodeWrapper, ICFGEdgeWrapper>::GEdgeSetTy ICFGEdgeWrapperSetTy;

    private:
        ICFGEdge *_icfgEdge;

    public:
        ICFGEdgeWrapper(ICFGNodeWrapper *src, ICFGNodeWrapper *dst, ICFGEdge *edge) :
                GenericICFGWrapperEdgeTy(src, dst, 0), _icfgEdge(edge) {

        }

        ~ICFGEdgeWrapper() {}

        /// Overloading operator << for dumping ICFG node ID
        //@{
        friend llvm::raw_ostream &operator<<(llvm::raw_ostream &o, const ICFGEdgeWrapper &edge) {
            o << edge.toString();
            return o;
        }
        //@}

        virtual const std::string toString() const {
            return _icfgEdge->toString();
        }

        inline ICFGEdge *getICFGEdge() const {
            return _icfgEdge;
        }

        inline void setICFGEdge(ICFGEdge *edge) {
            _icfgEdge = edge;
        }

    };

    typedef GenericNode<ICFGNodeWrapper, ICFGEdgeWrapper> GenericICFGNodeWrapperTy;

    class ICFGNodeWrapper : public GenericICFGNodeWrapperTy {
    public:
        typedef ICFGEdgeWrapper::ICFGEdgeWrapperSetTy::iterator iterator;
        typedef ICFGEdgeWrapper::ICFGEdgeWrapperSetTy::const_iterator const_iterator;
    private:
        const ICFGNode *_icfgNode;
    public:
        ICFGNodeWrapper(const ICFGNode *node) : GenericICFGNodeWrapperTy(node->getId(), 0), _icfgNode(node) {}

        /// Overloading operator << for dumping ICFG node ID
        //@{
        friend llvm::raw_ostream &operator<<(llvm::raw_ostream &o, const ICFGNodeWrapper &node) {
            o << node.toString();
            return o;
        }
        //@}


        virtual const std::string toString() const {
            return _icfgNode->toString();
        }

        const ICFGNode *getICFGNode() const {
            return _icfgNode;
        }

    };

    typedef std::vector<std::pair<NodeID, NodeID>> NodePairVector;
    typedef GenericGraph<ICFGNodeWrapper, ICFGEdgeWrapper> GenericICFGWrapperTy;

    class ICFGWrapper : public GenericICFGWrapperTy {
    public:

        typedef Map<NodeID, ICFGNodeWrapper *> ICFGWrapperNodeIDToNodeMapTy;
        typedef ICFGEdgeWrapper::ICFGEdgeWrapperSetTy ICFGEdgeWrapperSetTy;
        typedef ICFGWrapperNodeIDToNodeMapTy::iterator iterator;
        typedef ICFGWrapperNodeIDToNodeMapTy::const_iterator const_iterator;
        typedef std::vector<const ICFGNodeWrapper *> ICFGNodeWrapperVector;
        typedef std::vector<std::pair<const ICFGNodeWrapper *, const ICFGNodeWrapper *>> ICFGNodeWrapperPairVector;

    private:
        static std::unique_ptr<ICFGWrapper> _icfgWrapper; ///< Singleton pattern here
        u32_t _edgeWrapperNum;        ///< total num of node
        u32_t _nodeWrapperNum;        ///< total num of edge
        ICFG *_icfg;

        /// Constructor
        ICFGWrapper(ICFG *icfg) : _icfg(icfg), _edgeWrapperNum(0), _nodeWrapperNum(0) {

        }

    public:
        /// Singleton design here to make sure we only have one instance during any analysis
        //@{
        static inline const std::unique_ptr<ICFGWrapper> &getICFGWrapper(ICFG *_icfg) {
            if (_icfgWrapper == nullptr) {
                _icfgWrapper = std::make_unique<ICFGWrapper>(ICFGWrapper(_icfg));
            }
            return _icfgWrapper;
        }

        static inline const std::unique_ptr<ICFGWrapper> &getICFGWrapper() {
            assert(_icfgWrapper && "icfg wrapper not init?");
            return _icfgWrapper;
        }

        static void releaseICFGWrapper() {
            ICFGWrapper *w = _icfgWrapper.release();
            delete w;
            _icfgWrapper = nullptr;
        }
        //@}

        /// Destructor
        virtual ~ICFGWrapper() = default;

        /// Get a ICFG node wrapper
        inline ICFGNodeWrapper *getICFGNodeWrapper(NodeID id) const {
            if (!hasICFGNodeWrapper(id))
                return nullptr;
            return getGNode(id);
        }

        /// Whether has the ICFGNodeWrapper
        inline bool hasICFGNodeWrapper(NodeID id) const {
            return hasGNode(id);
        }

        /// Whether we has a ICFG Edge Wrapper
        bool hasICFGEdgeWrapper(ICFGNodeWrapper *src, ICFGNodeWrapper *dst, ICFGEdge *icfgEdge) {
            ICFGEdgeWrapper edge(src, dst, icfgEdge);
            ICFGEdgeWrapper *outEdge = src->hasOutgoingEdge(&edge);
            ICFGEdgeWrapper *inEdge = dst->hasIncomingEdge(&edge);
            if (outEdge && inEdge) {
                assert(outEdge == inEdge && "edges not match");
                return true;
            } else
                return false;
        }

        ICFGEdgeWrapper *hasICFGEdgeWrapper(ICFGNodeWrapper *src, ICFGNodeWrapper *dst) {
            for (const auto &e: src->getOutEdges()) {
                if (e->getDstNode() == dst)
                    return e;
            }
            return nullptr;
        }

        /// Get a ICFG edge wrapper according to src, dst and icfgEdge
        ICFGEdgeWrapper *
        getICFGEdgeWrapper(const ICFGNodeWrapper *src, const ICFGNodeWrapper *dst, ICFGEdge *icfgEdge) {
            ICFGEdgeWrapper *edge = nullptr;
            size_t counter = 0;
            for (ICFGEdgeWrapper::ICFGEdgeWrapperSetTy::iterator iter = src->OutEdgeBegin();
                 iter != src->OutEdgeEnd(); ++iter) {
                if ((*iter)->getDstID() == dst->getId()) {
                    counter++;
                    edge = (*iter);
                }
            }
            assert(counter <= 1 && "there's more than one edge between two ICFGNodeWrappers");
            return edge;
        }

        /// View graph from the debugger
        void view() {
            llvm::ViewGraph(this, "ICFG Wrapper");
        }

        /// Dump graph into dot file
        void dump(const std::string &filename) {
            GraphPrinter::WriteGraphToFile(SVFUtil::outs(), filename, this);
        }

        /// Remove a ICFGEdgeWrapper
        inline void removeICFGEdgeWrapper(ICFGEdgeWrapper *edge) {
            if (edge->getDstNode()->hasIncomingEdge(edge)) {
                edge->getDstNode()->removeIncomingEdge(edge);
            }
            if (edge->getSrcNode()->hasOutgoingEdge(edge)) {
                edge->getSrcNode()->removeOutgoingEdge(edge);
            }
            delete edge;
            _edgeWrapperNum--;
        }

        /// Remove a ICFGNodeWrapper
        inline void removeICFGNodeWrapper(ICFGNodeWrapper *node) {
            std::set<ICFGEdgeWrapper *> temp;
            for (ICFGEdgeWrapper *e: node->getInEdges())
                temp.insert(e);
            for (ICFGEdgeWrapper *e: node->getOutEdges())
                temp.insert(e);
            for (ICFGEdgeWrapper *e: temp) {
                removeICFGEdgeWrapper(e);
            }
            removeGNode(node);
            _nodeWrapperNum--;
        }

        /// Remove node from nodeID
        inline bool removeICFGNodeWrapper(NodeID id) {
            if (hasICFGNodeWrapper(id)) {
                removeICFGNodeWrapper(getICFGNodeWrapper(id));
                return true;
            }
            return false;
        }

        /// Add ICFGEdgeWrapper
        inline bool addICFGEdgeWrapper(ICFGEdgeWrapper *edge) {
            bool added1 = edge->getDstNode()->addIncomingEdge(edge);
            bool added2 = edge->getSrcNode()->addOutgoingEdge(edge);
            assert(added1 && added2 && "edge not added??");
            _edgeWrapperNum++;
            return true;
        }

        /// Add a ICFGNodeWrapper
        virtual inline void addICFGNodeWrapper(ICFGNodeWrapper *node) {
            addGNode(node->getId(), node);
            _nodeWrapperNum++;
        }

        /// Add ICFGEdgeWrappers from nodeid pair
        void addICFGNodeWrapperFromICFGNode(const ICFGNode *src);

        void addICFGNodeWrapperFromICFGNode(const ICFGNode *src, Set<u32_t> &nodeIDs);

        inline u32_t getNodeWrapperNum() const {
            return _nodeWrapperNum;
        }

        inline u32_t getEdgeWrapperNum() const {
            return _edgeWrapperNum;
        }

    };

    class ICFGWrapperBuilder {
    public:
        ICFGWrapperBuilder() {}

        ~ICFGWrapperBuilder() {}

        void build(ICFG *icfg);

        void build(ICFG *icfg, Set<u32_t> &nodeIDs);

    };
}

namespace llvm {
/* !
 * GraphTraits specializations for generic graph algorithms.
 * Provide graph traits for traversing from a constraint node using standard graph ICFGTraversals.
 */
    template<>
    struct GraphTraits<SVF::ICFGNodeWrapper *>
            : public GraphTraits<SVF::GenericNode<SVF::ICFGNodeWrapper, SVF::ICFGEdgeWrapper> *> {
    };

/// Inverse GraphTraits specializations for call graph node, it is used for inverse ICFGTraversal.
    template<>
    struct GraphTraits<Inverse<SVF::ICFGNodeWrapper *> > : public GraphTraits<
            Inverse<SVF::GenericNode<SVF::ICFGNodeWrapper, SVF::ICFGEdgeWrapper> *> > {
    };

    template<>
    struct GraphTraits<SVF::ICFGWrapper *>
            : public GraphTraits<SVF::GenericGraph<SVF::ICFGNodeWrapper, SVF::ICFGEdgeWrapper> *> {
        typedef SVF::ICFGNodeWrapper *NodeRef;
    };

    template<>
    struct DOTGraphTraits<SVF::ICFGWrapper *> : public DOTGraphTraits<SVF::SVFIR *> {

        typedef SVF::ICFGNodeWrapper NodeType;

        DOTGraphTraits(bool isSimple = false) :
                DOTGraphTraits<SVF::SVFIR *>(isSimple) {
        }

        /// Return name of the graph
        static std::string getGraphName(SVF::ICFGWrapper *) {
            return "ICFGWrapper";
        }

        static bool isNodeHidden(NodeType *node, SVF::ICFGWrapper *graph) {
            return false;
        }

        std::string getNodeLabel(NodeType *node, SVF::ICFGWrapper *graph) {
            return getSimpleNodeLabel(node, graph);
        }

        /// Return the label of an ICFG node
        static std::string getSimpleNodeLabel(NodeType *node, SVF::ICFGWrapper *) {
            std::string str;
            raw_string_ostream rawstr(str);
            rawstr << "NodeID: " << node->getId() << "\n";
            if (const SVF::IntraICFGNode *bNode = SVF::SVFUtil::dyn_cast<SVF::IntraICFGNode>(node->getICFGNode())) {
                rawstr << "IntraICFGNode ID: " << bNode->getId() << " \t";
                SVF::SVFIR::SVFStmtList &edges = SVF::SVFIR::getPAG()->getSVFStmtList(bNode);
                if (edges.empty()) {
                    rawstr << SVF::SVFUtil::value2String(bNode->getInst()) << " \t";
                } else {
                    for (SVF::SVFIR::SVFStmtList::iterator it = edges.begin(), eit = edges.end(); it != eit; ++it) {
                        const SVF::PAGEdge *edge = *it;
                        rawstr << edge->toString();
                    }
                }
                rawstr << " {fun: " << bNode->getFun()->getName() << "}";
            } else if (const SVF::FunEntryICFGNode *entry = SVF::SVFUtil::dyn_cast<SVF::FunEntryICFGNode>(
                    node->getICFGNode())) {
                rawstr << entry->toString();
            } else if (const SVF::FunExitICFGNode *exit = SVF::SVFUtil::dyn_cast<SVF::FunExitICFGNode>(
                    node->getICFGNode())) {
                rawstr << exit->toString();
            } else if (const SVF::CallICFGNode *call = SVF::SVFUtil::dyn_cast<SVF::CallICFGNode>(node->getICFGNode())) {
                rawstr << call->toString();
            } else if (const SVF::RetICFGNode *ret = SVF::SVFUtil::dyn_cast<SVF::RetICFGNode>(node->getICFGNode())) {
                rawstr << ret->toString();
            } else if (const SVF::GlobalICFGNode *glob = SVF::SVFUtil::dyn_cast<SVF::GlobalICFGNode>(
                    node->getICFGNode())) {
                SVF::SVFIR::SVFStmtList &edges = SVF::SVFIR::getPAG()->getSVFStmtList(glob);
                for (SVF::SVFIR::SVFStmtList::iterator it = edges.begin(), eit = edges.end(); it != eit; ++it) {
                    const SVF::PAGEdge *edge = *it;
                    rawstr << edge->toString();
                }
            } else
                assert(false && "what else kinds of nodes do we have??");

            return rawstr.str();
        }

        static std::string getNodeAttributes(NodeType *node, SVF::ICFGWrapper *) {
            std::string str;
            raw_string_ostream rawstr(str);

            if (SVF::SVFUtil::isa<SVF::IntraICFGNode>(node->getICFGNode())) {
                rawstr << "color=black";
            } else if (SVF::SVFUtil::isa<SVF::FunEntryICFGNode>(node->getICFGNode())) {
                rawstr << "color=yellow";
            } else if (SVF::SVFUtil::isa<SVF::FunExitICFGNode>(node->getICFGNode())) {
                rawstr << "color=green";
            } else if (SVF::SVFUtil::isa<SVF::CallICFGNode>(node->getICFGNode())) {
                rawstr << "color=red";
            } else if (SVF::SVFUtil::isa<SVF::RetICFGNode>(node->getICFGNode())) {
                rawstr << "color=blue";
            } else if (SVF::SVFUtil::isa<SVF::GlobalICFGNode>(node->getICFGNode())) {
                rawstr << "color=purple";
            } else
                assert(false && "no such kind of node!!");

//        rawstr << ", style=filled, fillcolor=red";

            rawstr << "";

            return rawstr.str();
        }

        template<class EdgeIter>
        static std::string getEdgeAttributes(NodeType *, EdgeIter EI, SVF::ICFGWrapper *) {
            SVF::ICFGEdgeWrapper *edge = *(EI.getCurrent());
            assert(edge && "No edge found!!");
            if (!edge->getICFGEdge())
                return "style=solid";
            if (SVF::SVFUtil::isa<SVF::CallCFGEdge>(edge->getICFGEdge()))
                return "style=solid,color=red";
            else if (SVF::SVFUtil::isa<SVF::RetCFGEdge>(edge->getICFGEdge()))
                return "style=solid,color=blue";
            else
                return "style=solid";
            return "";
        }

        template<class EdgeIter>
        static std::string getEdgeSourceLabel(NodeType *, EdgeIter EI) {
            SVF::ICFGEdgeWrapper *edge = *(EI.getCurrent());
            assert(edge && "No edge found!!");

            std::string str;
            raw_string_ostream rawstr(str);
            if (!edge->getICFGEdge())
                return rawstr.str();
            if (SVF::CallCFGEdge *dirCall = SVF::SVFUtil::dyn_cast<SVF::CallCFGEdge>(edge->getICFGEdge()))
                rawstr << dirCall->getCallSite();
            else if (SVF::RetCFGEdge *dirRet = SVF::SVFUtil::dyn_cast<SVF::RetCFGEdge>(edge->getICFGEdge()))
                rawstr << dirRet->getCallSite();

            return rawstr.str();
        }
    };

} // End namespace llvm
#endif //Z3_EXAMPLE_ICFGWRAPPER_H
