from libcpp.map cimport map
from libcpp.vector cimport vector

from tympan._core cimport SmartPtr, OGenID, SolverInterface
from tympan.models cimport _business as tybusiness
from tympan.models cimport _common as tycommon
from tympan.models cimport _solver as tysolver

cdef class Business2MicroSource:
    cdef map[tybusiness.TYElem_ptr, vector[SmartPtr[tybusiness.TYGeometryNode]]] map_sources

cdef business2microsource(map[tybusiness.TYElem_ptr, vector[SmartPtr[tybusiness.TYGeometryNode]]] map_elt_srcs)

cdef extern from "Tympan/models/business/TYPluginManager.h" namespace "tympan":
    SolverInterface* load_solver(const char *path, OGenID uuid)
