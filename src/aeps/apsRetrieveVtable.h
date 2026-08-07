// ============================================================================
// apsRetrieveVtable — shared vtable-retrieval macros for the APS registry.
// Used by apsRegister.o inline COMDATs (RetrieveVtable + retrieving-ctor).
// ============================================================================
#ifndef COD3_AEPS_APSRETRIEVEVTABLE_H
#define COD3_AEPS_APSRETRIEVEVTABLE_H

// Tag type for the vtable-only construction ctor.
enum APS_VTABLE_RETRIEVING_CTOR { APS_VTABLE_RETRIEVING_CTOR_VALUE };

// For classes whose base has a default ctor (e.g. apsSourceAction()):
//   APS_DECLARE_RETRIEVE(apsBurstAction, apsSourceAction())
// The retrieving ctor just initializes the base; MSVC sets the derived vptr.
#define APS_DECLARE_RETRIEVE(Class, BaseInit) \
    Class(APS_VTABLE_RETRIEVING_CTOR) : BaseInit {} \
    static unsigned int RetrieveVtable() { \
        Class tmp((APS_VTABLE_RETRIEVING_CTOR)0); \
        return *(unsigned int*)&tmp; \
    }

// For leaf classes whose base needs no init (apsDomain-derived, or the
// base default ctor is implicitly fine):
#define APS_DECLARE_RETRIEVE_LEAF(Class) \
    Class(APS_VTABLE_RETRIEVING_CTOR) {} \
    static unsigned int RetrieveVtable() { \
        Class tmp((APS_VTABLE_RETRIEVING_CTOR)0); \
        return *(unsigned int*)&tmp; \
    }

// Template-class version (apsBoxDomain<1>/<3> etc.): inside the class body the
// injected-class-name `Class` already names the current instantiation.
#define APS_DECLARE_RETRIEVE_LEAF_T(Class) \
    Class(APS_VTABLE_RETRIEVING_CTOR) {} \
    static unsigned int RetrieveVtable() { \
        Class tmp((APS_VTABLE_RETRIEVING_CTOR)0); \
        return *(unsigned int*)&tmp; \
    }

#endif // COD3_AEPS_APSRETRIEVEVTABLE_H
