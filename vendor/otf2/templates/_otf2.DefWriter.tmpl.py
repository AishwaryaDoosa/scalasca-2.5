
''' Generated by OTF2 Template Engine '''

import ctypes

from .Config import conf, StrParam
from .ErrorCodes import ErrorCode, HandleErrorCode
from .GeneralDefinitions import *
from .Definitions import *
from .AttributeValue import AttributeValue
from .IdMap import IdMap


class DefWriter(ctypes.Structure):
    pass

def DefWriter_GetLocationID(writer):
    c_GetLocationID = conf.lib.OTF2_DefWriter_GetLocationID
    c_GetLocationID.argtypes = [ ctypes.POINTER(DefWriter), ctypes.POINTER(LocationRef) ]
    c_GetLocationID.restype = ErrorCode
    c_GetLocationID.errcheck = HandleErrorCode
    location = LocationRef()
    c_GetLocationID(writer, ctypes.byref(location))
    return location.value

@otf2 for def in defs|local_defs:
@otf2  if def == Group:
def DefWriter_WriteGroup(writerHandle, self, name, groupType, paradigm, groupFlags, members):
    numberOfMembers = len(members)
    array_type = ctypes.c_uint64 * numberOfMembers
    members_array = array_type()
    members_array[:] = members

    c_WriteGroup = conf.lib.OTF2_DefWriter_WriteGroup
    c_WriteGroup.argtypes = [ ctypes.POINTER(DefWriter), GroupRef, StringRef, GroupType, Paradigm, GroupFlag, ctypes.c_uint32, array_type ]
    c_WriteGroup.restype = ErrorCode
    c_WriteGroup.errcheck = HandleErrorCode
    c_WriteGroup(writerHandle, self, name, groupType, paradigm, groupFlags, numberOfMembers, members_array)

@otf2  elif def == MetricClass:
def DefWriter_WriteMetricClass(writerHandle, self, metricMembers, metricOccurrence, recorderKind):
    numberOfMetrics = len(metricMembers)
    array_type = MetricMemberRef * numberOfMetrics
    metric_member_array = array_type()
    metric_member_array[:] = metricMembers

    c_WriteMetricClass = conf.lib.OTF2_DefWriter_WriteMetricClass
    c_WriteMetricClass.argtypes = [ ctypes.POINTER(DefWriter), MetricRef, ctypes.c_uint8, array_type, MetricOccurrence, RecorderKind ]
    c_WriteMetricClass.restype = ErrorCode
    c_WriteMetricClass.errcheck = HandleErrorCode
    c_WriteMetricClass(writerHandle, self, numberOfMetrics, metric_member_array, metricOccurrence, recorderKind)

@otf2  elif def == CartTopology:
def DefWriter_WriteCartTopology(writerHandle, self, name, communicator, cartDimensions):
    numberOfDimensions = len(cartDimensions)
    array_type = CartDimensionRef * numberOfDimensions
    cart_dimensions_array = array_type()
    cart_dimensions_array[:] = cartDimensions

    c_WriteCartTopology = conf.lib.OTF2_DefWriter_WriteCartTopology
    c_WriteCartTopology.argtypes = [ ctypes.POINTER(DefWriter), CartTopologyRef, StringRef, CommRef, ctypes.c_uint8, array_type ]
    c_WriteCartTopology.restype = ErrorCode
    c_WriteCartTopology.errcheck = HandleErrorCode
    c_WriteCartTopology(writerHandle, self, name, communicator, numberOfDimensions, cart_dimensions_array)

@otf2  elif def == CartCoordinate:
def DefWriter_WriteCartCoordinate(writerHandle, cartTopology, rank, coordinates):
    numberOfDimensions = len(coordinates)
    array_type = ctypes.c_uint32 * numberOfDimensions
    coordinates_array = array_type()
    coordinates_array[:] = coordinates

    c_WriteCartCoordinate = conf.lib.OTF2_DefWriter_WriteCartCoordinate
    c_WriteCartCoordinate.argtypes = [ ctypes.POINTER(DefWriter), CartTopologyRef, ctypes.c_uint32, ctypes.c_uint8, array_type ]
    c_WriteCartCoordinate.restype = ErrorCode
    c_WriteCartCoordinate.errcheck = HandleErrorCode
    c_WriteCartCoordinate(writerHandle, cartTopology, rank, numberOfDimensions, coordinates_array)

@otf2  else:
def DefWriter_Write@@def.name@@(writerHandle@@def.py_funcargs()@@):
    c_Write@@def.name@@ = conf.lib.OTF2_DefWriter_Write@@def.name@@
    c_Write@@def.name@@.argtypes = [ ctypes.POINTER(DefWriter)@@def.py_funcargtypes()@@ ]
    c_Write@@def.name@@.restype = ErrorCode
    c_Write@@def.name@@.errcheck = HandleErrorCode
    c_Write@@def.name@@(writerHandle@@def.py_callargs(pass_by_ref=True)@@)

@otf2  endif
@otf2 endfor
__all__ = [
    'DefWriter',
    'DefWriter_GetLocationID',
    @otf2 for def in defs|local_defs:
    'DefWriter_Write@@def.name@@',
    @otf2 endfor
]
